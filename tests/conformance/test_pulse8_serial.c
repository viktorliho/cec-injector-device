// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "pulse8_serial.h"

enum { MAX_CAPTURED_FRAMES = 8 };

typedef struct {
    size_t count;
    size_t lengths[MAX_CAPTURED_FRAMES];
    uint8_t frames[MAX_CAPTURED_FRAMES][PULSE8_SERIAL_MAX_DECODED_OCTETS];
} frame_collector_t;

static void collect_frame(const uint8_t *frame,
                          size_t frame_length,
                          void *context) {
    frame_collector_t *const collector = context;
    assert(collector->count < MAX_CAPTURED_FRAMES);
    collector->lengths[collector->count] = frame_length;
    memcpy(collector->frames[collector->count], frame, frame_length);
    collector->count++;
}

static void require_frame(const frame_collector_t *collector,
                          size_t index,
                          const uint8_t *expected,
                          size_t expected_length) {
    assert(index < collector->count);
    assert(collector->lengths[index] == expected_length);
    assert(memcmp(collector->frames[index], expected, expected_length) == 0);
}

static void feed(pulse8_serial_parser_t *parser,
                 frame_collector_t *collector,
                 const uint8_t *octets,
                 size_t octet_count) {
    pulse8_serial_parser_feed(
        parser, octets, octet_count, collect_frame, collector);
}

static void test_simple_and_escaped_frames(void) {
    pulse8_serial_parser_t parser;
    pulse8_serial_parser_initialize(&parser);
    frame_collector_t collector = {0};

    const uint8_t stream[] = {
        0xff, 0x11, 0x22, 0xfe,
        0xff, 0xfd, 0xfa, 0xfd, 0xfb, 0xfd, 0xfc, 0xfe,
    };
    const uint8_t simple[] = {0x11, 0x22};
    const uint8_t escaped[] = {0xfd, 0xfe, 0xff};
    feed(&parser, &collector, stream, sizeof(stream));

    assert(collector.count == 2);
    require_frame(&collector, 0, simple, sizeof(simple));
    require_frame(&collector, 1, escaped, sizeof(escaped));
}

static void test_every_split_boundary(void) {
    const uint8_t encoded[] = {0xff, 0x11, 0xfd, 0xfa, 0x22, 0xfe};
    const uint8_t expected[] = {0x11, 0xfd, 0x22};

    for (size_t split = 0; split <= sizeof(encoded); ++split) {
        pulse8_serial_parser_t parser;
        pulse8_serial_parser_initialize(&parser);
        frame_collector_t collector = {0};

        feed(&parser, &collector, encoded, split);
        feed(&parser,
             &collector,
             encoded + split,
             sizeof(encoded) - split);

        assert(collector.count == 1);
        require_frame(&collector, 0, expected, sizeof(expected));
    }
}

static void test_multiple_adjacent_and_outside_garbage(void) {
    pulse8_serial_parser_t parser;
    pulse8_serial_parser_initialize(&parser);
    frame_collector_t collector = {0};

    const uint8_t stream[] = {
        0x10, 0xfe, 0xfd, 0x20,
        0xff, 0x31, 0xfe, 0xff, 0x32, 0x33, 0xfe,
    };
    const uint8_t first[] = {0x31};
    const uint8_t second[] = {0x32, 0x33};
    feed(&parser, &collector, stream, sizeof(stream));

    assert(collector.count == 2);
    require_frame(&collector, 0, first, sizeof(first));
    require_frame(&collector, 1, second, sizeof(second));
}

static void require_malformed_then_recovery(const uint8_t *malformed,
                                            size_t malformed_length) {
    pulse8_serial_parser_t parser;
    pulse8_serial_parser_initialize(&parser);
    frame_collector_t collector = {0};
    const uint8_t recovery[] = {0xff, 0x55, 0xfe};
    const uint8_t expected[] = {0x55};

    feed(&parser, &collector, malformed, malformed_length);
    assert(collector.count == 0);
    feed(&parser, &collector, recovery, sizeof(recovery));
    assert(collector.count == 1);
    require_frame(&collector, 0, expected, sizeof(expected));
}

static void test_empty_and_invalid_escapes(void) {
    const uint8_t empty[] = {0xff, 0xfe};
    const uint8_t escape_end[] = {0xff, 0x11, 0xfd, 0xfe};
    const uint8_t escape_escape[] = {0xff, 0x11, 0xfd, 0xfd, 0xfe};
    const uint8_t escape_arbitrary[] = {0xff, 0x11, 0xfd, 0x42, 0xfe};

    require_malformed_then_recovery(empty, sizeof(empty));
    require_malformed_then_recovery(escape_end, sizeof(escape_end));
    require_malformed_then_recovery(escape_escape, sizeof(escape_escape));
    require_malformed_then_recovery(escape_arbitrary,
                                    sizeof(escape_arbitrary));
}

static void test_resynchronization(void) {
    const struct {
        const uint8_t *stream;
        size_t stream_length;
    } cases[] = {
        {(const uint8_t[]){0xff, 0x11, 0xfd, 0xff, 0x21, 0xfe}, 6},
        {(const uint8_t[]){0xff, 0x11, 0x12, 0xff, 0x21, 0xfe}, 6},
    };
    const uint8_t expected[] = {0x21};

    for (size_t index = 0; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        pulse8_serial_parser_t parser;
        pulse8_serial_parser_initialize(&parser);
        frame_collector_t collector = {0};
        feed(&parser,
             &collector,
             cases[index].stream,
             cases[index].stream_length);
        assert(collector.count == 1);
        require_frame(&collector, 0, expected, sizeof(expected));
    }
}

static void test_decoded_size_limit(void) {
    pulse8_serial_parser_t parser;
    pulse8_serial_parser_initialize(&parser);
    frame_collector_t collector = {0};
    uint8_t exact[66];
    exact[0] = 0xff;
    memset(exact + 1, 0x44, PULSE8_SERIAL_MAX_DECODED_OCTETS);
    exact[65] = 0xfe;
    feed(&parser, &collector, exact, sizeof(exact));
    assert(collector.count == 1);
    assert(collector.lengths[0] == PULSE8_SERIAL_MAX_DECODED_OCTETS);

    pulse8_serial_parser_reset(&parser);
    memset(&collector, 0, sizeof(collector));
    uint8_t overflow[67];
    overflow[0] = 0xff;
    memset(overflow + 1, 0x45, PULSE8_SERIAL_MAX_DECODED_OCTETS + 1u);
    overflow[66] = 0xfe;
    feed(&parser, &collector, overflow, sizeof(overflow));
    assert(collector.count == 0);

    const uint8_t escaped_overflow[] = {0xfd, 0xfa, 0xfe};
    pulse8_serial_parser_reset(&parser);
    memset(&collector, 0, sizeof(collector));
    feed(&parser, &collector, exact, sizeof(exact) - 1u);
    feed(&parser, &collector, escaped_overflow, sizeof(escaped_overflow));
    assert(collector.count == 0);

    const uint8_t recovery[] = {0xff, 0x46, 0xfe};
    const uint8_t expected[] = {0x46};
    feed(&parser, &collector, recovery, sizeof(recovery));
    assert(collector.count == 1);
    require_frame(&collector, 0, expected, sizeof(expected));
}

static void test_parser_reset_discards_partial_state(void) {
    pulse8_serial_parser_t parser;
    pulse8_serial_parser_initialize(&parser);
    frame_collector_t collector = {0};
    const uint8_t partial[] = {0xff, 0x11, 0xfd};
    const uint8_t tail[] = {0xfa, 0x22, 0xfe};
    const uint8_t complete[] = {0xff, 0x33, 0xfe};
    const uint8_t expected[] = {0x33};

    feed(&parser, &collector, partial, sizeof(partial));
    pulse8_serial_parser_reset(&parser);
    feed(&parser, &collector, tail, sizeof(tail));
    assert(collector.count == 0);
    feed(&parser, &collector, complete, sizeof(complete));
    assert(collector.count == 1);
    require_frame(&collector, 0, expected, sizeof(expected));
}

static void test_canonical_encoder(void) {
    const uint8_t decoded[] = {0x11, 0xfd, 0xfe, 0xff, 0x22};
    const uint8_t expected[] = {
        0xff, 0x11, 0xfd, 0xfa, 0xfd, 0xfb,
        0xfd, 0xfc, 0x22, 0xfe,
    };
    uint8_t encoded[PULSE8_SERIAL_MAX_ENCODED_OCTETS];
    const size_t length = pulse8_serial_encode(
        decoded, sizeof(decoded), encoded, sizeof(encoded));
    assert(length == sizeof(expected));
    assert(memcmp(encoded, expected, sizeof(expected)) == 0);

    assert(pulse8_serial_encode(decoded, 0, encoded, sizeof(encoded)) == 0);
    uint8_t oversized[PULSE8_SERIAL_MAX_DECODED_OCTETS + 1u] = {0};
    assert(pulse8_serial_encode(oversized,
                                sizeof(oversized),
                                encoded,
                                sizeof(encoded)) == 0);
    assert(pulse8_serial_encode(decoded,
                                sizeof(decoded),
                                encoded,
                                sizeof(expected) - 1u) == 0);
}

int main(void) {
    test_simple_and_escaped_frames();
    test_every_split_boundary();
    test_multiple_adjacent_and_outside_garbage();
    test_empty_and_invalid_escapes();
    test_resynchronization();
    test_decoded_size_limit();
    test_parser_reset_discards_partial_state();
    test_canonical_encoder();
    return 0;
}
