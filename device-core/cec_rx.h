// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "adapter_state.h"
#include "cec_limits.h"

typedef enum {
    CEC_RX_ERROR_NONE = 0,
    CEC_RX_ERROR_MALFORMED = 1u << 0,
    CEC_RX_ERROR_TRUNCATED = 1u << 1,
    CEC_RX_ERROR_TOO_LONG = 1u << 2,
    CEC_RX_ERROR_OBSERVATION_OVERFLOW = 1u << 3,
} cec_rx_error_flag_t;

typedef struct {
    uint64_t start_timestamp_us;
    size_t length;
    uint8_t bytes[CEC_INJECTOR_CEC_MAX_FRAME_OCTETS];
    uint16_t acknowledged_octets;
    bool final_eom;
    uint8_t error_flags;
    bool overlapped_local_tx;
    bool observation_overflow;
} cec_rx_frame_t;

/* A completed electrical level interval supplied by the platform. */
typedef struct {
    uint64_t timestamp_us;
    uint32_t duration_us;
    bool previous_level_high;
} cec_rx_observation_t;

typedef struct {
    void (*on_frame)(const cec_rx_frame_t *frame, void *context);
    void (*on_follower_ack_decision)(bool should_ack, void *context);
    void (*on_follower_ack_cancel)(void *context);
    void *context;
} cec_rx_sink_t;

typedef struct {
    cec_rx_frame_t _frame;
    adapter_state_snapshot_t _frame_start_state;
    cec_rx_sink_t _sink;
    uint64_t _pending_low_start_us;
    uint64_t _last_observation_timestamp_us;
    uint32_t _pending_low_us;
    uint8_t _current_byte;
    uint8_t _bit_count;
    uint8_t _state;
    bool _current_eom;
    bool _pending_low;
    bool _local_tx_active;
} cec_rx_decoder_t;

void cec_rx_decoder_initialize(cec_rx_decoder_t *decoder,
                               const cec_rx_sink_t *sink);
void cec_rx_decoder_reset(cec_rx_decoder_t *decoder);
void cec_rx_decoder_consume(cec_rx_decoder_t *decoder,
                            const cec_rx_observation_t *observation);
void cec_rx_decoder_poll(cec_rx_decoder_t *decoder, uint64_t now_us);
void cec_rx_decoder_notify_observation_overflow(cec_rx_decoder_t *decoder);
void cec_rx_decoder_notify_local_tx_begin(cec_rx_decoder_t *decoder);
void cec_rx_decoder_notify_local_tx_end(cec_rx_decoder_t *decoder);
