// SPDX-License-Identifier: Apache-2.0

#include "rp2040_usb_lifecycle.h"

static rp2040_usb_transport_t *bound_transport;

void rp2040_usb_lifecycle_bind(rp2040_usb_transport_t *transport) {
    bound_transport = transport;
}

void tud_mount_cb(void) {
    if (bound_transport != NULL) {
        rp2040_usb_transport_mount(bound_transport);
    }
}

void tud_umount_cb(void) {
    if (bound_transport != NULL) {
        rp2040_usb_transport_unmount(bound_transport);
    }
}

void tud_suspend_cb(bool remote_wakeup_en) {
    /* Authorization is intentionally connected in M3-D3. */
    (void)remote_wakeup_en;
    if (bound_transport != NULL) {
        rp2040_usb_transport_suspend(bound_transport);
    }
}

void tud_resume_cb(void) {
    if (bound_transport != NULL) {
        rp2040_usb_transport_resume(bound_transport);
    }
}
