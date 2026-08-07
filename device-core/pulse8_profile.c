// SPDX-License-Identifier: Apache-2.0

#include "pulse8_profile.h"

#include "adapter_state.h"

enum {
    PING = 0x01,
    COMMAND_ACCEPTED = 0x08,
    COMMAND_REJECTED = 0x09,
    SET_ACK_MASK = 0x0a,
    TRANSMIT = 0x0b,
    TRANSMIT_EOM = 0x0c,
    TRANSMIT_IDLETIME = 0x0d,
    TRANSMIT_ACK_POLARITY = 0x0e,
    FIRMWARE_VERSION = 0x15,
    GET_BUILDDATE = 0x17,
    SET_CONTROLLED = 0x18,
    GET_DEFAULT_LOGICAL_ADDRESS = 0x1b,
    SET_DEFAULT_LOGICAL_ADDRESS = 0x1c,
    GET_LOGICAL_ADDRESS_MASK = 0x1d,
    SET_LOGICAL_ADDRESS_MASK = 0x1e,
    GET_PHYSICAL_ADDRESS = 0x1f,
    SET_PHYSICAL_ADDRESS = 0x20,
    GET_ADAPTER_TYPE = 0x28,
};

static void generic_response(pulse8_profile_message_t *response,
                             bool accepted,
                             uint8_t command) {
    response->length = 2;
    response->data[0] = accepted ? COMMAND_ACCEPTED : COMMAND_REJECTED;
    response->data[1] = command;
}

static uint16_t read_u16be(const uint8_t *bytes) {
    return (uint16_t)((uint16_t)bytes[0] << 8) | bytes[1];
}

static void write_u16be(pulse8_profile_message_t *response,
                        uint8_t command,
                        uint16_t value) {
    response->length = 3;
    response->data[0] = command;
    response->data[1] = (uint8_t)(value >> 8);
    response->data[2] = (uint8_t)value;
}

void pulse8_profile_initialize(pulse8_profile_t *profile) {
    adapter_state_initialize();
    cec_transport_initialize(&profile->_tx);
}

void pulse8_profile_session_reset(pulse8_profile_t *profile) {
    adapter_state_reset();
    cec_transport_reset(&profile->_tx);
}

void pulse8_profile_poll(pulse8_profile_t *profile, uint64_t now_us) {
    cec_transport_poll(&profile->_tx, now_us);
}

static bool dispatch_tx(pulse8_profile_t *profile,
                        const uint8_t *request,
                        size_t request_length,
                        uint64_t now_us,
                        pulse8_profile_message_t *response) {
    const uint8_t command = request[0];
    if (request_length != 2) {
        generic_response(response, false, command);
        return true;
    }

    bool accepted = false;
    switch (command) {
        case TRANSMIT_IDLETIME:
            accepted = cec_transport_set_idletime(&profile->_tx, request[1]);
            break;
        case TRANSMIT_ACK_POLARITY:
            accepted = request[1] <= 1u &&
                       cec_transport_start(&profile->_tx,
                                           request[1] != 0,
                                           now_us);
            break;
        case TRANSMIT:
        case TRANSMIT_EOM:
            accepted = cec_transport_append(&profile->_tx,
                                             request[1],
                                             command == TRANSMIT_EOM,
                                             now_us);
            break;
    }
    generic_response(response, accepted, command);
    return true;
}

bool pulse8_profile_dispatch(pulse8_profile_t *profile,
                             const uint8_t *request,
                             size_t request_length,
                             uint64_t now_us,
                             pulse8_profile_message_t *response) {
    if (request == NULL || response == NULL || request_length == 0) {
        return false;
    }

    cec_transport_poll(&profile->_tx, now_us);

    const uint8_t command = request[0];
    if (command >= TRANSMIT && command <= TRANSMIT_ACK_POLARITY) {
        return dispatch_tx(profile, request, request_length, now_us, response);
    }

    const size_t payload_length = request_length - 1u;
    const adapter_state_snapshot_t state = adapter_state_snapshot();
    switch (command) {
        case PING:
            if (payload_length == 0) {
                generic_response(response, true, command);
                return true;
            }
            break;

        case SET_ACK_MASK:
            if (payload_length == 2) {
                adapter_state_set_ack_mask(read_u16be(&request[1]));
                generic_response(response, true, command);
                return true;
            }
            break;

        case FIRMWARE_VERSION:
            if (payload_length == 0) {
                response->length = 3;
                response->data[0] = command;
                response->data[1] = 0x00;
                response->data[2] = 0x02;
                return true;
            }
            break;

        case GET_BUILDDATE:
            if (payload_length == 0) {
                response->length = 5;
                response->data[0] = command;
                response->data[1] = 0x6a;
                response->data[2] = 0x6e;
                response->data[3] = 0x88;
                response->data[4] = 0x80;
                return true;
            }
            break;

        case SET_CONTROLLED:
            if (payload_length == 1 && request[1] <= 1u) {
                adapter_state_set_controlled(request[1] != 0);
                generic_response(response, true, command);
                return true;
            }
            break;

        case GET_DEFAULT_LOGICAL_ADDRESS:
            if (payload_length == 0) {
                response->length = 2;
                response->data[0] = command;
                response->data[1] = state.default_logical_address;
                return true;
            }
            break;

        case SET_DEFAULT_LOGICAL_ADDRESS:
            if (payload_length == 1 && request[1] <= 0x0fu) {
                adapter_state_set_default_logical_address(request[1]);
                generic_response(response, true, command);
                return true;
            }
            break;

        case GET_LOGICAL_ADDRESS_MASK:
            if (payload_length == 0) {
                write_u16be(response, command, state.logical_address_mask);
                return true;
            }
            break;

        case SET_LOGICAL_ADDRESS_MASK:
            if (payload_length == 2) {
                adapter_state_set_logical_address_mask(read_u16be(&request[1]));
                generic_response(response, true, command);
                return true;
            }
            break;

        case GET_PHYSICAL_ADDRESS:
            if (payload_length == 0) {
                write_u16be(response, command, state.physical_address);
                return true;
            }
            break;

        case SET_PHYSICAL_ADDRESS:
            if (payload_length == 2) {
                adapter_state_set_physical_address(read_u16be(&request[1]));
                generic_response(response, true, command);
                return true;
            }
            break;

        case GET_ADAPTER_TYPE:
            if (payload_length == 0) {
                response->length = 2;
                response->data[0] = command;
                response->data[1] = 0x01;
                return true;
            }
            break;
    }

    generic_response(response, false, command);
    return true;
}

bool pulse8_profile_take_committed_tx(pulse8_profile_t *profile,
                                      cec_transport_tx_frame_t *frame) {
    return cec_transport_take_committed(&profile->_tx, frame);
}

void pulse8_profile_complete_tx(pulse8_profile_t *profile,
                                cec_tx_result_t result) {
    cec_transport_complete(&profile->_tx, result);
}

bool pulse8_profile_take_async(pulse8_profile_t *profile,
                               pulse8_profile_message_t *message) {
    uint8_t message_id;
    if (!cec_transport_take_terminal(&profile->_tx, &message_id)) {
        return false;
    }
    message->length = 1;
    message->data[0] = message_id;
    return true;
}

size_t pulse8_profile_encode_rx(const cec_rx_frame_t *frame,
                                pulse8_profile_message_t *messages,
                                size_t capacity) {
    if (frame == NULL || messages == NULL || frame->length == 0 ||
        frame->length > CEC_INJECTOR_CEC_MAX_FRAME_OCTETS ||
        frame->length > capacity || !frame->final_eom ||
        frame->error_flags != CEC_RX_ERROR_NONE ||
        frame->overlapped_local_tx || frame->observation_overflow) {
        return 0;
    }

    for (size_t index = 0; index < frame->length; ++index) {
        uint8_t message_id = index == 0 ? 0x05u : 0x06u;
        if (index + 1u == frame->length) {
            message_id |= 0x80u;
        }
        if (index == 0 && (frame->acknowledged_octets & UINT16_C(1)) == 0) {
            message_id |= 0x40u;
        }
        messages[index].length = 2;
        messages[index].data[0] = message_id;
        messages[index].data[1] = frame->bytes[index];
    }
    return frame->length;
}
