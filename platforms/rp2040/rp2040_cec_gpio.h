// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>

void rp2040_cec_gpio_initialize(void);
void rp2040_cec_gpio_drive_low(void);
void rp2040_cec_gpio_release(void);
bool rp2040_cec_gpio_read(void);
