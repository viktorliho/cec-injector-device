// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "cec_rx.h"
#include "cec_transport.h"
#include "cec_tx_result.h"

cec_tx_result_t rp2040_cec_transmit(
    const cec_transport_tx_frame_t *frame,
    cec_rx_decoder_t *rx_decoder);
