// SPDX-License-Identifier: Apache-2.0

#include "rp2040_usb_tinyusb.h"

#include "tusb.h"

static size_t carrier_read(void *context, uint8_t *data, size_t capacity) {
    (void)context;
    const uint32_t available = tud_cdc_available();
    const uint32_t requested =
        available < capacity ? available : (uint32_t)capacity;
    return requested == 0 ? 0 : tud_cdc_read(data, requested);
}

static size_t carrier_write(void *context,
                            const uint8_t *data,
                            size_t length) {
    (void)context;
    const uint32_t available = tud_cdc_write_available();
    const uint32_t requested =
        available < length ? available : (uint32_t)length;
    return requested == 0 ? 0 : tud_cdc_write(data, requested);
}

static void carrier_flush(void *context) {
    (void)context;
    tud_cdc_write_flush();
}

static bool carrier_ready(void *context) {
    (void)context;
    return tud_cdc_connected();
}

rp2040_usb_carrier_t rp2040_usb_tinyusb_carrier(void) {
    return (rp2040_usb_carrier_t){
        .read = carrier_read,
        .write = carrier_write,
        .flush = carrier_flush,
        .ready = carrier_ready,
        .context = NULL,
    };
}

void rp2040_usb_tinyusb_start(void) {
    tusb_init();
}

void rp2040_usb_tinyusb_task(void) {
    tud_task();
}
