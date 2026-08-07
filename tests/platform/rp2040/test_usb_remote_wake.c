// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "rp2040_usb_lifecycle.h"

void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_suspend_cb(bool remote_wakeup_en);
void tud_resume_cb(void);

static uint64_t fake_time_us;
static bool remote_wake_result;
static unsigned remote_wake_calls;

uint64_t time_us_64(void) {
    return fake_time_us;
}

bool tud_remote_wakeup(void) {
    remote_wake_calls++;
    return remote_wake_result;
}

static size_t no_read(void *context, uint8_t *data, size_t capacity) {
    (void)context;
    (void)data;
    (void)capacity;
    return 0;
}

static size_t no_write(void *context,
                       const uint8_t *data,
                       size_t length) {
    (void)context;
    (void)data;
    return length;
}

static bool carrier_ready(void *context) {
    return *(bool *)context;
}

typedef struct {
    pulse8_profile_t profile;
    rp2040_usb_transport_t transport;
    rp2040_usb_remote_wake_t remote_wake;
    bool cdc_ready;
} fixture_t;

static void fixture_initialize(fixture_t *fixture) {
    pulse8_profile_initialize(&fixture->profile);
    fixture->cdc_ready = true;
    rp2040_usb_carrier_t carrier = {
        .read = no_read,
        .write = no_write,
        .flush = NULL,
        .ready = carrier_ready,
        .context = &fixture->cdc_ready,
    };
    rp2040_usb_transport_initialize(&fixture->transport,
                                    &fixture->profile,
                                    carrier);
    rp2040_usb_remote_wake_initialize(&fixture->remote_wake,
                                      &fixture->profile);
    rp2040_usb_lifecycle_bind(&fixture->transport, &fixture->remote_wake);
    remote_wake_calls = 0;
    remote_wake_result = true;
    fake_time_us = 1000;
    tud_mount_cb();
}

static void configure_and_arm(fixture_t *fixture) {
    wake_extension_t *const wake =
        pulse8_profile_wake_extension(&fixture->profile);
    assert(wake_extension_set_config(
        wake, true, 0x1000, WAKE_EXTENSION_MODE_SET_STREAM_PATH));
    assert(wake_extension_arm(wake));
}

static cec_rx_frame_t matching_frame(uint64_t timestamp_us) {
    return (cec_rx_frame_t){
        .bytes = {0x0f, 0x86, 0x10, 0x00},
        .length = 4,
        .final_eom = true,
        .start_timestamp_us = timestamp_us,
    };
}

static void test_authorized_success_and_timing(void) {
    fixture_t fixture;
    fixture_initialize(&fixture);
    configure_and_arm(&fixture);
    wake_extension_t *const wake =
        pulse8_profile_wake_extension(&fixture.profile);

    fake_time_us = 2000;
    tud_suspend_cb(true);
    wake_extension_status_t status = wake_extension_status(wake);
    assert(status.transport_suspended);
    assert(status.remote_wake_authorized);
    assert(status.config.valid && status.armed);

    cec_rx_frame_t frame = matching_frame(3000);
    wake_extension_offer_frame(wake, &frame);
    wake_extension_poll(wake, 11999);
    assert(remote_wake_calls == 0);
    wake_extension_poll(wake, 12000);
    assert(remote_wake_calls == 1);
    status = wake_extension_status(wake);
    assert(status.last_result == WAKE_EXTENSION_RESULT_SUCCESS);
    wake_extension_poll(wake, 50000);
    assert(remote_wake_calls == 1);

    tud_resume_cb();
    status = wake_extension_status(wake);
    assert(!status.transport_suspended);
    assert(!status.remote_wake_authorized);
    assert(rp2040_usb_transport_attached(&fixture.transport));
}

static void test_failure_has_no_platform_retry(void) {
    fixture_t fixture;
    fixture_initialize(&fixture);
    configure_and_arm(&fixture);
    remote_wake_result = false;
    fake_time_us = 5000;
    tud_suspend_cb(true);

    wake_extension_t *const wake =
        pulse8_profile_wake_extension(&fixture.profile);
    cec_rx_frame_t frame = matching_frame(6000);
    wake_extension_offer_frame(wake, &frame);
    wake_extension_poll(wake, 15000);
    assert(remote_wake_calls == 1);
    assert(wake_extension_status(wake).last_result ==
           WAKE_EXTENSION_RESULT_REMOTE_WAKE_FAILED);
    wake_extension_poll(wake, 50000);
    assert(remote_wake_calls == 1);
}

static void test_ineligible_states_do_not_deliver(void) {
    fixture_t fixture;
    fixture_initialize(&fixture);
    configure_and_arm(&fixture);
    wake_extension_t *const wake =
        pulse8_profile_wake_extension(&fixture.profile);
    cec_rx_frame_t frame = matching_frame(2000);

    wake_extension_offer_frame(wake, &frame); /* Awake. */
    wake_extension_poll(wake, 50000);
    assert(remote_wake_calls == 0);

    fake_time_us = 60000;
    tud_suspend_cb(false);
    assert(!wake_extension_status(wake).remote_wake_authorized);
    frame = matching_frame(61000);
    wake_extension_offer_frame(wake, &frame);
    wake_extension_poll(wake, 100000);
    assert(remote_wake_calls == 0);

    tud_resume_cb();
    wake_extension_disarm(wake);
    fake_time_us = 110000;
    tud_suspend_cb(true);
    frame = matching_frame(111000);
    wake_extension_offer_frame(wake, &frame);
    wake_extension_poll(wake, 150000);
    assert(remote_wake_calls == 0);
}

static void test_resume_and_unmount_boundaries(void) {
    fixture_t fixture;
    fixture_initialize(&fixture);
    configure_and_arm(&fixture);
    wake_extension_t *const wake =
        pulse8_profile_wake_extension(&fixture.profile);

    fake_time_us = 200000;
    tud_suspend_cb(true);
    cec_rx_frame_t frame = matching_frame(201000);
    wake_extension_offer_frame(wake, &frame);
    fixture.cdc_ready = false;
    rp2040_usb_transport_task(&fixture.transport, 205000);
    assert(wake_extension_status(wake).config.valid);
    assert(wake_extension_status(wake).armed);
    assert(wake_extension_status(wake).remote_wake_authorized);

    tud_resume_cb(); /* Cancels the pending delivery. */
    wake_extension_poll(wake, 250000);
    assert(remote_wake_calls == 0);

    tud_umount_cb();
    const wake_extension_status_t status = wake_extension_status(wake);
    assert(!status.config.valid);
    assert(!status.armed);
    assert(!status.transport_suspended);
    assert(!status.remote_wake_authorized);
    assert(!rp2040_usb_transport_attached(&fixture.transport));
}

int main(void) {
    test_authorized_success_and_timing();
    test_failure_has_no_platform_retry();
    test_ineligible_states_do_not_deliver();
    test_resume_and_unmount_boundaries();
    puts("RP2040 USB Remote Wake tests: PASS");
    return 0;
}
