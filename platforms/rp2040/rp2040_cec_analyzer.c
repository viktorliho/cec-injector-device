// SPDX-License-Identifier: Apache-2.0

#include "rp2040_cec_analyzer.h"

#include <limits.h>

#include "cec_gpio_config.h"
#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "pico/time.h"
#include "rp2040_cec_follower_ack.h"
#include "rp2040_cec_gpio.h"

enum {
    OBSERVATION_RING_CAPACITY = 1024,
    OBSERVATION_RING_MASK = OBSERVATION_RING_CAPACITY - 1,
};

_Static_assert((OBSERVATION_RING_CAPACITY & OBSERVATION_RING_MASK) == 0,
               "observation ring capacity must be a power of two");

static cec_rx_observation_t observation_ring[OBSERVATION_RING_CAPACITY];
static volatile uint32_t ring_write_index;
static volatile uint32_t ring_read_index;
static volatile uint32_t dropped_observation_count;
static uint64_t previous_edge_time_us;
static bool previous_level_high;

static void cec_gpio_irq(uint gpio, uint32_t events) {
    (void)events;
    if (gpio != CEC_INJECTOR_RP2040_CEC_GPIO) {
        return;
    }

    const uint64_t now_us = time_us_64();
    const bool current_level = rp2040_cec_gpio_read();
    rp2040_cec_follower_ack_on_edge(current_level, now_us);
    const uint64_t elapsed_us = now_us - previous_edge_time_us;
    previous_edge_time_us = now_us;

    const uint32_t write_index = ring_write_index;
    const uint32_t next_write_index =
        (write_index + 1u) & OBSERVATION_RING_MASK;
    if (next_write_index == ring_read_index) {
        dropped_observation_count++;
        previous_level_high = current_level;
        return;
    }

    observation_ring[write_index] = (cec_rx_observation_t){
        .timestamp_us = now_us,
        .duration_us = elapsed_us > UINT32_MAX ? UINT32_MAX
                                               : (uint32_t)elapsed_us,
        .previous_level_high = previous_level_high,
    };
    __dmb();
    ring_write_index = next_write_index;
    previous_level_high = current_level;
}

void rp2040_cec_analyzer_initialize(void) {
    ring_write_index = 0;
    ring_read_index = 0;
    dropped_observation_count = 0;
    previous_level_high = rp2040_cec_gpio_read();
    previous_edge_time_us = time_us_64();
    gpio_set_irq_enabled_with_callback(
        CEC_INJECTOR_RP2040_CEC_GPIO,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true,
        cec_gpio_irq);
}

bool rp2040_cec_analyzer_pop(cec_rx_observation_t *observation) {
    const uint32_t read_index = ring_read_index;
    if (read_index == ring_write_index) {
        return false;
    }
    __dmb();
    *observation = observation_ring[read_index];
    __dmb();
    ring_read_index = (read_index + 1u) & OBSERVATION_RING_MASK;
    return true;
}

void rp2040_cec_analyzer_set_irq_enabled(bool enabled) {
    gpio_set_irq_enabled(CEC_INJECTOR_RP2040_CEC_GPIO,
                         GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
                         enabled);
}

void rp2040_cec_analyzer_discard_pending(void) {
    ring_read_index = ring_write_index;
}

void rp2040_cec_analyzer_resynchronize(void) {
    rp2040_cec_gpio_release();
    gpio_acknowledge_irq(CEC_INJECTOR_RP2040_CEC_GPIO,
                         GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL);
    previous_level_high = rp2040_cec_gpio_read();
    previous_edge_time_us = time_us_64();
    rp2040_cec_analyzer_set_irq_enabled(true);
}

uint32_t rp2040_cec_analyzer_dropped_count(void) {
    return dropped_observation_count;
}
