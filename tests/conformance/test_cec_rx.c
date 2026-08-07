// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "adapter_state.h"
#include "cec_follower_ack.h"
#include "cec_rx.h"
#include "cec_rx_timings.h"
#include "pulse8_profile.h"

enum {
    CAPTURE_CAPACITY = 4,
    ACK_DECISION_CAPACITY = 2 * CEC_INJECTOR_CEC_MAX_FRAME_OCTETS,
};

typedef struct {
    cec_rx_frame_t frames[CAPTURE_CAPACITY];
    size_t frame_count;
    bool ack_decisions[ACK_DECISION_CAPACITY];
    size_t ack_decision_count;
    size_t ack_cancel_count;
} capture_t;

typedef struct {
    cec_rx_decoder_t decoder;
    capture_t capture;
    uint64_t time_us;
} fixture_t;

static void capture_frame(const cec_rx_frame_t *frame, void *context) {
    capture_t *const capture = context;
    assert(capture->frame_count < CAPTURE_CAPACITY);
    capture->frames[capture->frame_count++] = *frame;
}

static void capture_ack_decision(bool should_ack, void *context) {
    capture_t *const capture = context;
    assert(capture->ack_decision_count < ACK_DECISION_CAPACITY);
    capture->ack_decisions[capture->ack_decision_count++] = should_ack;
}

static void capture_ack_cancel(void *context) {
    capture_t *const capture = context;
    capture->ack_cancel_count++;
}

static void fixture_initialize(fixture_t *fixture) {
    memset(fixture, 0, sizeof(*fixture));
    adapter_state_reset();
    const cec_rx_sink_t sink = {
        .on_frame = capture_frame,
        .on_follower_ack_decision = capture_ack_decision,
        .on_follower_ack_cancel = capture_ack_cancel,
        .context = &fixture->capture,
    };
    cec_rx_decoder_initialize(&fixture->decoder, &sink);
    fixture->time_us = 100000;
}

static void emit_pulse(fixture_t *fixture,
                       uint32_t low_us,
                       uint32_t high_us) {
    cec_rx_observation_t observation = {
        .timestamp_us = fixture->time_us + low_us,
        .duration_us = low_us,
        .previous_level_high = false,
    };
    cec_rx_decoder_consume(&fixture->decoder, &observation);

    fixture->time_us += low_us + high_us;
    observation.timestamp_us = fixture->time_us;
    observation.duration_us = high_us;
    observation.previous_level_high = true;
    cec_rx_decoder_consume(&fixture->decoder, &observation);
}

static void emit_start(fixture_t *fixture) {
    emit_pulse(fixture, 3711, 789);
}

static void emit_bit(fixture_t *fixture, bool value) {
    emit_pulse(fixture, value ? 611u : 1511u, value ? 1789u : 889u);
}

static void emit_data_bits(fixture_t *fixture, uint8_t value) {
    for (int bit = 7; bit >= 0; --bit) {
        emit_bit(fixture, (value & (1u << bit)) != 0);
    }
}

static void emit_byte(fixture_t *fixture,
                      uint8_t value,
                      bool end_of_message,
                      bool acknowledged) {
    emit_data_bits(fixture, value);
    emit_bit(fixture, end_of_message);
    emit_bit(fixture, !acknowledged);
}

static void emit_frame(fixture_t *fixture,
                       const uint8_t *bytes,
                       size_t length,
                       uint16_t acknowledged_octets) {
    emit_start(fixture);
    for (size_t index = 0; index < length; ++index) {
        emit_byte(fixture,
                  bytes[index],
                  index + 1u == length,
                  (acknowledged_octets & (UINT16_C(1) << index)) != 0);
    }
}

static void test_polling_frame_and_ack_flag(void) {
    fixture_t fixture;
    fixture_initialize(&fixture);
    const uint8_t polling[] = {0x04};
    emit_frame(&fixture, polling, sizeof(polling), 0);
    assert(fixture.capture.frame_count == 1);
    const cec_rx_frame_t *const frame = &fixture.capture.frames[0];
    assert(frame->length == 1 && frame->bytes[0] == 0x04);
    assert(frame->final_eom && frame->acknowledged_octets == 0);

    pulse8_profile_message_t message;
    assert(pulse8_profile_encode_rx(frame, &message, 1) == 1);
    assert(message.length == 2);
    assert(message.data[0] == (uint8_t)(0x05 | 0x40 | 0x80));
    assert(message.data[1] == 0x04);

    fixture_initialize(&fixture);
    emit_frame(&fixture, polling, sizeof(polling), 1);
    assert(pulse8_profile_encode_rx(
               &fixture.capture.frames[0], &message, 1) == 1);
    assert(message.data[0] == (uint8_t)(0x05 | 0x80));
}

static void test_multibyte_async_representation(void) {
    fixture_t fixture;
    fixture_initialize(&fixture);
    const uint8_t bytes[] = {0x0f, 0x82, 0x10, 0x00};
    emit_frame(&fixture, bytes, sizeof(bytes), 1);
    assert(fixture.capture.frame_count == 1);

    pulse8_profile_message_t messages[4];
    assert(pulse8_profile_encode_rx(
               &fixture.capture.frames[0], messages, 4) == 4);
    const uint8_t expected_ids[] = {0x05, 0x06, 0x06, 0x86};
    for (size_t index = 0; index < sizeof(bytes); ++index) {
        assert(messages[index].length == 2);
        assert(messages[index].data[0] == expected_ids[index]);
        assert(messages[index].data[1] == bytes[index]);
    }
}

static void configure_ack(uint8_t destination) {
    adapter_state_set_controlled(true);
    adapter_state_set_ack_mask((uint16_t)(UINT16_C(1) << destination));
}

static void test_follower_ack_decisions(void) {
    adapter_state_snapshot_t state = {
        .controlled_mode = true,
        .receive_enabled = true,
        .ack_mask = UINT16_C(1) << 4,
    };
    assert(cec_follower_ack_should_ack(4, &state));
    assert(!cec_follower_ack_should_ack(5, &state));
    assert(!cec_follower_ack_should_ack(0x0f, &state));
    state.controlled_mode = false;
    assert(!cec_follower_ack_should_ack(4, &state));
    state.controlled_mode = true;
    state.receive_enabled = false;
    assert(!cec_follower_ack_should_ack(4, &state));

    fixture_t fixture;
    fixture_initialize(&fixture);
    configure_ack(4);
    const uint8_t direct[] = {0x04, 0x44};
    emit_frame(&fixture, direct, sizeof(direct), 0);
    assert(fixture.capture.ack_decision_count == 2);
    assert(fixture.capture.ack_decisions[0]);
    assert(fixture.capture.ack_decisions[1]);

    fixture_initialize(&fixture);
    configure_ack(5);
    emit_frame(&fixture, direct, sizeof(direct), 0);
    assert(fixture.capture.ack_decision_count == 2);
    assert(!fixture.capture.ack_decisions[0]);
    assert(!fixture.capture.ack_decisions[1]);

    fixture_initialize(&fixture);
    adapter_state_set_controlled(true);
    adapter_state_set_ack_mask(UINT16_MAX);
    const uint8_t broadcast[] = {0x0f};
    emit_frame(&fixture, broadcast, sizeof(broadcast), 0);
    assert(!fixture.capture.ack_decisions[0]);

    fixture_initialize(&fixture);
    adapter_state_set_ack_mask(UINT16_C(1) << 4);
    emit_frame(&fixture, direct, 1, 0);
    assert(!fixture.capture.ack_decisions[0]);

    fixture_initialize(&fixture);
    adapter_state_set_controlled(true);
    adapter_state_set_logical_address_mask(UINT16_C(1) << 4);
    emit_frame(&fixture, direct, 1, 0);
    assert(!fixture.capture.ack_decisions[0]);
}

static void test_frame_start_state_snapshot(void) {
    fixture_t fixture;
    fixture_initialize(&fixture);
    configure_ack(4);
    emit_start(&fixture);
    adapter_state_set_controlled(false);
    adapter_state_set_ack_mask(0);
    emit_byte(&fixture, 0x04, true, false);
    assert(fixture.capture.ack_decision_count == 1);
    assert(fixture.capture.ack_decisions[0]);

    fixture_initialize(&fixture);
    emit_start(&fixture);
    configure_ack(4);
    emit_byte(&fixture, 0x04, true, false);
    assert(fixture.capture.ack_decision_count == 1);
    assert(!fixture.capture.ack_decisions[0]);
}

static void test_malformed_truncated_and_recovery(void) {
    fixture_t fixture;
    fixture_initialize(&fixture);
    emit_start(&fixture);
    emit_pulse(&fixture, 900, 1500);
    assert(fixture.capture.frame_count == 0);

    const uint8_t valid[] = {0x04};
    emit_frame(&fixture, valid, sizeof(valid), 0);
    assert(fixture.capture.frame_count == 1);

    fixture_initialize(&fixture);
    emit_start(&fixture);
    for (int bit = 0; bit < 4; ++bit) {
        emit_bit(&fixture, false);
    }
    cec_rx_decoder_poll(&fixture.decoder,
                        fixture.time_us + CEC_RX_FRAME_IDLE_TIMEOUT_US);
    assert(fixture.capture.frame_count == 0);
    assert(fixture.capture.ack_cancel_count > 0);
    emit_frame(&fixture, valid, sizeof(valid), 0);
    assert(fixture.capture.frame_count == 1);
}

static void test_final_ack_low_can_complete_on_poll(void) {
    fixture_t fixture;
    fixture_initialize(&fixture);
    emit_start(&fixture);
    emit_data_bits(&fixture, 0x04);
    emit_bit(&fixture, true);

    const cec_rx_observation_t ack_low = {
        .timestamp_us = fixture.time_us + 1511,
        .duration_us = 1511,
        .previous_level_high = false,
    };
    cec_rx_decoder_consume(&fixture.decoder, &ack_low);
    cec_rx_decoder_poll(&fixture.decoder,
                        fixture.time_us + CEC_RX_BIT_TOTAL_MAX_US);
    assert(fixture.capture.frame_count == 1);
    assert(fixture.capture.frames[0].acknowledged_octets == 1);
}

static void test_overlength_overflow_and_local_tx(void) {
    fixture_t fixture;
    fixture_initialize(&fixture);
    uint8_t bytes[CEC_INJECTOR_CEC_MAX_FRAME_OCTETS];
    memset(bytes, 0x40, sizeof(bytes));
    emit_start(&fixture);
    for (size_t index = 0; index < sizeof(bytes); ++index) {
        emit_byte(&fixture, bytes[index], false, false);
    }
    emit_byte(&fixture, 0x41, true, false);
    assert(fixture.capture.frame_count == 0);

    const uint8_t valid[] = {0x04};
    emit_frame(&fixture, valid, sizeof(valid), 0);
    assert(fixture.capture.frame_count == 1);

    fixture_initialize(&fixture);
    emit_start(&fixture);
    emit_bit(&fixture, false);
    cec_rx_decoder_notify_observation_overflow(&fixture.decoder);
    emit_frame(&fixture, valid, sizeof(valid), 0);
    assert(fixture.capture.frame_count == 1);

    fixture_initialize(&fixture);
    emit_start(&fixture);
    emit_bit(&fixture, false);
    cec_rx_decoder_notify_local_tx_begin(&fixture.decoder);
    emit_frame(&fixture, valid, sizeof(valid), 0);
    assert(fixture.capture.frame_count == 0);
    cec_rx_decoder_notify_local_tx_end(&fixture.decoder);
    emit_frame(&fixture, valid, sizeof(valid), 0);
    assert(fixture.capture.frame_count == 1);
}

static void test_profile_suppresses_invalid_frames_atomically(void) {
    cec_rx_frame_t frame = {
        .length = 2,
        .bytes = {0x04, 0x44},
        .final_eom = true,
    };
    pulse8_profile_message_t messages[2];
    memset(messages, 0xa5, sizeof(messages));

    assert(pulse8_profile_encode_rx(&frame, messages, 1) == 0);
    assert(messages[0].length != 2);

    frame.final_eom = false;
    assert(pulse8_profile_encode_rx(&frame, messages, 2) == 0);
    frame.final_eom = true;
    frame.error_flags = CEC_RX_ERROR_MALFORMED;
    assert(pulse8_profile_encode_rx(&frame, messages, 2) == 0);
    frame.error_flags = CEC_RX_ERROR_NONE;
    frame.overlapped_local_tx = true;
    assert(pulse8_profile_encode_rx(&frame, messages, 2) == 0);
    frame.overlapped_local_tx = false;
    frame.observation_overflow = true;
    assert(pulse8_profile_encode_rx(&frame, messages, 2) == 0);
    frame.observation_overflow = false;
    frame.length = CEC_INJECTOR_CEC_MAX_FRAME_OCTETS + 1u;
    assert(pulse8_profile_encode_rx(&frame, messages, 2) == 0);
}

int main(void) {
    test_polling_frame_and_ack_flag();
    test_multibyte_async_representation();
    test_follower_ack_decisions();
    test_frame_start_state_snapshot();
    test_malformed_truncated_and_recovery();
    test_final_ack_low_can_complete_on_poll();
    test_overlength_overflow_and_local_tx();
    test_profile_suppresses_invalid_frames_atomically();
    return 0;
}
