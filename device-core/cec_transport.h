// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cec_limits.h"
#include "cec_tx_result.h"

enum {
    CEC_TRANSPORT_PARTIAL_TIMEOUT_US = 1000000,
    CEC_TRANSPORT_DEFAULT_IDLE_TIME = 0x03,
};

typedef struct {
    uint8_t bytes[CEC_INJECTOR_CEC_MAX_FRAME_OCTETS];
    size_t length;
    uint8_t idle_time;
    bool broadcast_ack_polarity;
} cec_transport_tx_frame_t;

typedef struct {
    uint8_t _bytes[CEC_INJECTOR_CEC_MAX_FRAME_OCTETS];
    size_t _length;
    uint64_t _deadline_us;
    uint8_t _state;
    uint8_t _idle_time;
    uint8_t _terminal_message;
    bool _broadcast_ack_polarity;
} cec_transport_t;

void cec_transport_initialize(cec_transport_t *transport);
void cec_transport_reset(cec_transport_t *transport);
void cec_transport_poll(cec_transport_t *transport, uint64_t now_us);

bool cec_transport_set_idletime(cec_transport_t *transport, uint8_t value);
bool cec_transport_start(cec_transport_t *transport,
                         bool broadcast_ack_polarity,
                         uint64_t now_us);
bool cec_transport_append(cec_transport_t *transport,
                          uint8_t cec_octet,
                          bool end_of_message,
                          uint64_t now_us);

bool cec_transport_take_committed(cec_transport_t *transport,
                                  cec_transport_tx_frame_t *frame);
void cec_transport_complete(cec_transport_t *transport,
                            cec_tx_result_t result);
bool cec_transport_take_terminal(cec_transport_t *transport,
                                 uint8_t *message_id);
