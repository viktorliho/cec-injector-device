// SPDX-License-Identifier: Apache-2.0

#include "rp2040_cec_follower_ack.h"

#include "hardware/timer.h"
#include "pico/time.h"
#include "rp2040_cec_gpio.h"

enum {
    ACK_DRIVE_OFFSET_US = 500,
    ACK_RELEASE_OFFSET_US = 1500,
};

enum ack_state {
    ACK_IDLE,
    ACK_WAIT_SLOT_FALL,
    ACK_WAIT_DRIVE,
    ACK_WAIT_RELEASE,
};

static volatile enum ack_state state;
static uint alarm_number;
static uint64_t ack_slot_start_us;

static void follower_ack_alarm(uint alarm_num) {
    (void)alarm_num;
    if (state == ACK_WAIT_DRIVE) {
        rp2040_cec_gpio_drive_low();
        state = ACK_WAIT_RELEASE;
        if (hardware_alarm_set_target(
                alarm_number,
                from_us_since_boot(ack_slot_start_us +
                                   ACK_RELEASE_OFFSET_US))) {
            rp2040_cec_gpio_release();
            state = ACK_IDLE;
        }
        return;
    }
    if (state == ACK_WAIT_RELEASE) {
        rp2040_cec_gpio_release();
        state = ACK_IDLE;
    }
}

void rp2040_cec_follower_ack_initialize(void) {
    state = ACK_IDLE;
    alarm_number = hardware_alarm_claim_unused(true);
    hardware_alarm_set_callback(alarm_number, follower_ack_alarm);
    rp2040_cec_gpio_release();
}

void rp2040_cec_follower_ack_apply_decision(bool should_ack) {
    if (!should_ack) {
        rp2040_cec_follower_ack_cancel();
    } else if (state == ACK_IDLE) {
        state = ACK_WAIT_SLOT_FALL;
    }
}

void rp2040_cec_follower_ack_cancel(void) {
    hardware_alarm_cancel(alarm_number);
    state = ACK_IDLE;
    rp2040_cec_gpio_release();
}

void rp2040_cec_follower_ack_on_edge(bool current_level,
                                     uint64_t timestamp_us) {
    if (!current_level && state == ACK_WAIT_SLOT_FALL) {
        state = ACK_WAIT_DRIVE;
        ack_slot_start_us = timestamp_us;
        if (hardware_alarm_set_target(
                alarm_number,
                from_us_since_boot(timestamp_us + ACK_DRIVE_OFFSET_US))) {
            state = ACK_IDLE;
            rp2040_cec_gpio_release();
        }
    }
}

void rp2040_cec_follower_ack_decision_sink(bool should_ack, void *context) {
    (void)context;
    rp2040_cec_follower_ack_apply_decision(should_ack);
}

void rp2040_cec_follower_ack_cancel_sink(void *context) {
    (void)context;
    rp2040_cec_follower_ack_cancel();
}
