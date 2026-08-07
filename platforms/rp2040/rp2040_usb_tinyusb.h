// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "rp2040_usb_transport.h"

/* Returns the TinyUSB CDC carrier without starting USB runtime services. */
rp2040_usb_carrier_t rp2040_usb_tinyusb_carrier(void);

/* Starts TinyUSB after composition and lifecycle wiring are complete. */
void rp2040_usb_tinyusb_start(void);

/* Services the sole TinyUSB device stack owner from the cooperative loop. */
void rp2040_usb_tinyusb_task(void);
