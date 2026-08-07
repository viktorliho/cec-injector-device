// SPDX-License-Identifier: Apache-2.0

#include "rp2040_adapter_runtime.h"

#include "pico/stdlib.h"
#include "pico/time.h"

int main(void) {
    static rp2040_adapter_runtime_t runtime;
    rp2040_adapter_runtime_initialize(&runtime);

    for (;;) {
        rp2040_adapter_runtime_task(&runtime, time_us_64());
        tight_loop_contents();
    }
}
