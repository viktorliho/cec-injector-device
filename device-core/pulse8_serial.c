// SPDX-License-Identifier: Apache-2.0

#include "pulse8_serial.h"

enum parser_state {
    PARSER_IDLE,
    PARSER_IN_FRAME,
    PARSER_ESCAPE_PENDING,
    PARSER_DISCARD,
};

static void start_frame(pulse8_serial_parser_t *parser) {
    parser->_decoded_length = 0;
    parser->_state = PARSER_IN_FRAME;
}

static void discard_frame(pulse8_serial_parser_t *parser) {
    parser->_decoded_length = 0;
    parser->_state = PARSER_DISCARD;
}

static void append_decoded(pulse8_serial_parser_t *parser, uint8_t octet) {
    if (parser->_decoded_length == PULSE8_SERIAL_MAX_DECODED_OCTETS) {
        discard_frame(parser);
        return;
    }

    parser->_decoded[parser->_decoded_length++] = octet;
    parser->_state = PARSER_IN_FRAME;
}

void pulse8_serial_parser_initialize(pulse8_serial_parser_t *parser) {
    pulse8_serial_parser_reset(parser);
}

void pulse8_serial_parser_reset(pulse8_serial_parser_t *parser) {
    parser->_decoded_length = 0;
    parser->_state = PARSER_IDLE;
}

void pulse8_serial_parser_feed(pulse8_serial_parser_t *parser,
                               const uint8_t *octets,
                               size_t octet_count,
                               pulse8_serial_frame_callback_t on_frame,
                               void *context) {
    for (size_t index = 0; index < octet_count; ++index) {
        const uint8_t octet = octets[index];

        if (octet == PULSE8_SERIAL_FRAME_START) {
            start_frame(parser);
            continue;
        }

        switch ((enum parser_state)parser->_state) {
            case PARSER_IDLE:
            case PARSER_DISCARD:
                break;

            case PARSER_IN_FRAME:
                if (octet == PULSE8_SERIAL_FRAME_END) {
                    if (parser->_decoded_length == 0) {
                        discard_frame(parser);
                    } else {
                        const size_t frame_length = parser->_decoded_length;
                        parser->_decoded_length = 0;
                        parser->_state = PARSER_IDLE;
                        on_frame(parser->_decoded, frame_length, context);
                    }
                } else if (octet == PULSE8_SERIAL_FRAME_ESCAPE) {
                    parser->_state = PARSER_ESCAPE_PENDING;
                } else {
                    append_decoded(parser, octet);
                }
                break;

            case PARSER_ESCAPE_PENDING:
                if (octet >= PULSE8_SERIAL_ESCAPE_CODE_FD &&
                    octet <= PULSE8_SERIAL_ESCAPE_CODE_FF) {
                    append_decoded(parser, (uint8_t)(octet + 3u));
                } else {
                    discard_frame(parser);
                }
                break;
        }
    }
}

size_t pulse8_serial_encode(const uint8_t *decoded,
                            size_t decoded_length,
                            uint8_t *encoded,
                            size_t encoded_capacity) {
    if (decoded == NULL || encoded == NULL || decoded_length == 0 ||
        decoded_length > PULSE8_SERIAL_MAX_DECODED_OCTETS) {
        return 0;
    }

    size_t required = 2;
    for (size_t index = 0; index < decoded_length; ++index) {
        required += decoded[index] >= PULSE8_SERIAL_FRAME_ESCAPE ? 2u : 1u;
    }
    if (encoded_capacity < required) {
        return 0;
    }

    size_t output_length = 0;
    encoded[output_length++] = PULSE8_SERIAL_FRAME_START;
    for (size_t index = 0; index < decoded_length; ++index) {
        const uint8_t octet = decoded[index];
        if (octet >= PULSE8_SERIAL_FRAME_ESCAPE) {
            encoded[output_length++] = PULSE8_SERIAL_FRAME_ESCAPE;
            encoded[output_length++] = (uint8_t)(octet - 3u);
        } else {
            encoded[output_length++] = octet;
        }
    }
    encoded[output_length++] = PULSE8_SERIAL_FRAME_END;

    return output_length;
}
