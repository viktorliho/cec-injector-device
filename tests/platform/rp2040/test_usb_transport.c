// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "adapter_state.h"
#include "rp2040_usb_transport.h"

enum { BUFFER_CAPACITY = 4096 };

typedef struct {
    uint8_t input[BUFFER_CAPACITY];
    size_t input_length;
    size_t input_offset;
    uint8_t output[BUFFER_CAPACITY];
    size_t output_length;
    size_t max_write;
    bool ready;
} fake_carrier_t;

static size_t fake_read(void *context, uint8_t *data, size_t capacity) {
    fake_carrier_t *const fake = context;
    size_t remaining = fake->input_length - fake->input_offset;
    if (remaining > capacity) {
        remaining = capacity;
    }
    memcpy(data, fake->input + fake->input_offset, remaining);
    fake->input_offset += remaining;
    return remaining;
}

static size_t fake_write(void *context,
                         const uint8_t *data,
                         size_t length) {
    fake_carrier_t *const fake = context;
    size_t accepted = length;
    if (accepted > fake->max_write) {
        accepted = fake->max_write;
    }
    assert(fake->output_length + accepted <= sizeof(fake->output));
    memcpy(fake->output + fake->output_length, data, accepted);
    fake->output_length += accepted;
    return accepted;
}

static void fake_flush(void *context) {
    (void)context;
}

static bool fake_ready(void *context) {
    return ((fake_carrier_t *)context)->ready;
}

static rp2040_usb_carrier_t carrier_for(fake_carrier_t *fake) {
    return (rp2040_usb_carrier_t){
        .read = fake_read,
        .write = fake_write,
        .flush = fake_flush,
        .ready = fake_ready,
        .context = fake,
    };
}

static void append_encoded(fake_carrier_t *fake,
                           const uint8_t *decoded,
                           size_t length) {
    const size_t encoded = pulse8_serial_encode(
        decoded,
        length,
        fake->input + fake->input_length,
        sizeof(fake->input) - fake->input_length);
    assert(encoded != 0);
    fake->input_length += encoded;
}

static void drain(rp2040_usb_transport_t *transport,
                  fake_carrier_t *fake,
                  uint64_t now_us) {
    for (size_t attempt = 0; attempt < BUFFER_CAPACITY &&
                             (fake->input_offset != fake->input_length ||
                              !rp2040_usb_transport_output_idle(transport));
         ++attempt) {
        rp2040_usb_transport_task(transport, now_us);
    }
    assert(fake->input_offset == fake->input_length);
    assert(rp2040_usb_transport_output_idle(transport));
}

static void reset_io(fake_carrier_t *fake) {
    fake->input_length = 0;
    fake->input_offset = 0;
    fake->output_length = 0;
}

static void test_streaming_and_partial_output(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    fake_carrier_t fake = {.max_write = 1, .ready = true};
    rp2040_usb_transport_t transport;
    rp2040_usb_transport_initialize(&transport, &profile, carrier_for(&fake));
    rp2040_usb_transport_mount(&transport);

    const uint8_t ping[] = {0x01};
    const uint8_t get_type[] = {0x28};
    append_encoded(&fake, ping, sizeof(ping));
    append_encoded(&fake, get_type, sizeof(get_type));
    drain(&transport, &fake, 100);

    const uint8_t expected[] = {
        0xff, 0x08, 0x01, 0xfe,
        0xff, 0x28, 0x01, 0xfe,
    };
    assert(fake.output_length == sizeof(expected));
    assert(memcmp(fake.output, expected, sizeof(expected)) == 0);
}

static void test_async_output_order(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    fake_carrier_t fake = {.max_write = 64, .ready = true};
    rp2040_usb_transport_t transport;
    rp2040_usb_transport_initialize(&transport, &profile, carrier_for(&fake));
    rp2040_usb_transport_mount(&transport);

    cec_rx_frame_t frame = {
        .bytes = {0x04, 0x44},
        .length = 2,
        .final_eom = true,
        .acknowledged_octets = 1,
    };
    pulse8_profile_message_t rx[2];
    assert(pulse8_profile_encode_rx(&frame, rx, 2) == 2);
    assert(rp2040_usb_transport_enqueue_batch(&transport, rx, 2));

    const uint8_t polarity[] = {0x0e, 0x00};
    const uint8_t eom[] = {0x0c, 0x04};
    pulse8_profile_message_t response;
    assert(pulse8_profile_dispatch(&profile, polarity, 2, 0, &response));
    assert(pulse8_profile_dispatch(&profile, eom, 2, 0, &response));
    cec_transport_tx_frame_t committed;
    assert(pulse8_profile_take_committed_tx(&profile, &committed));
    pulse8_profile_complete_tx(&profile, CEC_TX_RESULT_OK);

    drain(&transport, &fake, 1);
    const uint8_t expected[] = {
        0xff, 0x05, 0x04, 0xfe,
        0xff, 0x86, 0x44, 0xfe,
        0xff, 0x10, 0xfe,
    };
    assert(fake.output_length == sizeof(expected));
    assert(memcmp(fake.output, expected, sizeof(expected)) == 0);
}

static void test_lifecycle_boundaries(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    fake_carrier_t fake = {.max_write = 1, .ready = true};
    rp2040_usb_transport_t transport;
    rp2040_usb_transport_initialize(&transport, &profile, carrier_for(&fake));
    rp2040_usb_transport_mount(&transport);

    const uint8_t controlled[] = {0x18, 0x01};
    append_encoded(&fake, controlled, sizeof(controlled));
    drain(&transport, &fake, 10);
    assert(adapter_state_snapshot().controlled_mode);

    reset_io(&fake);
    const uint8_t partial[] = {0xff, 0x01};
    memcpy(fake.input, partial, sizeof(partial));
    fake.input_length = sizeof(partial);
    rp2040_usb_transport_task(&transport, 11);
    rp2040_usb_transport_suspend(&transport);
    fake.ready = false; /* CDC disconnected while the same session sleeps. */
    rp2040_usb_transport_task(&transport, 12);
    assert(adapter_state_snapshot().controlled_mode);
    rp2040_usb_transport_resume(&transport);
    fake.ready = true;
    const uint8_t end = 0xfe;
    fake.input[0] = end;
    fake.input_length = 1;
    fake.input_offset = 0;
    drain(&transport, &fake, 13);
    assert(fake.output_length == 4);

    reset_io(&fake);
    assert(rp2040_usb_transport_enqueue(&transport,
                                        controlled,
                                        sizeof(controlled)));
    rp2040_usb_transport_unmount(&transport);
    assert(!adapter_state_snapshot().controlled_mode);
    assert(rp2040_usb_transport_output_idle(&transport));

    rp2040_usb_transport_mount(&transport);
    fake.input[0] = 0xfe; /* Must not complete the old partial frame. */
    fake.input_length = 1;
    fake.input_offset = 0;
    rp2040_usb_transport_task(&transport, 14);
    assert(fake.output_length == 0);
}

int main(void) {
    test_streaming_and_partial_output();
    test_async_output_order();
    test_lifecycle_boundaries();
    puts("RP2040 USB transport tests: PASS");
    return 0;
}
