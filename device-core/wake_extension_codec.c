// SPDX-License-Identifier: Apache-2.0

#include "wake_extension_codec.h"

enum {
    COMMAND_ACCEPTED = 0x08,
    COMMAND_REJECTED = 0x09,
    GET_WAKE_CAPABILITIES = 0xe0,
    SET_WAKE_CONFIG = 0xe1,
    GET_WAKE_CONFIG = 0xe2,
    ARM_WAKE = 0xe3,
    DISARM_WAKE = 0xe4,
    GET_WAKE_STATUS = 0xe5,
    GET_LAST_WAKE_REASON = 0xe6,
    CLEAR_LAST_WAKE_REASON = 0xe7,
};

static void generic_response(uint8_t *response,
                             size_t *length,
                             bool accepted,
                             uint8_t command) {
    *length = 2;
    response[0] = accepted ? COMMAND_ACCEPTED : COMMAND_REJECTED;
    response[1] = command;
}

static void append_u32be(uint8_t *response, size_t *length, uint32_t value) {
    response[(*length)++] = (uint8_t)(value >> 24);
    response[(*length)++] = (uint8_t)(value >> 16);
    response[(*length)++] = (uint8_t)(value >> 8);
    response[(*length)++] = (uint8_t)value;
}

bool wake_extension_codec_dispatch(wake_extension_t *wake,
                                   const uint8_t *request,
                                   size_t request_length,
                                   uint8_t *response,
                                   size_t *response_length) {
    if (request_length == 0 || request[0] < GET_WAKE_CAPABILITIES ||
        request[0] > CLEAR_LAST_WAKE_REASON) {
        return false;
    }

    const uint8_t command = request[0];
    const size_t payload_length = request_length - 1u;
    if (command == GET_WAKE_CAPABILITIES && payload_length == 0) {
        *response_length = 4;
        response[0] = command;
        response[1] = WAKE_EXTENSION_VERSION;
        response[2] = WAKE_EXTENSION_SUPPORTED_MODES;
        response[3] = 0;
        return true;
    }

    if (command == SET_WAKE_CONFIG && payload_length == 5 &&
        request[1] == WAKE_EXTENSION_VERSION && request[2] <= 1u) {
        const uint16_t physical_address =
            (uint16_t)((uint16_t)request[3] << 8) | request[4];
        if (wake_extension_set_config(wake,
                                      request[2] != 0,
                                      physical_address,
                                      request[5])) {
            generic_response(response, response_length, true, command);
            return true;
        }
    } else if (command == GET_WAKE_CONFIG && payload_length == 0) {
        const wake_extension_status_t status = wake_extension_status(wake);
        *response_length = 7;
        response[0] = command;
        response[1] = WAKE_EXTENSION_VERSION;
        response[2] = status.config.valid ? 1u : 0u;
        response[3] = status.config.enabled ? 1u : 0u;
        response[4] = (uint8_t)(status.config.physical_address >> 8);
        response[5] = (uint8_t)status.config.physical_address;
        response[6] = status.config.mode;
        return true;
    } else if (command == ARM_WAKE && payload_length == 0) {
        generic_response(response,
                         response_length,
                         wake_extension_arm(wake),
                         command);
        return true;
    } else if (command == DISARM_WAKE && payload_length == 0) {
        wake_extension_disarm(wake);
        generic_response(response, response_length, true, command);
        return true;
    } else if (command == GET_WAKE_STATUS && payload_length == 0) {
        const wake_extension_status_t status = wake_extension_status(wake);
        uint8_t flags = 0;
        flags |= status.config.valid ? 0x01u : 0u;
        flags |= status.config.enabled ? 0x02u : 0u;
        flags |= status.armed ? 0x04u : 0u;
        flags |= status.transport_configured ? 0x08u : 0u;
        flags |= status.transport_suspended ? 0x10u : 0u;
        flags |= status.remote_wake_authorized ? 0x20u : 0u;
        flags |= status.wake_consumed ? 0x40u : 0u;
        *response_length = 0;
        response[(*response_length)++] = command;
        response[(*response_length)++] = WAKE_EXTENSION_VERSION;
        response[(*response_length)++] = wake_extension_derived_state(wake);
        response[(*response_length)++] = flags;
        response[(*response_length)++] =
            (uint8_t)(status.config.physical_address >> 8);
        response[(*response_length)++] = (uint8_t)status.config.physical_address;
        response[(*response_length)++] = status.config.mode;
        append_u32be(response, response_length, status.suspend_cycles);
        append_u32be(response, response_length, status.matching_frames);
        append_u32be(response, response_length, status.rejected_frames);
        append_u32be(response,
                     response_length,
                     status.remote_wakeup_requests);
        append_u32be(response, response_length, status.duplicate_suppressed);
        response[(*response_length)++] = (uint8_t)status.last_result;
        append_u32be(response, response_length, status.last_wake_offset_us);
        return true;
    } else if (command == GET_LAST_WAKE_REASON && payload_length == 0) {
        const wake_extension_status_t status = wake_extension_status(wake);
        *response_length = 0;
        response[(*response_length)++] = command;
        response[(*response_length)++] = WAKE_EXTENSION_VERSION;
        response[(*response_length)++] = (uint8_t)status.last_result;
        append_u32be(response, response_length, status.last_wake_offset_us);
        response[(*response_length)++] = status.last_frame_length;
        for (uint8_t index = 0; index < status.last_frame_length; ++index) {
            response[(*response_length)++] = status.last_frame[index];
        }
        return true;
    } else if (command == CLEAR_LAST_WAKE_REASON && payload_length == 0) {
        wake_extension_clear_last_reason(wake);
        generic_response(response, response_length, true, command);
        return true;
    }

    generic_response(response, response_length, false, command);
    return true;
}
