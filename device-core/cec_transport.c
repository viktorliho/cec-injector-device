// SPDX-License-Identifier: Apache-2.0

#include "cec_transport.h"

#include <string.h>

enum tx_state {
    TX_IDLE,
    TX_ASSEMBLING,
    TX_COMMITTED,
    TX_IN_PROGRESS,
    TX_TERMINAL_PENDING,
};

enum {
    TRANSMIT_SUCCEEDED = 0x10,
    TRANSMIT_FAILED_LINE = 0x11,
    TRANSMIT_FAILED_ACK = 0x12,
    TRANSMIT_FAILED_TIMEOUT_DATA = 0x13,
    TRANSMIT_FAILED_TIMEOUT_LINE = 0x14,
};

static void reset_transaction(cec_transport_t *transport) {
    memset(transport, 0, sizeof(*transport));
    transport->_state = TX_IDLE;
    transport->_idle_time = CEC_TRANSPORT_DEFAULT_IDLE_TIME;
}

void cec_transport_initialize(cec_transport_t *transport) {
    reset_transaction(transport);
}

void cec_transport_reset(cec_transport_t *transport) {
    reset_transaction(transport);
}

void cec_transport_poll(cec_transport_t *transport, uint64_t now_us) {
    if (transport->_state == TX_ASSEMBLING &&
        now_us >= transport->_deadline_us) {
        reset_transaction(transport);
    }
}

bool cec_transport_set_idletime(cec_transport_t *transport, uint8_t value) {
    if (transport->_state != TX_IDLE) {
        return false;
    }
    transport->_idle_time = value;
    return true;
}

bool cec_transport_start(cec_transport_t *transport,
                         bool broadcast_ack_polarity,
                         uint64_t now_us) {
    if (transport->_state != TX_IDLE) {
        return false;
    }
    transport->_state = TX_ASSEMBLING;
    transport->_broadcast_ack_polarity = broadcast_ack_polarity;
    transport->_deadline_us = now_us + CEC_TRANSPORT_PARTIAL_TIMEOUT_US;
    return true;
}

bool cec_transport_append(cec_transport_t *transport,
                          uint8_t cec_octet,
                          bool end_of_message,
                          uint64_t now_us) {
    if (transport->_state != TX_ASSEMBLING ||
        transport->_length == CEC_INJECTOR_CEC_MAX_FRAME_OCTETS) {
        return false;
    }

    transport->_bytes[transport->_length++] = cec_octet;
    transport->_deadline_us = now_us + CEC_TRANSPORT_PARTIAL_TIMEOUT_US;

    if (!end_of_message) {
        return true;
    }

    const bool header_is_broadcast = (transport->_bytes[0] & 0x0fu) == 0x0fu;
    if (header_is_broadcast != transport->_broadcast_ack_polarity) {
        transport->_length--;
        return false;
    }

    transport->_state = TX_COMMITTED;
    return true;
}

bool cec_transport_take_committed(cec_transport_t *transport,
                                  cec_transport_tx_frame_t *frame) {
    if (transport->_state != TX_COMMITTED) {
        return false;
    }

    memcpy(frame->bytes, transport->_bytes, transport->_length);
    frame->length = transport->_length;
    frame->idle_time = transport->_idle_time;
    frame->broadcast_ack_polarity = transport->_broadcast_ack_polarity;
    transport->_state = TX_IN_PROGRESS;
    return true;
}

static uint8_t terminal_message_for(cec_tx_result_t result) {
    switch (result) {
        case CEC_TX_RESULT_OK:
            return TRANSMIT_SUCCEEDED;
        case CEC_TX_RESULT_NACK:
            return TRANSMIT_FAILED_ACK;
        case CEC_TX_RESULT_BUS_BUSY:
            return TRANSMIT_FAILED_TIMEOUT_LINE;
        case CEC_TX_RESULT_TIMEOUT:
            return TRANSMIT_FAILED_TIMEOUT_DATA;
        case CEC_TX_RESULT_ARBITRATION_LOST:
        case CEC_TX_RESULT_DISABLED:
        case CEC_TX_RESULT_LINE_ERROR:
        case CEC_TX_RESULT_INTERNAL_ERROR:
            return TRANSMIT_FAILED_LINE;
    }
    return TRANSMIT_FAILED_LINE;
}

void cec_transport_complete(cec_transport_t *transport,
                            cec_tx_result_t result) {
    if (transport->_state != TX_IN_PROGRESS) {
        return;
    }
    transport->_terminal_message = terminal_message_for(result);
    transport->_state = TX_TERMINAL_PENDING;
}

bool cec_transport_take_terminal(cec_transport_t *transport,
                                 uint8_t *message_id) {
    if (transport->_state != TX_TERMINAL_PENDING) {
        return false;
    }
    *message_id = transport->_terminal_message;
    reset_transaction(transport);
    return true;
}
