// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "adapter_state.h"
#include "cec_limits.h"

static void require_defaults(void) {
    const adapter_state_snapshot_t state = adapter_state_snapshot();

    assert(!state.controlled_mode);
    assert(state.receive_enabled);
    assert(state.ack_mask == UINT16_C(0x0000));
    assert(state.logical_address_mask == UINT16_C(0x0000));
    assert(state.default_logical_address == UINT8_C(0x0f));
    assert(state.physical_address == UINT16_C(0xffff));
}

int main(void) {
    assert(CEC_INJECTOR_CEC_MAX_FRAME_OCTETS == 16);

    adapter_state_initialize();
    require_defaults();

    adapter_state_set_controlled(true);
    adapter_state_set_ack_mask(UINT16_C(0xffff));
    adapter_state_set_logical_address_mask(UINT16_C(0x8001));
    adapter_state_set_default_logical_address(UINT8_C(0x04));
    adapter_state_set_physical_address(UINT16_C(0x1234));

    adapter_state_snapshot_t state = adapter_state_snapshot();
    assert(state.controlled_mode);
    assert(state.receive_enabled);
    assert(state.ack_mask == UINT16_C(0xffff));
    assert(state.logical_address_mask == UINT16_C(0x8001));
    assert(state.default_logical_address == UINT8_C(0x04));
    assert(state.physical_address == UINT16_C(0x1234));

    adapter_state_set_logical_address_mask(UINT16_C(0xffff));
    adapter_state_set_physical_address(UINT16_C(0x0000));
    state = adapter_state_snapshot();
    assert(state.logical_address_mask == UINT16_C(0xffff));
    assert(state.physical_address == UINT16_C(0x0000));

    adapter_state_reset();
    require_defaults();

    return 0;
}
