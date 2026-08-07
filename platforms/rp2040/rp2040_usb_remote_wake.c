// SPDX-License-Identifier: Apache-2.0

#include "rp2040_usb_remote_wake.h"

#include "tusb.h"

static bool request_remote_wake(void *context) {
    (void)context;
    return tud_remote_wakeup();
}

void rp2040_usb_remote_wake_initialize(
    rp2040_usb_remote_wake_t *binding,
    pulse8_profile_t *profile) {
    binding->wake = pulse8_profile_wake_extension(profile);
    pulse8_profile_set_wake_delivery(profile, request_remote_wake, binding);
}

void rp2040_usb_remote_wake_mount(rp2040_usb_remote_wake_t *binding) {
    wake_extension_transport_configured(binding->wake);
}

void rp2040_usb_remote_wake_suspend(rp2040_usb_remote_wake_t *binding,
                                    bool authorized,
                                    uint64_t timestamp_us) {
    wake_extension_suspend_begin(binding->wake, authorized, timestamp_us);
}

void rp2040_usb_remote_wake_resume(rp2040_usb_remote_wake_t *binding) {
    wake_extension_resume(binding->wake);
}
