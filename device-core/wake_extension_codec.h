// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "wake_extension.h"

bool wake_extension_codec_dispatch(wake_extension_t *wake,
                                   const uint8_t *request,
                                   size_t request_length,
                                   uint8_t *response,
                                   size_t *response_length);
