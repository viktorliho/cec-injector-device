// SPDX-License-Identifier: Apache-2.0

#include "rp2040_usb_lifecycle.h"

#include "pico/time.h"

static rp2040_usb_transport_t *bound_transport;
static rp2040_usb_remote_wake_t *bound_remote_wake;

void rp2040_usb_lifecycle_bind(rp2040_usb_transport_t *transport,
                               rp2040_usb_remote_wake_t *remote_wake) {
    bound_transport = transport;
    bound_remote_wake = remote_wake;
}

void tud_mount_cb(void) {
    if (bound_transport != NULL) {
        rp2040_usb_transport_mount(bound_transport);
    }
    if (bound_remote_wake != NULL) {
        rp2040_usb_remote_wake_mount(bound_remote_wake);
    }
}

void tud_umount_cb(void) {
    if (bound_transport != NULL) {
        rp2040_usb_transport_unmount(bound_transport);
    }
}

void tud_suspend_cb(bool remote_wakeup_en) {
    if (bound_transport != NULL) {
        rp2040_usb_transport_suspend(bound_transport);
    }
    if (bound_remote_wake != NULL) {
        rp2040_usb_remote_wake_suspend(bound_remote_wake,
                                       remote_wakeup_en,
                                       time_us_64());
    }
}

void tud_resume_cb(void) {
    if (bound_remote_wake != NULL) {
        rp2040_usb_remote_wake_resume(bound_remote_wake);
    }
    if (bound_transport != NULL) {
        rp2040_usb_transport_resume(bound_transport);
    }
}
