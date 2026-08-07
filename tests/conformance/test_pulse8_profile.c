// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "adapter_state.h"
#include "pulse8_profile.h"
#include "pulse8_serial.h"

enum {
    COMMAND_ACCEPTED = 0x08,
    COMMAND_REJECTED = 0x09,
};

static pulse8_profile_message_t dispatch(pulse8_profile_t *profile,
                                         const uint8_t *request,
                                         size_t request_length,
                                         uint64_t now_us) {
    pulse8_profile_message_t response;
    assert(pulse8_profile_dispatch(
        profile, request, request_length, now_us, &response));
    return response;
}

static void require_message(const pulse8_profile_message_t *message,
                            const uint8_t *expected,
                            size_t expected_length) {
    assert(message->length == expected_length);
    assert(memcmp(message->data, expected, expected_length) == 0);
}

static void require_generic(pulse8_profile_t *profile,
                            const uint8_t *request,
                            size_t request_length,
                            uint64_t now_us,
                            bool accepted) {
    const uint8_t expected[] = {
        accepted ? COMMAND_ACCEPTED : COMMAND_REJECTED,
        request[0],
    };
    const pulse8_profile_message_t response =
        dispatch(profile, request, request_length, now_us);
    require_message(&response, expected, sizeof(expected));
}

static void test_supported_commands_and_metadata(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);

    const struct {
        uint8_t command;
        uint8_t response[5];
        size_t response_length;
    } cases[] = {
        {0x01, {0x08, 0x01}, 2},
        {0x15, {0x15, 0x00, 0x02}, 3},
        {0x17, {0x17, 0x6a, 0x6e, 0x88, 0x80}, 5},
        {0x1b, {0x1b, 0x0f}, 2},
        {0x1d, {0x1d, 0x00, 0x00}, 3},
        {0x1f, {0x1f, 0xff, 0xff}, 3},
        {0x28, {0x28, 0x01}, 2},
    };

    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        const pulse8_profile_message_t response =
            dispatch(&profile, &cases[index].command, 1, 0);
        require_message(
            &response, cases[index].response, cases[index].response_length);
    }
}

static void test_adapter_state_commands(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    const uint8_t controlled[] = {0x18, 0x01};
    const uint8_t ack_mask[] = {0x0a, 0xab, 0xcd};
    const uint8_t default_address[] = {0x1c, 0x04};
    const uint8_t logical_mask[] = {0x1e, 0x80, 0x01};
    const uint8_t physical_address[] = {0x20, 0x12, 0x34};
    require_generic(&profile, controlled, sizeof(controlled), 0, true);
    require_generic(&profile, ack_mask, sizeof(ack_mask), 0, true);
    require_generic(&profile,
                    default_address,
                    sizeof(default_address),
                    0,
                    true);
    require_generic(&profile, logical_mask, sizeof(logical_mask), 0, true);
    require_generic(&profile,
                    physical_address,
                    sizeof(physical_address),
                    0,
                    true);

    const adapter_state_snapshot_t state = adapter_state_snapshot();
    assert(state.controlled_mode);
    assert(state.ack_mask == UINT16_C(0xabcd));
    assert(state.default_logical_address == UINT8_C(0x04));
    assert(state.logical_address_mask == UINT16_C(0x8001));
    assert(state.physical_address == UINT16_C(0x1234));

    const struct {
        uint8_t request;
        uint8_t response[3];
        size_t response_length;
    } getters[] = {
        {0x1b, {0x1b, 0x04}, 2},
        {0x1d, {0x1d, 0x80, 0x01}, 3},
        {0x1f, {0x1f, 0x12, 0x34}, 3},
    };
    for (size_t index = 0; index < sizeof(getters) / sizeof(getters[0]);
         ++index) {
        const pulse8_profile_message_t response =
            dispatch(&profile, &getters[index].request, 1, 0);
        require_message(&response,
                        getters[index].response,
                        getters[index].response_length);
    }

    const uint8_t full_logical[] = {0x1e, 0xff, 0xff};
    const uint8_t zero_physical[] = {0x20, 0x00, 0x00};
    require_generic(&profile, full_logical, sizeof(full_logical), 0, true);
    require_generic(&profile, zero_physical, sizeof(zero_physical), 0, true);
    const adapter_state_snapshot_t independent = adapter_state_snapshot();
    assert(independent.logical_address_mask == UINT16_MAX);
    assert(independent.physical_address == 0);
}

static void test_invalid_values_lengths_and_unknown(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);

    const uint8_t invalid_controlled[] = {0x18, 0x02};
    const uint8_t invalid_default[] = {0x1c, 0x10};
    require_generic(&profile,
                    invalid_controlled,
                    sizeof(invalid_controlled),
                    0,
                    false);
    require_generic(&profile,
                    invalid_default,
                    sizeof(invalid_default),
                    0,
                    false);

    const struct {
        uint8_t bytes[4];
        size_t length;
    } wrong_lengths[] = {
        {{0x01, 0x00}, 2}, {{0x0a, 0x00}, 2},
        {{0x0b}, 1},       {{0x0c, 0x00, 0x01}, 3},
        {{0x0d}, 1},       {{0x0e, 0x00, 0x01}, 3},
        {{0x15, 0x00}, 2}, {{0x17, 0x00}, 2},
        {{0x18}, 1},       {{0x1b, 0x00}, 2},
        {{0x1c}, 1},       {{0x1d, 0x00}, 2},
        {{0x1e, 0x00}, 2}, {{0x1f, 0x00}, 2},
        {{0x20, 0x00}, 2}, {{0x28, 0x00}, 2},
    };
    for (size_t index = 0;
         index < sizeof(wrong_lengths) / sizeof(wrong_lengths[0]); ++index) {
        require_generic(&profile,
                        wrong_lengths[index].bytes,
                        wrong_lengths[index].length,
                        0,
                        false);
    }

    const uint8_t unsupported[] = {0x16};
    require_generic(&profile, unsupported, sizeof(unsupported), 0, false);
}

static void start_tx(pulse8_profile_t *profile,
                     bool broadcast,
                     uint64_t now_us) {
    const uint8_t request[] = {0x0e, broadcast ? 1 : 0};
    require_generic(profile, request, sizeof(request), now_us, true);
}

static cec_transport_tx_frame_t commit_one(pulse8_profile_t *profile,
                                           uint8_t header,
                                           bool broadcast,
                                           uint64_t now_us) {
    start_tx(profile, broadcast, now_us);
    assert(!pulse8_profile_take_committed_tx(
        profile, &(cec_transport_tx_frame_t){0}));
    const uint8_t eom[] = {0x0c, header};
    require_generic(profile, eom, sizeof(eom), now_us + 1u, true);
    cec_transport_tx_frame_t frame;
    assert(pulse8_profile_take_committed_tx(profile, &frame));
    assert(!pulse8_profile_take_committed_tx(profile, &frame));
    return frame;
}

static void complete_and_require(pulse8_profile_t *profile,
                                 cec_tx_result_t result,
                                 uint8_t expected) {
    pulse8_profile_complete_tx(profile, result);
    pulse8_profile_message_t terminal;
    assert(pulse8_profile_take_async(profile, &terminal));
    const uint8_t expected_message[] = {expected};
    require_message(&terminal, expected_message, sizeof(expected_message));
    assert(!pulse8_profile_take_async(profile, &terminal));
}

static void test_tx_assembly_and_limits(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    start_tx(&profile, false, 100);

    for (uint8_t index = 0; index < 15; ++index) {
        const uint8_t segment[] = {0x0b, index == 0 ? 0x40 : index};
        require_generic(&profile, segment, sizeof(segment), 200 + index, true);
        assert(!pulse8_profile_take_committed_tx(
            &profile, &(cec_transport_tx_frame_t){0}));
    }
    const uint8_t eom[] = {0x0c, 0xaa};
    require_generic(&profile, eom, sizeof(eom), 300, true);
    cec_transport_tx_frame_t frame;
    assert(pulse8_profile_take_committed_tx(&profile, &frame));
    assert(frame.length == 16);
    assert(frame.bytes[0] == 0x40 && frame.bytes[15] == 0xaa);
    complete_and_require(&profile, CEC_TX_RESULT_OK, 0x10);

    start_tx(&profile, false, 400);
    for (uint8_t index = 0; index < 16; ++index) {
        const uint8_t segment[] = {0x0b, index == 0 ? 0x40 : index};
        require_generic(&profile, segment, sizeof(segment), 500 + index, true);
    }
    require_generic(&profile, eom, sizeof(eom), 600, false);
    assert(!pulse8_profile_take_committed_tx(&profile, &frame));
}

static void test_polarity_and_rejection_preservation(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    start_tx(&profile, true, 100);
    const uint8_t direct_eom[] = {0x0c, 0x40};
    require_generic(&profile, direct_eom, sizeof(direct_eom), 200, false);

    const uint8_t ping[] = {0x01};
    require_generic(&profile, ping, sizeof(ping), 250, true);
    const uint8_t malformed_segment[] = {0x0b};
    require_generic(&profile,
                    malformed_segment,
                    sizeof(malformed_segment),
                    300,
                    false);
    const uint8_t unsupported[] = {0x77};
    require_generic(&profile, unsupported, sizeof(unsupported), 350, false);
    const uint8_t broadcast_eom[] = {0x0c, 0x4f};
    require_generic(&profile, broadcast_eom, sizeof(broadcast_eom), 400, true);
    cec_transport_tx_frame_t frame;
    assert(pulse8_profile_take_committed_tx(&profile, &frame));
    assert(frame.length == 1 && frame.bytes[0] == 0x4f);
    complete_and_require(&profile, CEC_TX_RESULT_OK, 0x10);

    const uint8_t invalid_polarity[] = {0x0e, 0x02};
    require_generic(&profile,
                    invalid_polarity,
                    sizeof(invalid_polarity),
                    500,
                    false);
    frame = commit_one(&profile, 0x40, false, 600);
    assert(frame.length == 1 && !frame.broadcast_ack_polarity);
    complete_and_require(&profile, CEC_TX_RESULT_OK, 0x10);
}

static void test_idletime_timeout_and_reset(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    cec_transport_tx_frame_t frame = commit_one(&profile, 0x40, false, 0);
    assert(frame.idle_time == 0x03);
    complete_and_require(&profile, CEC_TX_RESULT_OK, 0x10);

    const uint8_t idletime[] = {0x0d, 0x7a};
    require_generic(&profile, idletime, sizeof(idletime), 10, true);
    start_tx(&profile, false, 20);
    require_generic(&profile, idletime, sizeof(idletime), 30, false);
    const uint8_t header[] = {0x0b, 0x40};
    require_generic(&profile, header, sizeof(header), 40, true);
    pulse8_profile_poll(&profile, 40 + CEC_TRANSPORT_PARTIAL_TIMEOUT_US - 1u);
    const uint8_t eom[] = {0x0c, 0x01};
    require_generic(&profile,
                    eom,
                    sizeof(eom),
                    40 + CEC_TRANSPORT_PARTIAL_TIMEOUT_US - 1u,
                    true);
    assert(pulse8_profile_take_committed_tx(&profile, &frame));
    assert(frame.idle_time == 0x7a);
    complete_and_require(&profile, CEC_TX_RESULT_OK, 0x10);

    require_generic(&profile, idletime, sizeof(idletime), 100, true);
    start_tx(&profile, false, 200);
    pulse8_profile_poll(&profile, 200 + CEC_TRANSPORT_PARTIAL_TIMEOUT_US);
    frame = commit_one(&profile, 0x40, false, 2000000);
    assert(frame.idle_time == 0x03);
    complete_and_require(&profile, CEC_TX_RESULT_OK, 0x10);
}

static void test_mismatch_refreshes_deadline(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    start_tx(&profile, true, 100);
    const uint8_t direct_eom[] = {0x0c, 0x40};
    require_generic(&profile, direct_eom, sizeof(direct_eom), 200, false);
    pulse8_profile_poll(&profile, 100 + CEC_TRANSPORT_PARTIAL_TIMEOUT_US);
    const uint8_t broadcast_eom[] = {0x0c, 0x4f};
    require_generic(&profile,
                    broadcast_eom,
                    sizeof(broadcast_eom),
                    100 + CEC_TRANSPORT_PARTIAL_TIMEOUT_US,
                    true);
}

static void test_dispatch_observes_assembly_expiry(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    start_tx(&profile, false, 100);
    const uint8_t segment[] = {0x0b, 0x40};
    require_generic(&profile,
                    segment,
                    sizeof(segment),
                    100 + CEC_TRANSPORT_PARTIAL_TIMEOUT_US,
                    false);
    pulse8_profile_message_t terminal;
    assert(!pulse8_profile_take_async(&profile, &terminal));
}

static void test_terminal_mapping_and_session_reset(void) {
    const struct {
        cec_tx_result_t result;
        uint8_t message;
    } cases[] = {
        {CEC_TX_RESULT_OK, 0x10},
        {CEC_TX_RESULT_ARBITRATION_LOST, 0x11},
        {CEC_TX_RESULT_DISABLED, 0x11},
        {CEC_TX_RESULT_LINE_ERROR, 0x11},
        {CEC_TX_RESULT_INTERNAL_ERROR, 0x11},
        {CEC_TX_RESULT_NACK, 0x12},
        {CEC_TX_RESULT_TIMEOUT, 0x13},
        {CEC_TX_RESULT_BUS_BUSY, 0x14},
    };
    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        pulse8_profile_t profile;
        pulse8_profile_initialize(&profile);
        (void)commit_one(&profile, 0x4f, true, 0);
        complete_and_require(&profile, cases[index].result, cases[index].message);
    }

    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    const uint8_t controlled[] = {0x18, 0x01};
    require_generic(&profile, controlled, sizeof(controlled), 0, true);
    (void)commit_one(&profile, 0x4f, true, 10);
    pulse8_profile_complete_tx(&profile, CEC_TX_RESULT_OK);
    pulse8_profile_session_reset(&profile);
    pulse8_profile_message_t message;
    assert(!pulse8_profile_take_async(&profile, &message));
    assert(!adapter_state_snapshot().controlled_mode);
    const uint8_t segment[] = {0x0b, 0x40};
    require_generic(&profile, segment, sizeof(segment), 20, false);
}

typedef struct {
    pulse8_profile_t *profile;
    size_t dispatch_count;
} composition_context_t;

static void dispatch_decoded(const uint8_t *frame,
                             size_t frame_length,
                             void *context) {
    composition_context_t *const composition = context;
    pulse8_profile_message_t response;
    assert(pulse8_profile_dispatch(composition->profile,
                                   frame,
                                   frame_length,
                                   0,
                                   &response));
    composition->dispatch_count++;
}

static void test_serial_composition_and_parser_reset_independence(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    const uint8_t controlled[] = {0x18, 0x01};
    require_generic(&profile, controlled, sizeof(controlled), 0, true);
    start_tx(&profile, false, 10);

    pulse8_serial_parser_t parser;
    pulse8_serial_parser_initialize(&parser);
    composition_context_t context = {&profile, 0};
    const uint8_t malformed[] = {0xff, 0x01, 0xfd, 0xfe};
    pulse8_serial_parser_feed(&parser,
                              malformed,
                              sizeof(malformed),
                              dispatch_decoded,
                              &context);
    assert(context.dispatch_count == 0);
    pulse8_serial_parser_reset(&parser);
    assert(adapter_state_snapshot().controlled_mode);

    const uint8_t eom[] = {0x0c, 0x40};
    require_generic(&profile, eom, sizeof(eom), 20, true);
    assert(pulse8_profile_take_committed_tx(
        &profile, &(cec_transport_tx_frame_t){0}));
}

int main(void) {
    test_supported_commands_and_metadata();
    test_adapter_state_commands();
    test_invalid_values_lengths_and_unknown();
    test_tx_assembly_and_limits();
    test_polarity_and_rejection_preservation();
    test_idletime_timeout_and_reset();
    test_mismatch_refreshes_deadline();
    test_dispatch_observes_assembly_expiry();
    test_terminal_mapping_and_session_reset();
    test_serial_composition_and_parser_reset_independence();
    return 0;
}
