// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "pulse8_profile.h"

typedef struct {
    wake_extension_t *wake;
} rp2040_usb_remote_wake_t;

/* Connects the Wake core delivery callback to TinyUSB Remote Wake. */
void rp2040_usb_remote_wake_initialize(
    rp2040_usb_remote_wake_t *binding,
    pulse8_profile_t *profile);

void rp2040_usb_remote_wake_mount(rp2040_usb_remote_wake_t *binding);
void rp2040_usb_remote_wake_suspend(rp2040_usb_remote_wake_t *binding,
                                    bool authorized,
                                    uint64_t timestamp_us);
void rp2040_usb_remote_wake_resume(rp2040_usb_remote_wake_t *binding);
