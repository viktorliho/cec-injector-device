// SPDX-License-Identifier: Apache-2.0

#include "rp2040_adapter_runtime.h"

#include "rp2040_cec_analyzer.h"
#include "rp2040_cec_follower_ack.h"
#include "rp2040_cec_gpio.h"
#include "rp2040_cec_tx.h"
#include "rp2040_usb_lifecycle.h"
#include "rp2040_usb_tinyusb.h"

enum { MAX_ANALYZER_OBSERVATIONS_PER_TASK = 64 };

static void on_received_frame(const cec_rx_frame_t *frame, void *context) {
    rp2040_adapter_runtime_t *const runtime = context;
    wake_extension_offer_frame(
        pulse8_profile_wake_extension(&runtime->profile), frame);

    pulse8_profile_message_t messages[CEC_INJECTOR_CEC_MAX_FRAME_OCTETS];
    const size_t count = pulse8_profile_encode_rx(
        frame, messages, CEC_INJECTOR_CEC_MAX_FRAME_OCTETS);
    if (count != 0) {
        (void)rp2040_usb_transport_enqueue_batch(
            &runtime->usb_transport, messages, count);
    }
}

void rp2040_adapter_runtime_initialize(rp2040_adapter_runtime_t *runtime) {
    /* Electrical safety and all core/sink state precede USB activation. */
    rp2040_cec_gpio_initialize();
    rp2040_cec_follower_ack_initialize();
    pulse8_profile_initialize(&runtime->profile);

    const cec_rx_sink_t sink = {
        .on_frame = on_received_frame,
        .on_follower_ack_decision =
            rp2040_cec_follower_ack_decision_sink,
        .on_follower_ack_cancel = rp2040_cec_follower_ack_cancel_sink,
        .context = runtime,
    };
    cec_rx_decoder_initialize(&runtime->rx_decoder, &sink);
    rp2040_cec_analyzer_initialize();
    runtime->analyzer_dropped = rp2040_cec_analyzer_dropped_count();

    rp2040_usb_transport_initialize(
        &runtime->usb_transport,
        &runtime->profile,
        rp2040_usb_tinyusb_carrier());
    rp2040_usb_remote_wake_initialize(&runtime->remote_wake,
                                      &runtime->profile);
    rp2040_usb_lifecycle_bind(&runtime->usb_transport,
                              &runtime->remote_wake);
    rp2040_usb_tinyusb_start();
}

static void process_receive(rp2040_adapter_runtime_t *runtime,
                            uint64_t now_us) {
    cec_rx_observation_t observation;
    size_t count = 0;
    while (count < MAX_ANALYZER_OBSERVATIONS_PER_TASK &&
           rp2040_cec_analyzer_pop(&observation)) {
        cec_rx_decoder_consume(&runtime->rx_decoder, &observation);
        count++;
    }

    const uint32_t dropped = rp2040_cec_analyzer_dropped_count();
    if (dropped != runtime->analyzer_dropped) {
        cec_rx_decoder_notify_observation_overflow(&runtime->rx_decoder);
        runtime->analyzer_dropped = dropped;
    }
    /* Do not apply an idle timeout while older analyzer input remains queued. */
    if (count < MAX_ANALYZER_OBSERVATIONS_PER_TASK) {
        cec_rx_decoder_poll(&runtime->rx_decoder, now_us);
    }
}

static void process_transmit(rp2040_adapter_runtime_t *runtime) {
    if (!rp2040_usb_transport_attached(&runtime->usb_transport) ||
        rp2040_usb_transport_suspended(&runtime->usb_transport) ||
        !rp2040_usb_transport_output_idle(&runtime->usb_transport)) {
        return;
    }

    cec_transport_tx_frame_t frame;
    if (pulse8_profile_take_committed_tx(&runtime->profile, &frame)) {
        const cec_tx_result_t result =
            rp2040_cec_transmit(&frame, &runtime->rx_decoder);
        pulse8_profile_complete_tx(&runtime->profile, result);
    }
}

void rp2040_adapter_runtime_task(rp2040_adapter_runtime_t *runtime,
                                 uint64_t now_us) {
    rp2040_usb_tinyusb_task();
    rp2040_usb_transport_task(&runtime->usb_transport, now_us);
    process_receive(runtime, now_us);
    process_transmit(runtime);
}
