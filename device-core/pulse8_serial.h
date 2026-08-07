// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stddef.h>
#include <stdint.h>

enum {
    PULSE8_SERIAL_ESCAPE_CODE_FD = 0xfa,
    PULSE8_SERIAL_ESCAPE_CODE_FE = 0xfb,
    PULSE8_SERIAL_ESCAPE_CODE_FF = 0xfc,
    PULSE8_SERIAL_FRAME_ESCAPE = 0xfd,
    PULSE8_SERIAL_FRAME_END = 0xfe,
    PULSE8_SERIAL_FRAME_START = 0xff,
    PULSE8_SERIAL_MAX_DECODED_OCTETS = 64,
    PULSE8_SERIAL_MAX_ENCODED_OCTETS =
        2 * PULSE8_SERIAL_MAX_DECODED_OCTETS + 2,
};

typedef void (*pulse8_serial_frame_callback_t)(const uint8_t *frame,
                                                size_t frame_length,
                                                void *context);

/*
 * Storage for one streaming parser. Members prefixed with an underscore are
 * binding-local state and are not part of the module's behavioral API.
 */
typedef struct {
    uint8_t _decoded[PULSE8_SERIAL_MAX_DECODED_OCTETS];
    size_t _decoded_length;
    uint8_t _state;
} pulse8_serial_parser_t;

void pulse8_serial_parser_initialize(pulse8_serial_parser_t *parser);

/* Discards only binding-local partial-frame state. */
void pulse8_serial_parser_reset(pulse8_serial_parser_t *parser);

void pulse8_serial_parser_feed(pulse8_serial_parser_t *parser,
                               const uint8_t *octets,
                               size_t octet_count,
                               pulse8_serial_frame_callback_t on_frame,
                               void *context);

/* Returns the encoded length, or zero if the input or capacity is invalid. */
size_t pulse8_serial_encode(const uint8_t *decoded,
                            size_t decoded_length,
                            uint8_t *encoded,
                            size_t encoded_capacity);
