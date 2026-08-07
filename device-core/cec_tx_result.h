// SPDX-License-Identifier: Apache-2.0

#pragma once

/* Platform-neutral completion categories supplied by a physical CEC driver. */
typedef enum {
    CEC_TX_RESULT_OK,
    CEC_TX_RESULT_BUS_BUSY,
    CEC_TX_RESULT_NACK,
    CEC_TX_RESULT_ARBITRATION_LOST,
    CEC_TX_RESULT_DISABLED,
    CEC_TX_RESULT_TIMEOUT,
    CEC_TX_RESULT_LINE_ERROR,
    CEC_TX_RESULT_INTERNAL_ERROR,
} cec_tx_result_t;
