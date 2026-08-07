// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "rp2040_adapter_runtime.h"

void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_suspend_cb(bool remote_wakeup_en);

enum { BUFFER_CAPACITY = 4096, OBSERVATION_CAPACITY = 256 };

static uint8_t carrier_input[BUFFER_CAPACITY];
static size_t carrier_input_length;
static size_t carrier_input_offset;
static uint8_t carrier_output[BUFFER_CAPACITY];
static size_t carrier_output_length;
static bool cdc_ready;
static bool usb_started;
static unsigned usb_task_count;
static cec_rx_observation_t observations[OBSERVATION_CAPACITY];
static size_t observation_count;
static size_t observation_offset;
static uint32_t analyzer_dropped;
static unsigned physical_tx_count;
static cec_transport_tx_frame_t last_physical_tx;
static unsigned remote_wake_count;
static uint64_t fake_time_us;

uint64_t time_us_64(void) {
    return fake_time_us;
}

bool tud_remote_wakeup(void) {
    remote_wake_count++;
    return true;
}

void rp2040_cec_gpio_initialize(void) {}
void rp2040_cec_follower_ack_initialize(void) {}
void rp2040_cec_follower_ack_decision_sink(bool should_ack, void *context) {
    (void)should_ack;
    (void)context;
}
void rp2040_cec_follower_ack_cancel_sink(void *context) {
    (void)context;
}
void rp2040_cec_analyzer_initialize(void) {
    observation_count = 0;
    observation_offset = 0;
    analyzer_dropped = 0;
}
bool rp2040_cec_analyzer_pop(cec_rx_observation_t *observation) {
    if (observation_offset == observation_count) {
        return false;
    }
    *observation = observations[observation_offset++];
    return true;
}
uint32_t rp2040_cec_analyzer_dropped_count(void) {
    return analyzer_dropped;
}

cec_tx_result_t rp2040_cec_transmit(
    const cec_transport_tx_frame_t *frame,
    cec_rx_decoder_t *rx_decoder) {
    physical_tx_count++;
    last_physical_tx = *frame;
    cec_rx_decoder_notify_local_tx_begin(rx_decoder);
    cec_rx_decoder_notify_local_tx_end(rx_decoder);
    return CEC_TX_RESULT_OK;
}

static size_t carrier_read(void *context, uint8_t *data, size_t capacity) {
    (void)context;
    size_t remaining = carrier_input_length - carrier_input_offset;
    if (remaining > capacity) {
        remaining = capacity;
    }
    memcpy(data, carrier_input + carrier_input_offset, remaining);
    carrier_input_offset += remaining;
    return remaining;
}

static size_t carrier_write(void *context,
                            const uint8_t *data,
                            size_t length) {
    (void)context;
    assert(carrier_output_length + length <= sizeof(carrier_output));
    memcpy(carrier_output + carrier_output_length, data, length);
    carrier_output_length += length;
    return length;
}

static bool carrier_ready(void *context) {
    (void)context;
    return cdc_ready;
}

rp2040_usb_carrier_t rp2040_usb_tinyusb_carrier(void) {
    return (rp2040_usb_carrier_t){
        .read = carrier_read,
        .write = carrier_write,
        .flush = NULL,
        .ready = carrier_ready,
        .context = NULL,
    };
}

void rp2040_usb_tinyusb_start(void) {
    usb_started = true;
}

void rp2040_usb_tinyusb_task(void) {
    usb_task_count++;
}

static void reset_carrier_output(void) {
    carrier_output_length = 0;
}

static void append_host_message(const uint8_t *message, size_t length) {
    const size_t encoded = pulse8_serial_encode(
        message,
        length,
        carrier_input + carrier_input_length,
        sizeof(carrier_input) - carrier_input_length);
    assert(encoded != 0);
    carrier_input_length += encoded;
}

typedef struct {
    pulse8_profile_message_t messages[32];
    size_t count;
} decoded_output_t;

static void capture_output(const uint8_t *frame,
                           size_t length,
                           void *context) {
    decoded_output_t *const output = context;
    assert(output->count < 32);
    output->messages[output->count].length = length;
    memcpy(output->messages[output->count].data, frame, length);
    output->count++;
}

static decoded_output_t decode_output(void) {
    decoded_output_t output = {0};
    pulse8_serial_parser_t parser;
    pulse8_serial_parser_initialize(&parser);
    pulse8_serial_parser_feed(&parser,
                              carrier_output,
                              carrier_output_length,
                              capture_output,
                              &output);
    return output;
}

static void run_tasks(rp2040_adapter_runtime_t *runtime, size_t count) {
    for (size_t index = 0; index < count; ++index) {
        rp2040_adapter_runtime_task(runtime, fake_time_us);
    }
}

static void queue_observation(uint32_t low_us, uint32_t high_us) {
    assert(observation_count + 2 <= OBSERVATION_CAPACITY);
    observations[observation_count++] = (cec_rx_observation_t){
        .timestamp_us = fake_time_us + low_us,
        .duration_us = low_us,
        .previous_level_high = false,
    };
    fake_time_us += low_us + high_us;
    observations[observation_count++] = (cec_rx_observation_t){
        .timestamp_us = fake_time_us,
        .duration_us = high_us,
        .previous_level_high = true,
    };
}

static void queue_bit(bool value) {
    queue_observation(value ? 611u : 1511u, value ? 1789u : 889u);
}

static void queue_byte(uint8_t byte, bool eom) {
    for (int bit = 7; bit >= 0; --bit) {
        queue_bit((byte & (1u << bit)) != 0);
    }
    queue_bit(eom);
    queue_bit(true); /* Physically HIGH ACK slot. */
}

static void append_cec_frame(const uint8_t *bytes, size_t length) {
    queue_observation(3711, 789);
    for (size_t index = 0; index < length; ++index) {
        queue_byte(bytes[index], index + 1u == length);
    }
}

static void queue_cec_frame(const uint8_t *bytes, size_t length) {
    observation_count = 0;
    observation_offset = 0;
    append_cec_frame(bytes, length);
}

int main(void) {
    rp2040_adapter_runtime_t runtime;
    cdc_ready = true;
    fake_time_us = 100000;
    rp2040_adapter_runtime_initialize(&runtime);
    assert(usb_started);
    assert(physical_tx_count == 0);
    assert(remote_wake_count == 0);
    tud_mount_cb();

    const uint8_t ping[] = {0x01};
    append_host_message(ping, sizeof(ping));
    run_tasks(&runtime, 2);
    assert(usb_task_count == 2);
    decoded_output_t output = decode_output();
    assert(output.count == 1);
    assert(output.messages[0].length == 2);
    assert(output.messages[0].data[0] == 0x08);
    assert(output.messages[0].data[1] == 0x01);

    reset_carrier_output();
    const uint8_t tx_polarity[] = {0x0e, 0x00};
    const uint8_t tx_eom[] = {0x0c, 0x04};
    append_host_message(tx_polarity, sizeof(tx_polarity));
    append_host_message(tx_eom, sizeof(tx_eom));
    run_tasks(&runtime, 5);
    assert(physical_tx_count == 1);
    assert(last_physical_tx.length == 1 && last_physical_tx.bytes[0] == 0x04);
    output = decode_output();
    assert(output.count == 3);
    assert(output.messages[2].length == 1);
    assert(output.messages[2].data[0] == 0x10);

    reset_carrier_output();
    const uint8_t received[] = {0x04};
    queue_cec_frame(received, sizeof(received));
    run_tasks(&runtime, 3);
    output = decode_output();
    assert(output.count == 1);
    assert(output.messages[0].length == 2);
    assert(output.messages[0].data[0] == (uint8_t)(0x05 | 0x40 | 0x80));
    assert(output.messages[0].data[1] == 0x04);

    reset_carrier_output();
    observation_count = 0;
    observation_offset = 0;
    const uint8_t first_received[] = {0x04};
    const uint8_t second_received[] = {0x0f};
    append_cec_frame(first_received, sizeof(first_received));
    append_cec_frame(second_received, sizeof(second_received));
    run_tasks(&runtime, 3);
    output = decode_output();
    assert(output.count == 2);
    assert(output.messages[0].data[1] == 0x04);
    assert(output.messages[1].data[1] == 0x0f);

    wake_extension_t *const wake =
        pulse8_profile_wake_extension(&runtime.profile);
    assert(wake_extension_set_config(
        wake, true, 0x1000, WAKE_EXTENSION_MODE_SET_STREAM_PATH));
    assert(wake_extension_arm(wake));
    tud_suspend_cb(true);
    const uint8_t trigger[] = {0x0f, 0x86, 0x10, 0x00};
    queue_cec_frame(trigger, sizeof(trigger));
    run_tasks(&runtime, 4);
    assert(remote_wake_count == 1);

    tud_umount_cb();
    assert(!wake_extension_status(wake).config.valid);
    assert(rp2040_usb_transport_output_idle(&runtime.usb_transport));
    puts("RP2040 production composition tests: PASS");
    return 0;
}
