// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cec_rx.h"

void rp2040_cec_analyzer_initialize(void);
bool rp2040_cec_analyzer_pop(cec_rx_observation_t *observation);
void rp2040_cec_analyzer_set_irq_enabled(bool enabled);
void rp2040_cec_analyzer_discard_pending(void);
void rp2040_cec_analyzer_resynchronize(void);
uint32_t rp2040_cec_analyzer_dropped_count(void);
