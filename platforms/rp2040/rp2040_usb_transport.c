// SPDX-License-Identifier: Apache-2.0

#include "rp2040_usb_transport.h"

#include <string.h>

static void clear_output(rp2040_usb_transport_t *transport) {
    transport->output_read = 0;
    transport->output_write = 0;
    transport->output_count = 0;
}

void rp2040_usb_transport_initialize(rp2040_usb_transport_t *transport,
                                     pulse8_profile_t *profile,
                                     rp2040_usb_carrier_t carrier) {
    memset(transport, 0, sizeof(*transport));
    transport->profile = profile;
    transport->carrier = carrier;
    pulse8_serial_parser_initialize(&transport->parser);
}

void rp2040_usb_transport_mount(rp2040_usb_transport_t *transport) {
    if (!transport->attached) {
        pulse8_serial_parser_reset(&transport->parser);
        clear_output(transport);
    }
    transport->attached = true;
    transport->suspended = false;
}

void rp2040_usb_transport_unmount(rp2040_usb_transport_t *transport) {
    if (transport->attached) {
        pulse8_serial_parser_reset(&transport->parser);
        pulse8_profile_session_reset(transport->profile);
        clear_output(transport);
    }
    transport->attached = false;
    transport->suspended = false;
}

void rp2040_usb_transport_suspend(rp2040_usb_transport_t *transport) {
    if (transport->attached) {
        transport->suspended = true;
    }
}

void rp2040_usb_transport_resume(rp2040_usb_transport_t *transport) {
    if (transport->attached) {
        transport->suspended = false;
    }
}

static size_t output_free(const rp2040_usb_transport_t *transport) {
    return RP2040_USB_TRANSPORT_OUTPUT_CAPACITY - transport->output_count;
}

bool rp2040_usb_transport_enqueue(rp2040_usb_transport_t *transport,
                                  const uint8_t *decoded,
                                  size_t decoded_length) {
    if (!transport->attached || output_free(transport) == 0) {
        return false;
    }

    rp2040_usb_output_slot_t *const slot =
        &transport->output[transport->output_write];
    const size_t encoded_length = pulse8_serial_encode(
        decoded, decoded_length, slot->data, sizeof(slot->data));
    if (encoded_length == 0) {
        return false;
    }

    slot->length = (uint16_t)encoded_length;
    slot->offset = 0;
    transport->output_write = (uint8_t)(
        (transport->output_write + 1u) % RP2040_USB_TRANSPORT_OUTPUT_CAPACITY);
    transport->output_count++;
    return true;
}

bool rp2040_usb_transport_enqueue_batch(
    rp2040_usb_transport_t *transport,
    const pulse8_profile_message_t *messages,
    size_t count) {
    if (count == 0 || messages == NULL || count > output_free(transport)) {
        return false;
    }

    uint8_t encoded[PULSE8_SERIAL_MAX_ENCODED_OCTETS];
    for (size_t index = 0; index < count; ++index) {
        if (pulse8_serial_encode(messages[index].data,
                                 messages[index].length,
                                 encoded,
                                 sizeof(encoded)) == 0) {
            return false;
        }
    }
    for (size_t index = 0; index < count; ++index) {
        if (!rp2040_usb_transport_enqueue(transport,
                                          messages[index].data,
                                          messages[index].length)) {
            return false;
        }
    }
    return true;
}

bool rp2040_usb_transport_output_idle(
    const rp2040_usb_transport_t *transport) {
    return transport->output_count == 0;
}

bool rp2040_usb_transport_attached(const rp2040_usb_transport_t *transport) {
    return transport->attached;
}

bool rp2040_usb_transport_suspended(const rp2040_usb_transport_t *transport) {
    return transport->suspended;
}

static void on_decoded_frame(const uint8_t *frame,
                             size_t frame_length,
                             void *context) {
    rp2040_usb_transport_t *const transport = context;
    pulse8_profile_message_t response;
    if (pulse8_profile_dispatch(transport->profile,
                                frame,
                                frame_length,
                                transport->dispatch_now_us,
                                &response)) {
        (void)rp2040_usb_transport_enqueue(transport,
                                           response.data,
                                           response.length);
    }
}

static void receive_input(rp2040_usb_transport_t *transport,
                          uint64_t now_us) {
    transport->dispatch_now_us = now_us;
    while (output_free(transport) != 0) {
        uint8_t octet;
        if (transport->carrier.read(transport->carrier.context,
                                    &octet,
                                    1) != 1) {
            break;
        }
        pulse8_serial_parser_feed(&transport->parser,
                                  &octet,
                                  1,
                                  on_decoded_frame,
                                  transport);
    }
}

static void collect_async(rp2040_usb_transport_t *transport) {
    pulse8_profile_message_t message;
    while (output_free(transport) != 0 &&
           pulse8_profile_take_async(transport->profile, &message)) {
        (void)rp2040_usb_transport_enqueue(transport,
                                           message.data,
                                           message.length);
    }
}

static void transmit_output(rp2040_usb_transport_t *transport) {
    if (transport->output_count == 0) {
        return;
    }
    rp2040_usb_output_slot_t *const slot =
        &transport->output[transport->output_read];
    const size_t remaining = slot->length - slot->offset;
    const size_t written = transport->carrier.write(
        transport->carrier.context, slot->data + slot->offset, remaining);
    if (written > remaining) {
        return;
    }
    slot->offset = (uint16_t)(slot->offset + written);
    if (written != 0 && transport->carrier.flush != NULL) {
        transport->carrier.flush(transport->carrier.context);
    }
    if (slot->offset == slot->length) {
        transport->output_read = (uint8_t)(
            (transport->output_read + 1u) %
            RP2040_USB_TRANSPORT_OUTPUT_CAPACITY);
        transport->output_count--;
    }
}

void rp2040_usb_transport_task(rp2040_usb_transport_t *transport,
                               uint64_t now_us) {
    if (!transport->attached) {
        return;
    }
    pulse8_profile_poll(transport->profile, now_us);
    if (transport->suspended ||
        !transport->carrier.ready(transport->carrier.context)) {
        return;
    }
    transmit_output(transport);
    receive_input(transport, now_us);
    collect_async(transport);
}
