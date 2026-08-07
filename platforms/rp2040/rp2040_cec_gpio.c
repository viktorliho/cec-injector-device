// SPDX-License-Identifier: Apache-2.0

#include "rp2040_cec_gpio.h"

#include "cec_gpio_config.h"
#include "hardware/gpio.h"

void rp2040_cec_gpio_initialize(void) {
    gpio_init(CEC_INJECTOR_RP2040_CEC_GPIO);
    rp2040_cec_gpio_release();
}

void rp2040_cec_gpio_release(void) {
    gpio_set_dir(CEC_INJECTOR_RP2040_CEC_GPIO, GPIO_IN);
    gpio_disable_pulls(CEC_INJECTOR_RP2040_CEC_GPIO);
}

bool rp2040_cec_gpio_read(void) {
    return gpio_get(CEC_INJECTOR_RP2040_CEC_GPIO);
}

void rp2040_cec_gpio_drive_low(void) {
    /* Preload LOW before output enable. The CEC pin is never driven HIGH. */
    gpio_put(CEC_INJECTOR_RP2040_CEC_GPIO, 0);
    gpio_set_dir(CEC_INJECTOR_RP2040_CEC_GPIO, GPIO_OUT);
}
