// SPDX-License-Identifier: Apache-2.0

#include "wake_extension.h"

#include <string.h>

static void clear_pending(wake_extension_t *wake) {
    wake->_match_pending = false;
    wake->_status.wake_consumed = false;
}

static void reset_status(wake_extension_t *wake) {
    memset(&wake->_status, 0, sizeof(wake->_status));
    wake->_status.config.physical_address = UINT16_MAX;
    wake->_suspend_start_us = 0;
    wake->_earliest_delivery_us = 0;
    wake->_match_pending = false;
}

void wake_extension_initialize(wake_extension_t *wake) {
    memset(wake, 0, sizeof(*wake));
    reset_status(wake);
}

void wake_extension_session_reset(wake_extension_t *wake) {
    reset_status(wake);
}

void wake_extension_set_delivery(wake_extension_t *wake,
                                 wake_extension_remote_wake_fn request,
                                 void *context) {
    wake->_request_remote_wake = request;
    wake->_delivery_context = context;
}

bool wake_extension_physical_address_valid(uint16_t address) {
    if (address == 0 || address == UINT16_MAX) {
        return false;
    }

    bool zero_seen = false;
    for (int shift = 12; shift >= 0; shift -= 4) {
        const uint8_t nibble = (uint8_t)((address >> shift) & 0x0fu);
        if (zero_seen && nibble != 0) {
            return false;
        }
        if (nibble == 0) {
            zero_seen = true;
        }
    }
    return true;
}

bool wake_extension_set_config(wake_extension_t *wake,
                               bool enabled,
                               uint16_t physical_address,
                               uint8_t mode) {
    if (mode != WAKE_EXTENSION_MODE_SET_STREAM_PATH ||
        !wake_extension_physical_address_valid(physical_address)) {
        return false;
    }

    wake->_status.config.valid = true;
    wake->_status.config.enabled = enabled;
    wake->_status.config.physical_address = physical_address;
    wake->_status.config.mode = mode;
    wake->_status.armed = false;
    clear_pending(wake);
    return true;
}

bool wake_extension_arm(wake_extension_t *wake) {
    if (!wake->_status.config.valid || !wake->_status.config.enabled ||
        wake->_status.transport_suspended) {
        return false;
    }
    wake->_status.armed = true;
    clear_pending(wake);
    return true;
}

void wake_extension_disarm(wake_extension_t *wake) {
    wake->_status.armed = false;
    clear_pending(wake);
}

void wake_extension_clear_last_reason(wake_extension_t *wake) {
    wake->_status.last_result = WAKE_EXTENSION_RESULT_NONE;
    wake->_status.last_wake_offset_us = 0;
    wake->_status.last_frame_length = 0;
    memset(wake->_status.last_frame, 0, sizeof(wake->_status.last_frame));
}

void wake_extension_transport_configured(wake_extension_t *wake) {
    wake->_status.transport_configured = true;
    wake->_status.transport_suspended = false;
    wake->_status.remote_wake_authorized = false;
    clear_pending(wake);
}

void wake_extension_transport_unconfigured(wake_extension_t *wake) {
    wake_extension_session_reset(wake);
}

void wake_extension_suspend_begin(wake_extension_t *wake,
                                  bool remote_wake_authorized,
                                  uint64_t timestamp_us) {
    wake->_status.transport_suspended = true;
    wake->_status.remote_wake_authorized = remote_wake_authorized;
    clear_pending(wake);
    wake->_status.suspend_cycles++;
    wake->_suspend_start_us = timestamp_us;
    wake->_earliest_delivery_us =
        timestamp_us + WAKE_EXTENSION_MINIMUM_DELIVERY_DELAY_US;
}

void wake_extension_set_remote_wake_authorized(wake_extension_t *wake,
                                               bool authorized) {
    wake->_status.remote_wake_authorized = authorized;
}

void wake_extension_resume(wake_extension_t *wake) {
    wake->_status.transport_suspended = false;
    wake->_status.remote_wake_authorized = false;
    clear_pending(wake);
}

static bool frame_matches(const wake_extension_t *wake,
                          const cec_rx_frame_t *frame) {
    if (frame->length != 4 || !frame->final_eom ||
        frame->error_flags != CEC_RX_ERROR_NONE ||
        frame->overlapped_local_tx || frame->observation_overflow) {
        return false;
    }
    if (frame->bytes[0] != 0x0fu || frame->bytes[1] != 0x86u) {
        return false;
    }
    const uint16_t physical_address =
        (uint16_t)((uint16_t)frame->bytes[2] << 8) | frame->bytes[3];
    return physical_address == wake->_status.config.physical_address;
}

static bool eligible(const wake_extension_t *wake) {
    return wake->_status.config.valid && wake->_status.config.enabled &&
           wake->_status.armed && wake->_status.transport_suspended &&
           wake->_status.remote_wake_authorized &&
           !wake->_status.wake_consumed;
}

void wake_extension_offer_frame(wake_extension_t *wake,
                                const cec_rx_frame_t *frame) {
    if (frame == NULL) {
        return;
    }
    if (wake->_status.transport_suspended &&
        frame->start_timestamp_us < wake->_suspend_start_us) {
        wake->_status.rejected_frames++;
        return;
    }

    const bool matches = frame_matches(wake, frame);
    if (wake->_status.wake_consumed || wake->_match_pending) {
        if (matches) {
            wake->_status.duplicate_suppressed++;
        }
        return;
    }
    if (!matches) {
        if (wake->_status.transport_suspended) {
            wake->_status.rejected_frames++;
        }
        return;
    }
    if (!eligible(wake)) {
        wake->_status.rejected_frames++;
        return;
    }

    wake->_status.matching_frames++;
    wake->_status.last_frame_length = 4;
    memcpy(wake->_status.last_frame, frame->bytes, 4);
    const uint64_t offset = frame->start_timestamp_us - wake->_suspend_start_us;
    wake->_status.last_wake_offset_us =
        offset > UINT32_MAX ? UINT32_MAX : (uint32_t)offset;
    wake->_match_pending = true;
}

void wake_extension_poll(wake_extension_t *wake, uint64_t now_us) {
    if (!wake->_match_pending || now_us < wake->_earliest_delivery_us) {
        return;
    }
    wake->_match_pending = false;

    if (!eligible(wake)) {
        wake->_status.rejected_frames++;
        return;
    }

    wake->_status.wake_consumed = true;
    wake->_status.remote_wakeup_requests++;
    const bool success = wake->_request_remote_wake != NULL &&
                         wake->_request_remote_wake(wake->_delivery_context);
    wake->_status.last_result = success
                                    ? WAKE_EXTENSION_RESULT_SUCCESS
                                    : WAKE_EXTENSION_RESULT_REMOTE_WAKE_FAILED;
}

uint8_t wake_extension_derived_state(const wake_extension_t *wake) {
    if (!wake->_status.config.valid || !wake->_status.config.enabled) {
        return 0;
    }
    if (wake->_status.wake_consumed) {
        return 4;
    }
    if (wake->_status.armed && wake->_status.transport_suspended &&
        wake->_status.remote_wake_authorized) {
        return 3;
    }
    if (wake->_status.armed) {
        return 2;
    }
    return 1;
}

wake_extension_status_t wake_extension_status(const wake_extension_t *wake) {
    return wake->_status;
}

#ifdef CEC_INJECTOR_TESTING
void wake_extension_test_set_counters(wake_extension_t *wake,
                                      uint32_t suspend_cycles,
                                      uint32_t matching_frames,
                                      uint32_t rejected_frames,
                                      uint32_t remote_wakeup_requests,
                                      uint32_t duplicate_suppressed) {
    wake->_status.suspend_cycles = suspend_cycles;
    wake->_status.matching_frames = matching_frames;
    wake->_status.rejected_frames = rejected_frames;
    wake->_status.remote_wakeup_requests = remote_wakeup_requests;
    wake->_status.duplicate_suppressed = duplicate_suppressed;
}
#endif
