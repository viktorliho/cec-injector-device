// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdint.h>

#include "cec_rx.h"
#include "pulse8_profile.h"
#include "rp2040_usb_remote_wake.h"
#include "rp2040_usb_transport.h"

enum { RP2040_ADAPTER_RX_FRAME_QUEUE_CAPACITY = 8 };

typedef struct {
    pulse8_profile_t profile;
    cec_rx_decoder_t rx_decoder;
    rp2040_usb_transport_t usb_transport;
    rp2040_usb_remote_wake_t remote_wake;
    uint32_t analyzer_dropped;
    cec_rx_frame_t rx_frame_queue[RP2040_ADAPTER_RX_FRAME_QUEUE_CAPACITY];
    uint8_t rx_frame_read;
    uint8_t rx_frame_write;
    uint8_t rx_frame_count;
} rp2040_adapter_runtime_t;

/* Performs safe platform, core, transport, lifecycle, then USB initialization. */
void rp2040_adapter_runtime_initialize(rp2040_adapter_runtime_t *runtime);

/* Executes one bounded cooperative scheduling pass. */
void rp2040_adapter_runtime_task(rp2040_adapter_runtime_t *runtime,
                                 uint64_t now_us);
