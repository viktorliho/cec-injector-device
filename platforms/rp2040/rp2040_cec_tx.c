// SPDX-License-Identifier: Apache-2.0

#include "rp2040_cec_tx.h"

#include <stdbool.h>

#include "cec_gpio_config.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/time.h"
#include "rp2040_cec_analyzer.h"
#include "rp2040_cec_gpio.h"

#ifndef CEC_INJECTOR_RP2040_ENABLE_TRANSMIT
#define CEC_INJECTOR_RP2040_ENABLE_TRANSMIT 1
#endif

enum {
    CEC_START_LOW_US = 3700,
    CEC_START_TOTAL_US = 4500,
    CEC_BIT_0_LOW_US = 1500,
    CEC_BIT_1_LOW_US = 600,
    CEC_BIT_TOTAL_US = 2400,
    CEC_SAMPLE_US = 850,
    CEC_EXPECT_HIGH_US = 1750,
    CEC_IDLE_REQUIRED_US = 5 * CEC_BIT_TOTAL_US,
    CEC_IDLE_TIMEOUT_US = 1000000,
};

/*
 * The profile idletime byte is compatibility state. This backend deliberately
 * retains the reference implementation's fixed 12 ms bus-idle requirement.
 */

#if CEC_INJECTOR_RP2040_ENABLE_TRANSMIT
static bool wait_for_idle_bus(void) {
    const uint64_t timeout = time_us_64() + CEC_IDLE_TIMEOUT_US;
    uint64_t high_since = 0;
    while (time_us_64() < timeout) {
        const uint64_t now_us = time_us_64();
        if (rp2040_cec_gpio_read()) {
            if (high_since == 0) {
                high_since = now_us;
            } else if (now_us - high_since >= CEC_IDLE_REQUIRED_US) {
                return true;
            }
        } else {
            high_since = 0;
        }
        busy_wait_us_32(50);
    }
    return false;
}

static void wait_until_us(uint64_t deadline_us) {
    busy_wait_until(from_us_since_boot(deadline_us));
}

static cec_tx_result_t transmit_start_bit(void) {
    const uint64_t start_us = time_us_64();
    rp2040_cec_gpio_drive_low();
    wait_until_us(start_us + CEC_START_LOW_US);
    rp2040_cec_gpio_release();
    wait_until_us(start_us + CEC_START_TOTAL_US);
    return rp2040_cec_gpio_read() ? CEC_TX_RESULT_OK
                                  : CEC_TX_RESULT_ARBITRATION_LOST;
}

static cec_tx_result_t transmit_data_bit(bool value) {
    const uint32_t low_us = value ? CEC_BIT_1_LOW_US : CEC_BIT_0_LOW_US;
    const uint64_t start_us = time_us_64();
    rp2040_cec_gpio_drive_low();
    wait_until_us(start_us + low_us);
    rp2040_cec_gpio_release();
    if (value) {
        wait_until_us(start_us + CEC_SAMPLE_US);
        if (!rp2040_cec_gpio_read()) {
            return CEC_TX_RESULT_ARBITRATION_LOST;
        }
    }
    wait_until_us(start_us + CEC_EXPECT_HIGH_US);
    if (!rp2040_cec_gpio_read()) {
        return CEC_TX_RESULT_ARBITRATION_LOST;
    }
    wait_until_us(start_us + CEC_BIT_TOTAL_US);
    return CEC_TX_RESULT_OK;
}

static cec_tx_result_t transmit_ack_slot(bool broadcast) {
    const uint64_t start_us = time_us_64();
    rp2040_cec_gpio_drive_low();
    wait_until_us(start_us + CEC_BIT_1_LOW_US);
    rp2040_cec_gpio_release();
    wait_until_us(start_us + CEC_SAMPLE_US);
    const bool sampled_high = rp2040_cec_gpio_read();
    const bool acknowledged = broadcast ? sampled_high : !sampled_high;
    wait_until_us(start_us + CEC_BIT_TOTAL_US);
    return acknowledged ? CEC_TX_RESULT_OK : CEC_TX_RESULT_NACK;
}
#endif

cec_tx_result_t rp2040_cec_transmit(
    const cec_transport_tx_frame_t *frame,
    cec_rx_decoder_t *rx_decoder) {
    if (frame == NULL || rx_decoder == NULL || frame->length == 0 ||
        frame->length > CEC_INJECTOR_CEC_MAX_FRAME_OCTETS) {
        rp2040_cec_gpio_release();
        return CEC_TX_RESULT_INTERNAL_ERROR;
    }

#if CEC_INJECTOR_RP2040_ENABLE_TRANSMIT
    const bool broadcast = (frame->bytes[0] & 0x0fu) == 0x0fu;
    cec_tx_result_t result = CEC_TX_RESULT_OK;
    rp2040_cec_gpio_release();
    if (!wait_for_idle_bus()) {
        rp2040_cec_gpio_release();
        return CEC_TX_RESULT_BUS_BUSY;
    }

    rp2040_cec_analyzer_set_irq_enabled(false);
    cec_rx_decoder_notify_local_tx_begin(rx_decoder);
    rp2040_cec_analyzer_discard_pending();
    gpio_acknowledge_irq(CEC_INJECTOR_RP2040_CEC_GPIO,
                         GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
    const uint32_t saved_interrupts = save_and_disable_interrupts();

    result = transmit_start_bit();
    for (size_t byte_index = 0;
         result == CEC_TX_RESULT_OK && byte_index < frame->length;
         ++byte_index) {
        for (int bit = 7; result == CEC_TX_RESULT_OK && bit >= 0; --bit) {
            result = transmit_data_bit(
                (frame->bytes[byte_index] & (1u << bit)) != 0);
        }
        if (result == CEC_TX_RESULT_OK) {
            result = transmit_data_bit(byte_index + 1u == frame->length);
        }
        if (result == CEC_TX_RESULT_OK) {
            result = transmit_ack_slot(broadcast);
        }
    }

    rp2040_cec_gpio_release();
    restore_interrupts(saved_interrupts);
    rp2040_cec_analyzer_resynchronize();
    cec_rx_decoder_notify_local_tx_end(rx_decoder);
    return result;
#else
    (void)frame;
    (void)rx_decoder;
    rp2040_cec_gpio_release();
    return CEC_TX_RESULT_DISABLED;
#endif
}
