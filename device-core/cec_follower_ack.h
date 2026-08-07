// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "adapter_state.h"

bool cec_follower_ack_should_ack(
    uint8_t destination,
    const adapter_state_snapshot_t *frame_start_state);
