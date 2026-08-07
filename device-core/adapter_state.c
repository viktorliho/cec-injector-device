// SPDX-License-Identifier: Apache-2.0

#include "adapter_state.h"

#include <string.h>

static adapter_state_snapshot_t state;

void adapter_state_initialize(void) {
    adapter_state_reset();
}

void adapter_state_reset(void) {
    memset(&state, 0, sizeof(state));
    state.receive_enabled = true;
    state.physical_address = 0xffffu;
    state.default_logical_address = 0x0fu;
}

void adapter_state_set_controlled(bool enabled) {
    state.controlled_mode = enabled;
}

void adapter_state_set_ack_mask(uint16_t mask) {
    state.ack_mask = mask;
}

void adapter_state_set_logical_address_mask(uint16_t mask) {
    state.logical_address_mask = mask;
}

void adapter_state_set_physical_address(uint16_t address) {
    state.physical_address = address;
}

void adapter_state_set_default_logical_address(uint8_t address) {
    state.default_logical_address = address;
}

adapter_state_snapshot_t adapter_state_snapshot(void) {
    return state;
}
