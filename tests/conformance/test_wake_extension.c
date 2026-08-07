// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "pulse8_profile.h"
#include "pulse8_serial.h"
#include "wake_extension.h"

typedef struct {
    uint32_t calls;
    bool result;
} delivery_t;

static bool request_remote_wake(void *context) {
    delivery_t *const delivery = context;
    delivery->calls++;
    return delivery->result;
}

static pulse8_profile_message_t dispatch(pulse8_profile_t *profile,
                                         const uint8_t *request,
                                         size_t request_length) {
    pulse8_profile_message_t response;
    assert(pulse8_profile_dispatch(
        profile, request, request_length, 0, &response));
    return response;
}

static void require_response(pulse8_profile_t *profile,
                             const uint8_t *request,
                             size_t request_length,
                             const uint8_t *expected,
                             size_t expected_length) {
    const pulse8_profile_message_t response =
        dispatch(profile, request, request_length);
    assert(response.length == expected_length);
    assert(memcmp(response.data, expected, expected_length) == 0);
}

static void require_generic(pulse8_profile_t *profile,
                            const uint8_t *request,
                            size_t request_length,
                            bool accepted) {
    const uint8_t expected[] = {accepted ? 0x08 : 0x09, request[0]};
    require_response(profile,
                     request,
                     request_length,
                     expected,
                     sizeof(expected));
}

static cec_rx_frame_t trigger(uint64_t timestamp_us, uint16_t address) {
    const cec_rx_frame_t frame = {
        .start_timestamp_us = timestamp_us,
        .length = 4,
        .bytes = {0x0f, 0x86, (uint8_t)(address >> 8), (uint8_t)address},
        .final_eom = true,
    };
    return frame;
}

static wake_extension_t *wake(pulse8_profile_t *profile) {
    return pulse8_profile_wake_extension(profile);
}

static void configure(pulse8_profile_t *profile,
                      bool enabled,
                      uint16_t address) {
    const uint8_t request[] = {
        0xe1, 0x01, enabled ? 1 : 0,
        (uint8_t)(address >> 8), (uint8_t)address, 0x01,
    };
    require_generic(profile, request, sizeof(request), true);
}

static void arm(pulse8_profile_t *profile) {
    const uint8_t request[] = {0xe3};
    require_generic(profile, request, sizeof(request), true);
}

static pulse8_profile_message_t get_status(pulse8_profile_t *profile) {
    const uint8_t request[] = {0xe5};
    return dispatch(profile, request, sizeof(request));
}

static uint32_t read_u32be(const uint8_t *bytes) {
    return (uint32_t)bytes[0] << 24 | (uint32_t)bytes[1] << 16 |
           (uint32_t)bytes[2] << 8 | bytes[3];
}

static void test_capabilities_defaults_and_unknown(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    const uint8_t capabilities[] = {0xe0};
    const uint8_t expected_capabilities[] = {0xe0, 0x01, 0x01, 0x00};
    require_response(&profile,
                     capabilities,
                     sizeof(capabilities),
                     expected_capabilities,
                     sizeof(expected_capabilities));

    const uint8_t config[] = {0xe2};
    const uint8_t expected_config[] = {
        0xe2, 0x01, 0x00, 0x00, 0xff, 0xff, 0x00,
    };
    require_response(&profile,
                     config,
                     sizeof(config),
                     expected_config,
                     sizeof(expected_config));

    const uint8_t reason[] = {0xe6};
    const uint8_t expected_reason[] = {
        0xe6, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    };
    require_response(&profile,
                     reason,
                     sizeof(reason),
                     expected_reason,
                     sizeof(expected_reason));

    const uint8_t unknown[] = {0xe8};
    require_generic(&profile, unknown, sizeof(unknown), false);
    const uint8_t ping[] = {0x01};
    require_generic(&profile, ping, sizeof(ping), true);

    const uint8_t wrong_lengths[][2] = {
        {0xe0, 0x00}, {0xe1, 0x00}, {0xe2, 0x00}, {0xe3, 0x00},
        {0xe4, 0x00}, {0xe5, 0x00}, {0xe6, 0x00}, {0xe7, 0x00},
    };
    for (size_t index = 0;
         index < sizeof(wrong_lengths) / sizeof(wrong_lengths[0]); ++index) {
        require_generic(&profile,
                        wrong_lengths[index],
                        sizeof(wrong_lengths[index]),
                        false);
    }
}

static void test_invalid_e1_is_non_destructive(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    configure(&profile, true, 0x1200);
    arm(&profile);

    const uint8_t get_config[] = {0xe2};
    const pulse8_profile_message_t baseline_config =
        dispatch(&profile, get_config, sizeof(get_config));
    const pulse8_profile_message_t baseline_status = get_status(&profile);

    const struct {
        uint8_t data[7];
        size_t length;
    } invalid[] = {
        {{0xe1, 0x01, 0x01, 0x12, 0x00}, 5},
        {{0xe1, 0x02, 0x01, 0x12, 0x00, 0x01}, 6},
        {{0xe1, 0x01, 0x02, 0x12, 0x00, 0x01}, 6},
        {{0xe1, 0x01, 0x01, 0x00, 0x00, 0x01}, 6},
        {{0xe1, 0x01, 0x01, 0xff, 0xff, 0x01}, 6},
        {{0xe1, 0x01, 0x01, 0x10, 0x20, 0x01}, 6},
        {{0xe1, 0x01, 0x01, 0x12, 0x00, 0x02}, 6},
    };

    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]);
         ++index) {
        require_generic(&profile,
                        invalid[index].data,
                        invalid[index].length,
                        false);
        const pulse8_profile_message_t config_after =
            dispatch(&profile, get_config, sizeof(get_config));
        const pulse8_profile_message_t status_after = get_status(&profile);
        assert(config_after.length == baseline_config.length);
        assert(memcmp(config_after.data,
                      baseline_config.data,
                      baseline_config.length) == 0);
        assert(status_after.length == baseline_status.length);
        assert(memcmp(status_after.data,
                      baseline_status.data,
                      baseline_status.length) == 0);
    }

    configure(&profile, true, 0x2000);
    const pulse8_profile_message_t replaced = get_status(&profile);
    assert(replaced.data[2] == 1);
    assert((replaced.data[3] & 0x04u) == 0);
}

static void test_arm_disarm_and_states(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    const uint8_t arm_request[] = {0xe3};
    require_generic(&profile, arm_request, sizeof(arm_request), false);

    configure(&profile, false, 0x1000);
    require_generic(&profile, arm_request, sizeof(arm_request), false);
    assert(get_status(&profile).data[2] == 0);

    configure(&profile, true, 0x1000);
    pulse8_profile_message_t status = get_status(&profile);
    assert(status.length == 32 && status.data[2] == 1);
    assert(status.data[3] == 0x03);
    arm(&profile);
    arm(&profile);
    status = get_status(&profile);
    assert(status.data[2] == 2 && status.data[3] == 0x07);

    wake_extension_transport_configured(wake(&profile));
    status = get_status(&profile);
    assert(status.data[3] == 0x0f);
    wake_extension_suspend_begin(wake(&profile), true, 1000);
    status = get_status(&profile);
    assert(status.data[2] == 3 && status.data[3] == 0x3f);
    assert((status.data[3] & 0x80u) == 0);
    require_generic(&profile, arm_request, sizeof(arm_request), false);

    wake_extension_resume(wake(&profile));
    const uint8_t disarm[] = {0xe4};
    require_generic(&profile, disarm, sizeof(disarm), true);
    require_generic(&profile, disarm, sizeof(disarm), true);
    status = get_status(&profile);
    assert(status.data[2] == 1);
    assert((status.data[3] & 0x04u) == 0);
}

static void test_exact_trigger_delay_duplicate_and_results(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    delivery_t delivery = {.result = true};
    pulse8_profile_set_wake_delivery(&profile, request_remote_wake, &delivery);
    wake_extension_transport_configured(wake(&profile));
    configure(&profile, true, 0x1000);
    arm(&profile);
    wake_extension_suspend_begin(wake(&profile), true, 10000);

    cec_rx_frame_t frame = trigger(17000, 0x1000);
    wake_extension_offer_frame(wake(&profile), &frame);
    assert(wake_extension_status(wake(&profile)).matching_frames == 1);
    pulse8_profile_poll(&profile, 19999);
    assert(delivery.calls == 0);
    wake_extension_offer_frame(wake(&profile), &frame);
    assert(wake_extension_status(wake(&profile)).duplicate_suppressed == 1);
    pulse8_profile_poll(&profile, 20000);
    assert(delivery.calls == 1);

    pulse8_profile_message_t status = get_status(&profile);
    assert(status.data[2] == 4);
    assert((status.data[3] & 0x40u) != 0);
    assert(status.data[27] == WAKE_EXTENSION_RESULT_SUCCESS);
    assert(read_u32be(&status.data[28]) == 7000);
    wake_extension_offer_frame(wake(&profile), &frame);
    pulse8_profile_poll(&profile, 30000);
    assert(delivery.calls == 1);
    assert(wake_extension_status(wake(&profile)).duplicate_suppressed == 2);

    const uint8_t reason_request[] = {0xe6};
    const pulse8_profile_message_t reason =
        dispatch(&profile, reason_request, sizeof(reason_request));
    const uint8_t expected_reason[] = {
        0xe6, 0x01, 0x01, 0x00, 0x00, 0x1b, 0x58,
        0x04, 0x0f, 0x86, 0x10, 0x00,
    };
    assert(reason.length == sizeof(expected_reason));
    assert(memcmp(reason.data, expected_reason, sizeof(expected_reason)) == 0);

    wake_extension_resume(wake(&profile));
    wake_extension_suspend_begin(wake(&profile), true, 40000);
    delivery.result = false;
    frame = trigger(51000, 0x1000);
    wake_extension_offer_frame(wake(&profile), &frame);
    pulse8_profile_poll(&profile, 51000);
    assert(delivery.calls == 2);
    assert(wake_extension_status(wake(&profile)).last_result ==
           WAKE_EXTENSION_RESULT_REMOTE_WAKE_FAILED);
    const pulse8_profile_message_t failed_reason =
        dispatch(&profile, reason_request, sizeof(reason_request));
    assert(failed_reason.length == 12);
    assert(failed_reason.data[2] ==
           WAKE_EXTENSION_RESULT_REMOTE_WAKE_FAILED);
    assert(failed_reason.data[7] == 4);
    assert(memcmp(&failed_reason.data[8], frame.bytes, 4) == 0);
    pulse8_profile_poll(&profile, 60000);
    assert(delivery.calls == 2);
}

static void test_invalid_e1_preserves_pending_match(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    delivery_t delivery = {.result = true};
    pulse8_profile_set_wake_delivery(&profile, request_remote_wake, &delivery);
    configure(&profile, true, 0x1000);
    arm(&profile);
    wake_extension_suspend_begin(wake(&profile), true, 10000);
    const cec_rx_frame_t frame = trigger(15000, 0x1000);
    wake_extension_offer_frame(wake(&profile), &frame);

    const uint8_t invalid[] = {0xe1, 0x02, 0x01, 0x10, 0x00, 0x01};
    require_generic(&profile, invalid, sizeof(invalid), false);
    wake_extension_offer_frame(wake(&profile), &frame);
    assert(wake_extension_status(wake(&profile)).duplicate_suppressed == 1);
    pulse8_profile_poll(&profile, 20000);
    assert(delivery.calls == 1);
}

static void test_trigger_rejections_and_eligibility(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    delivery_t delivery = {.result = true};
    pulse8_profile_set_wake_delivery(&profile, request_remote_wake, &delivery);
    configure(&profile, true, 0x1000);

    cec_rx_frame_t frame = trigger(1000, 0x1000);
    wake_extension_offer_frame(wake(&profile), &frame);
    assert(wake_extension_status(wake(&profile)).rejected_frames == 1);

    arm(&profile);
    wake_extension_suspend_begin(wake(&profile), false, 10000);
    frame = trigger(21000, 0x1000);
    wake_extension_offer_frame(wake(&profile), &frame);
    assert(wake_extension_status(wake(&profile)).rejected_frames == 2);
    wake_extension_resume(wake(&profile));
    wake_extension_disarm(wake(&profile));
    wake_extension_suspend_begin(wake(&profile), true, 30000);
    frame.start_timestamp_us = 41000;
    wake_extension_offer_frame(wake(&profile), &frame);
    assert(wake_extension_status(wake(&profile)).rejected_frames == 3);

    wake_extension_resume(wake(&profile));
    configure(&profile, false, 0x1000);
    wake_extension_suspend_begin(wake(&profile), true, 45000);
    frame = trigger(46000, 0x1000);
    wake_extension_offer_frame(wake(&profile), &frame);
    assert(wake_extension_status(wake(&profile)).rejected_frames == 4);

    wake_extension_resume(wake(&profile));
    configure(&profile, true, 0x1000);
    arm(&profile);
    wake_extension_suspend_begin(wake(&profile), true, 50000);
    frame = trigger(49999, 0x1000);
    wake_extension_offer_frame(wake(&profile), &frame);

    const cec_rx_frame_t base = trigger(61000, 0x1000);
    cec_rx_frame_t invalid[] = {
        base, base, base, base, base, base, base,
    };
    invalid[0].bytes[0] = 0x4f;
    invalid[1].bytes[0] = 0x00;
    invalid[2].bytes[1] = 0x85;
    invalid[3].bytes[2] = 0x20;
    invalid[4].length = 3;
    invalid[5].final_eom = false;
    invalid[6].overlapped_local_tx = true;
    for (size_t index = 0; index < sizeof(invalid) / sizeof(invalid[0]);
         ++index) {
        wake_extension_offer_frame(wake(&profile), &invalid[index]);
    }
    cec_rx_frame_t overflow = base;
    overflow.observation_overflow = true;
    wake_extension_offer_frame(wake(&profile), &overflow);
    cec_rx_frame_t malformed = base;
    malformed.error_flags = CEC_RX_ERROR_MALFORMED;
    wake_extension_offer_frame(wake(&profile), &malformed);
    assert(delivery.calls == 0);
    assert(wake_extension_status(wake(&profile)).matching_frames == 0);
}

static void test_delivery_recheck_and_lifecycle(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    delivery_t delivery = {.result = true};
    pulse8_profile_set_wake_delivery(&profile, request_remote_wake, &delivery);
    wake_extension_transport_configured(wake(&profile));
    configure(&profile, true, 0x1000);
    arm(&profile);
    wake_extension_suspend_begin(wake(&profile), true, 10000);
    cec_rx_frame_t frame = trigger(15000, 0x1000);
    wake_extension_offer_frame(wake(&profile), &frame);
    wake_extension_set_remote_wake_authorized(wake(&profile), false);
    pulse8_profile_poll(&profile, 20000);
    assert(delivery.calls == 0);
    assert(wake_extension_status(wake(&profile)).rejected_frames == 1);

    wake_extension_resume(wake(&profile));
    wake_extension_status_t status = wake_extension_status(wake(&profile));
    assert(status.config.valid && status.armed);
    assert(!status.transport_suspended && !status.wake_consumed);

    pulse8_serial_parser_t parser;
    pulse8_serial_parser_initialize(&parser);
    const uint8_t partial[] = {0xff, 0xe2};
    pulse8_serial_parser_feed(&parser, partial, sizeof(partial), NULL, NULL);
    pulse8_serial_parser_reset(&parser);
    status = wake_extension_status(wake(&profile));
    assert(status.config.valid && status.armed);

    pulse8_profile_session_reset(&profile);
    status = wake_extension_status(wake(&profile));
    assert(!status.config.valid && !status.config.enabled && !status.armed);
    assert(status.config.physical_address == UINT16_MAX);
    assert(status.config.mode == 0 && !status.transport_configured);
    assert(status.suspend_cycles == 0 && status.last_result == 0);
    assert(delivery.calls == 0);

    wake_extension_transport_configured(wake(&profile));
    wake_extension_transport_unconfigured(wake(&profile));
    assert(!wake_extension_status(wake(&profile)).transport_configured);
}

static void test_counter_wrap_offset_saturation_and_e7(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    delivery_t delivery = {.result = true};
    pulse8_profile_set_wake_delivery(&profile, request_remote_wake, &delivery);
    configure(&profile, true, 0x1000);
    arm(&profile);
    wake_extension_test_set_counters(wake(&profile),
                                     UINT32_MAX,
                                     UINT32_MAX,
                                     UINT32_MAX,
                                     UINT32_MAX,
                                     UINT32_MAX);
    wake_extension_suspend_begin(wake(&profile), true, 1);
    assert(wake_extension_status(wake(&profile)).suspend_cycles == 0);

    cec_rx_frame_t wrong = trigger(10001, 0x2000);
    wake_extension_offer_frame(wake(&profile), &wrong);
    assert(wake_extension_status(wake(&profile)).rejected_frames == 0);

    const uint64_t timestamp = (uint64_t)UINT32_MAX + 2u;
    cec_rx_frame_t frame = trigger(timestamp, 0x1000);
    wake_extension_offer_frame(wake(&profile), &frame);
    assert(wake_extension_status(wake(&profile)).matching_frames == 0);
    wake_extension_offer_frame(wake(&profile), &frame);
    assert(wake_extension_status(wake(&profile)).duplicate_suppressed == 0);
    pulse8_profile_poll(&profile, timestamp);
    wake_extension_status_t status = wake_extension_status(wake(&profile));
    assert(status.remote_wakeup_requests == 0);
    assert(status.last_wake_offset_us == UINT32_MAX);
    assert(delivery.calls == 1);
    const pulse8_profile_message_t encoded_status = get_status(&profile);
    assert(encoded_status.length == 32);
    assert(read_u32be(&encoded_status.data[7]) == status.suspend_cycles);
    assert(read_u32be(&encoded_status.data[11]) == status.matching_frames);
    assert(read_u32be(&encoded_status.data[15]) == status.rejected_frames);
    assert(read_u32be(&encoded_status.data[19]) ==
           status.remote_wakeup_requests);
    assert(read_u32be(&encoded_status.data[23]) ==
           status.duplicate_suppressed);

    const uint8_t clear[] = {0xe7};
    require_generic(&profile, clear, sizeof(clear), true);
    const wake_extension_status_t cleared = wake_extension_status(wake(&profile));
    assert(cleared.config.valid && cleared.armed && cleared.wake_consumed);
    assert(cleared.suspend_cycles == status.suspend_cycles);
    assert(cleared.matching_frames == status.matching_frames);
    assert(cleared.remote_wakeup_requests == status.remote_wakeup_requests);
    assert(cleared.last_result == WAKE_EXTENSION_RESULT_NONE);
    assert(cleared.last_wake_offset_us == 0 && cleared.last_frame_length == 0);
}

typedef struct {
    pulse8_profile_t *profile;
    pulse8_profile_message_t response;
    size_t calls;
} composition_t;

static void dispatch_serial_frame(const uint8_t *frame,
                                  size_t length,
                                  void *context) {
    composition_t *const composition = context;
    assert(pulse8_profile_dispatch(composition->profile,
                                   frame,
                                   length,
                                   0,
                                   &composition->response));
    composition->calls++;
}

static void test_serial_profile_wake_composition(void) {
    pulse8_profile_t profile;
    pulse8_profile_initialize(&profile);
    pulse8_serial_parser_t parser;
    pulse8_serial_parser_initialize(&parser);
    composition_t composition = {.profile = &profile};
    const uint8_t encoded[] = {0xff, 0xe0, 0xfe};
    pulse8_serial_parser_feed(&parser,
                              encoded,
                              sizeof(encoded),
                              dispatch_serial_frame,
                              &composition);
    const uint8_t expected[] = {0xe0, 0x01, 0x01, 0x00};
    assert(composition.calls == 1);
    assert(composition.response.length == sizeof(expected));
    assert(memcmp(composition.response.data, expected, sizeof(expected)) == 0);
}

int main(void) {
    test_capabilities_defaults_and_unknown();
    test_invalid_e1_is_non_destructive();
    test_arm_disarm_and_states();
    test_exact_trigger_delay_duplicate_and_results();
    test_invalid_e1_preserves_pending_match();
    test_trigger_rejections_and_eligibility();
    test_delivery_recheck_and_lifecycle();
    test_counter_wrap_offset_saturation_and_e7();
    test_serial_profile_wake_composition();
    return 0;
}
