// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stdint.h>

/* Runtime adapter state exposed by the Pulse-Eight Compatibility Profile. */
typedef struct {
    bool controlled_mode;
    bool receive_enabled;
    uint16_t ack_mask;
    uint16_t logical_address_mask;
    uint16_t physical_address;
    uint8_t default_logical_address;
} adapter_state_snapshot_t;

void adapter_state_initialize(void);
void adapter_state_reset(void);

void adapter_state_set_controlled(bool enabled);
void adapter_state_set_ack_mask(uint16_t mask);
void adapter_state_set_logical_address_mask(uint16_t mask);
void adapter_state_set_physical_address(uint16_t address);
void adapter_state_set_default_logical_address(uint8_t address);

adapter_state_snapshot_t adapter_state_snapshot(void);
