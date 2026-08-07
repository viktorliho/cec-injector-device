// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stdint.h>

void rp2040_cec_follower_ack_initialize(void);
void rp2040_cec_follower_ack_apply_decision(bool should_ack);
void rp2040_cec_follower_ack_cancel(void);
void rp2040_cec_follower_ack_on_edge(bool current_level,
                                     uint64_t timestamp_us);
void rp2040_cec_follower_ack_decision_sink(bool should_ack, void *context);
void rp2040_cec_follower_ack_cancel_sink(void *context);
