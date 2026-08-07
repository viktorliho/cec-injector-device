/*
 * Derived from pico-sdk pico_stdio_usb/stdio_usb_descriptors.c.
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 * Copyright (c) 2019 Damien P. George
 * SPDX-License-Identifier: MIT
 */

#include "usb_identity.h"

#include "pico/unique_id.h"
#include "tusb.h"

enum {
    USB_INTERFACE_CDC = 0,
    USB_INTERFACE_COUNT = 2,
    USB_STRING_LANGUAGE = 0,
    USB_STRING_MANUFACTURER,
    USB_STRING_PRODUCT,
    USB_STRING_SERIAL,
    USB_STRING_CDC,
};

enum {
    USB_DESCRIPTOR_LENGTH = TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN,
    USB_CDC_ENDPOINT_COMMAND = 0x81,
    USB_CDC_ENDPOINT_OUT = 0x02,
    USB_CDC_ENDPOINT_IN = 0x82,
};

static const tusb_desc_device_t device_descriptor = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = CEC_INJECTOR_USB_VID,
    .idProduct = CEC_INJECTOR_USB_PID,
    .bcdDevice = 0x0100,
    .iManufacturer = USB_STRING_MANUFACTURER,
    .iProduct = USB_STRING_PRODUCT,
    .iSerialNumber = USB_STRING_SERIAL,
    .bNumConfigurations = 1,
};

static const uint8_t configuration_descriptor[USB_DESCRIPTOR_LENGTH] = {
    TUD_CONFIG_DESCRIPTOR(1,
                          USB_INTERFACE_COUNT,
                          USB_STRING_LANGUAGE,
                          USB_DESCRIPTOR_LENGTH,
                          TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP,
                          250),
    TUD_CDC_DESCRIPTOR(USB_INTERFACE_CDC,
                       USB_STRING_CDC,
                       USB_CDC_ENDPOINT_COMMAND,
                       8,
                       USB_CDC_ENDPOINT_OUT,
                       USB_CDC_ENDPOINT_IN,
                       64),
};

static char serial_string[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1];
static const char *const strings[] = {
    [USB_STRING_MANUFACTURER] = CEC_INJECTOR_USB_MANUFACTURER,
    [USB_STRING_PRODUCT] = CEC_INJECTOR_USB_PRODUCT,
    [USB_STRING_SERIAL] = serial_string,
    [USB_STRING_CDC] = "Board CDC",
};

const uint8_t *tud_descriptor_device_cb(void) {
    return (const uint8_t *)&device_descriptor;
}

const uint8_t *tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;
    return configuration_descriptor;
}

const uint16_t *tud_descriptor_string_cb(uint8_t index,
                                         uint16_t language_id) {
    (void)language_id;
    static uint16_t descriptor[20];
    uint8_t length;

    if (!serial_string[0]) {
        pico_get_unique_board_id_string(serial_string, sizeof(serial_string));
    }
    if (index == USB_STRING_LANGUAGE) {
        descriptor[1] = 0x0409;
        length = 1;
    } else {
        if (index >= sizeof(strings) / sizeof(strings[0])) {
            return NULL;
        }
        const char *const string = strings[index];
        for (length = 0; length < 19 && string[length]; ++length) {
            descriptor[1 + length] = (uint8_t)string[length];
        }
    }
    descriptor[0] = (uint16_t)((TUSB_DESC_STRING << 8) |
                               (2u * length + 2u));
    return descriptor;
}
