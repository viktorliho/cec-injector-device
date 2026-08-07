// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cec_transport.h"

enum { PULSE8_PROFILE_MAX_MESSAGE_OCTETS = 64 };

typedef struct {
    size_t length;
    uint8_t data[PULSE8_PROFILE_MAX_MESSAGE_OCTETS];
} pulse8_profile_message_t;

typedef struct {
    cec_transport_t _tx;
} pulse8_profile_t;

void pulse8_profile_initialize(pulse8_profile_t *profile);

/* Ends the attachment session and restores all volatile profile defaults. */
void pulse8_profile_session_reset(pulse8_profile_t *profile);

void pulse8_profile_poll(pulse8_profile_t *profile, uint64_t now_us);

bool pulse8_profile_dispatch(pulse8_profile_t *profile,
                             const uint8_t *request,
                             size_t request_length,
                             uint64_t now_us,
                             pulse8_profile_message_t *response);

bool pulse8_profile_take_committed_tx(pulse8_profile_t *profile,
                                      cec_transport_tx_frame_t *frame);
void pulse8_profile_complete_tx(pulse8_profile_t *profile,
                                cec_tx_result_t result);
bool pulse8_profile_take_async(pulse8_profile_t *profile,
                               pulse8_profile_message_t *message);
