// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "rp2040_usb_transport.h"

/* Initializes Pico stdio USB and returns the TinyUSB CDC carrier adapter. */
rp2040_usb_carrier_t rp2040_usb_tinyusb_initialize(void);
