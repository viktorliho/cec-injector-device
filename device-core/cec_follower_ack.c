// SPDX-License-Identifier: Apache-2.0

#include "cec_follower_ack.h"

bool cec_follower_ack_should_ack(
    uint8_t destination,
    const adapter_state_snapshot_t *frame_start_state) {
    return frame_start_state->controlled_mode &&
           frame_start_state->receive_enabled && destination < 0x0fu &&
           (frame_start_state->ack_mask &
            (uint16_t)(UINT16_C(1) << destination)) != 0;
}
