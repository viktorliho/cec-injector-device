// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "pulse8_profile.h"
#include "pulse8_serial.h"

enum { RP2040_USB_TRANSPORT_OUTPUT_CAPACITY = 32 };

typedef struct {
    size_t (*read)(void *context, uint8_t *data, size_t capacity);
    size_t (*write)(void *context, const uint8_t *data, size_t length);
    void (*flush)(void *context);
    bool (*ready)(void *context);
    void *context;
} rp2040_usb_carrier_t;

typedef struct {
    uint16_t length;
    uint16_t offset;
    uint8_t data[PULSE8_SERIAL_MAX_ENCODED_OCTETS];
} rp2040_usb_output_slot_t;

typedef struct {
    pulse8_profile_t *profile;
    rp2040_usb_carrier_t carrier;
    pulse8_serial_parser_t parser;
    rp2040_usb_output_slot_t output[RP2040_USB_TRANSPORT_OUTPUT_CAPACITY];
    uint8_t output_read;
    uint8_t output_write;
    uint8_t output_count;
    uint64_t dispatch_now_us;
    bool attached;
    bool suspended;
} rp2040_usb_transport_t;

void rp2040_usb_transport_initialize(rp2040_usb_transport_t *transport,
                                     pulse8_profile_t *profile,
                                     rp2040_usb_carrier_t carrier);

void rp2040_usb_transport_mount(rp2040_usb_transport_t *transport);
void rp2040_usb_transport_unmount(rp2040_usb_transport_t *transport);
void rp2040_usb_transport_suspend(rp2040_usb_transport_t *transport);
void rp2040_usb_transport_resume(rp2040_usb_transport_t *transport);

bool rp2040_usb_transport_enqueue(rp2040_usb_transport_t *transport,
                                  const uint8_t *decoded,
                                  size_t decoded_length);
bool rp2040_usb_transport_enqueue_batch(
    rp2040_usb_transport_t *transport,
    const pulse8_profile_message_t *messages,
    size_t count);
bool rp2040_usb_transport_output_idle(
    const rp2040_usb_transport_t *transport);
bool rp2040_usb_transport_attached(const rp2040_usb_transport_t *transport);
bool rp2040_usb_transport_suspended(const rp2040_usb_transport_t *transport);

void rp2040_usb_transport_task(rp2040_usb_transport_t *transport,
                               uint64_t now_us);
