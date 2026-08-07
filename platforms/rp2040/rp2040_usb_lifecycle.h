// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "rp2040_usb_transport.h"

/* Binds the instance that receives TinyUSB attachment lifecycle callbacks. */
void rp2040_usb_lifecycle_bind(rp2040_usb_transport_t *transport);
