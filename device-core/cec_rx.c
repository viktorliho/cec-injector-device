// SPDX-License-Identifier: Apache-2.0

#include "cec_rx.h"

#include <string.h>

#include "cec_follower_ack.h"
#include "cec_rx_timings.h"

enum decoder_state {
    RX_WAIT_START,
    RX_DATA,
    RX_EOM,
    RX_ACK,
};

enum symbol {
    SYMBOL_INVALID,
    SYMBOL_ZERO,
    SYMBOL_ONE,
};

static bool in_range(uint32_t value, uint32_t minimum, uint32_t maximum) {
    return value >= minimum && value <= maximum;
}

static bool valid_start(uint32_t low_us, uint32_t high_us) {
    const uint32_t total_us = low_us + high_us;
    return in_range(low_us, CEC_RX_START_LOW_MIN_US, CEC_RX_START_LOW_MAX_US) &&
           in_range(high_us,
                    CEC_RX_START_HIGH_MIN_US,
                    CEC_RX_START_HIGH_MAX_US) &&
           in_range(total_us,
                    CEC_RX_START_TOTAL_MIN_US,
                    CEC_RX_START_TOTAL_MAX_US);
}

static enum symbol classify_bit(uint32_t low_us, uint32_t high_us) {
    const uint32_t total_us = low_us + high_us;
    if (!in_range(total_us,
                  CEC_RX_BIT_TOTAL_MIN_US,
                  CEC_RX_BIT_TOTAL_MAX_US)) {
        return SYMBOL_INVALID;
    }
    if (in_range(low_us, CEC_RX_BIT_0_LOW_MIN_US, CEC_RX_BIT_0_LOW_MAX_US) &&
        in_range(high_us,
                 CEC_RX_BIT_0_HIGH_MIN_US,
                 CEC_RX_BIT_0_HIGH_MAX_US)) {
        return SYMBOL_ZERO;
    }
    if (in_range(low_us, CEC_RX_BIT_1_LOW_MIN_US, CEC_RX_BIT_1_LOW_MAX_US) &&
        in_range(high_us,
                 CEC_RX_BIT_1_HIGH_MIN_US,
                 CEC_RX_BIT_1_HIGH_MAX_US)) {
        return SYMBOL_ONE;
    }
    return SYMBOL_INVALID;
}

static enum symbol classify_ack_low(uint32_t low_us) {
    if (in_range(low_us, CEC_RX_BIT_0_LOW_MIN_US, CEC_RX_BIT_0_LOW_MAX_US)) {
        return SYMBOL_ZERO;
    }
    if (in_range(low_us, CEC_RX_BIT_1_LOW_MIN_US, CEC_RX_BIT_1_LOW_MAX_US)) {
        return SYMBOL_ONE;
    }
    return SYMBOL_INVALID;
}

static bool frame_in_progress(const cec_rx_decoder_t *decoder) {
    return decoder->_state != RX_WAIT_START;
}

static void cancel_ack(cec_rx_decoder_t *decoder) {
    if (decoder->_sink.on_follower_ack_cancel != NULL) {
        decoder->_sink.on_follower_ack_cancel(decoder->_sink.context);
    }
}

static void clear_partial(cec_rx_decoder_t *decoder) {
    cancel_ack(decoder);
    memset(&decoder->_frame, 0, sizeof(decoder->_frame));
    decoder->_state = RX_WAIT_START;
    decoder->_current_byte = 0;
    decoder->_bit_count = 0;
    decoder->_current_eom = false;
    decoder->_pending_low = false;
}

static void start_frame(cec_rx_decoder_t *decoder, uint64_t timestamp_us) {
    memset(&decoder->_frame, 0, sizeof(decoder->_frame));
    decoder->_frame.start_timestamp_us = timestamp_us;
    decoder->_frame_start_state = adapter_state_snapshot();
    decoder->_current_byte = 0;
    decoder->_bit_count = 0;
    decoder->_current_eom = false;
    decoder->_state = RX_DATA;
}

static void complete_ack(cec_rx_decoder_t *decoder, enum symbol symbol) {
    if (symbol == SYMBOL_INVALID) {
        clear_partial(decoder);
        return;
    }

    const size_t index = decoder->_frame.length;
    decoder->_frame.bytes[index] = decoder->_current_byte;
    if (symbol == SYMBOL_ZERO) {
        decoder->_frame.acknowledged_octets |= (uint16_t)(UINT16_C(1) << index);
    }
    decoder->_frame.length++;

    if (decoder->_current_eom) {
        decoder->_frame.final_eom = true;
        if (decoder->_sink.on_frame != NULL) {
            decoder->_sink.on_frame(&decoder->_frame, decoder->_sink.context);
        }
        clear_partial(decoder);
        return;
    }

    if (decoder->_frame.length == CEC_INJECTOR_CEC_MAX_FRAME_OCTETS) {
        clear_partial(decoder);
        return;
    }

    decoder->_current_byte = 0;
    decoder->_bit_count = 0;
    decoder->_state = RX_DATA;
}

static void process_symbol(cec_rx_decoder_t *decoder, enum symbol symbol) {
    if (symbol == SYMBOL_INVALID) {
        clear_partial(decoder);
        return;
    }

    switch ((enum decoder_state)decoder->_state) {
        case RX_DATA:
            decoder->_current_byte =
                (uint8_t)((decoder->_current_byte << 1) |
                          (symbol == SYMBOL_ONE ? 1u : 0u));
            decoder->_bit_count++;
            if (decoder->_bit_count == 8u) {
                const uint8_t destination =
                    decoder->_frame.length == 0
                        ? decoder->_current_byte & 0x0fu
                        : decoder->_frame.bytes[0] & 0x0fu;
                const bool should_ack = cec_follower_ack_should_ack(
                    destination, &decoder->_frame_start_state);
                if (decoder->_sink.on_follower_ack_decision != NULL) {
                    decoder->_sink.on_follower_ack_decision(
                        should_ack, decoder->_sink.context);
                }
                decoder->_state = RX_EOM;
            }
            break;
        case RX_EOM:
            decoder->_current_eom = symbol == SYMBOL_ONE;
            decoder->_state = RX_ACK;
            break;
        case RX_ACK:
            complete_ack(decoder, symbol);
            break;
        case RX_WAIT_START:
            break;
    }
}

static void process_pulse(cec_rx_decoder_t *decoder,
                          uint32_t low_us,
                          uint32_t high_us,
                          uint64_t low_start_us) {
    if (decoder->_state == RX_WAIT_START) {
        if (valid_start(low_us, high_us)) {
            start_frame(decoder, low_start_us);
        }
        return;
    }

    if (valid_start(low_us, high_us)) {
        clear_partial(decoder);
        start_frame(decoder, low_start_us);
        return;
    }

    process_symbol(decoder, classify_bit(low_us, high_us));
}

void cec_rx_decoder_initialize(cec_rx_decoder_t *decoder,
                               const cec_rx_sink_t *sink) {
    memset(decoder, 0, sizeof(*decoder));
    if (sink != NULL) {
        decoder->_sink = *sink;
    }
    decoder->_state = RX_WAIT_START;
}

void cec_rx_decoder_reset(cec_rx_decoder_t *decoder) {
    clear_partial(decoder);
    decoder->_last_observation_timestamp_us = 0;
    decoder->_local_tx_active = false;
}

void cec_rx_decoder_consume(cec_rx_decoder_t *decoder,
                            const cec_rx_observation_t *observation) {
    if (decoder->_local_tx_active) {
        return;
    }

    decoder->_last_observation_timestamp_us = observation->timestamp_us;
    if (!observation->previous_level_high) {
        if (decoder->_pending_low) {
            if (frame_in_progress(decoder)) {
                clear_partial(decoder);
            } else {
                decoder->_pending_low = false;
            }
        }
        decoder->_pending_low = true;
        decoder->_pending_low_us = observation->duration_us;
        decoder->_pending_low_start_us =
            observation->timestamp_us - observation->duration_us;
        return;
    }

    if (!decoder->_pending_low) {
        return;
    }
    const uint32_t low_us = decoder->_pending_low_us;
    const uint64_t low_start_us = decoder->_pending_low_start_us;
    decoder->_pending_low = false;
    process_pulse(decoder,
                  low_us,
                  observation->duration_us,
                  low_start_us);
}

void cec_rx_decoder_poll(cec_rx_decoder_t *decoder, uint64_t now_us) {
    if (decoder->_local_tx_active || !frame_in_progress(decoder)) {
        return;
    }

    if (decoder->_state == RX_ACK && decoder->_pending_low) {
        const uint64_t deadline =
            decoder->_pending_low_start_us + CEC_RX_BIT_TOTAL_MAX_US;
        if (now_us >= deadline) {
            const uint32_t low_us = decoder->_pending_low_us;
            decoder->_pending_low = false;
            complete_ack(decoder, classify_ack_low(low_us));
            return;
        }
    }

    if (decoder->_last_observation_timestamp_us != 0 &&
        now_us - decoder->_last_observation_timestamp_us >=
            CEC_RX_FRAME_IDLE_TIMEOUT_US) {
        clear_partial(decoder);
    }
}

void cec_rx_decoder_notify_observation_overflow(cec_rx_decoder_t *decoder) {
    if (frame_in_progress(decoder) || decoder->_pending_low) {
        clear_partial(decoder);
    }
}

void cec_rx_decoder_notify_local_tx_begin(cec_rx_decoder_t *decoder) {
    clear_partial(decoder);
    decoder->_local_tx_active = true;
}

void cec_rx_decoder_notify_local_tx_end(cec_rx_decoder_t *decoder) {
    clear_partial(decoder);
    decoder->_last_observation_timestamp_us = 0;
    decoder->_local_tx_active = false;
}
