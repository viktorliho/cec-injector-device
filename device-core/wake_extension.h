// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "cec_rx.h"

enum {
    WAKE_EXTENSION_VERSION = 0x01,
    WAKE_EXTENSION_MODE_NONE = 0x00,
    WAKE_EXTENSION_MODE_SET_STREAM_PATH = 0x01,
    WAKE_EXTENSION_SUPPORTED_MODES = 0x01,
    WAKE_EXTENSION_MINIMUM_DELIVERY_DELAY_US = 10000,
};

typedef enum {
    WAKE_EXTENSION_RESULT_NONE = 0,
    WAKE_EXTENSION_RESULT_SUCCESS = 1,
    WAKE_EXTENSION_RESULT_REMOTE_WAKE_FAILED = 2,
} wake_extension_result_t;

typedef struct {
    bool valid;
    bool enabled;
    uint16_t physical_address;
    uint8_t mode;
} wake_extension_config_t;

typedef struct {
    wake_extension_config_t config;
    bool armed;
    bool transport_configured;
    bool transport_suspended;
    bool remote_wake_authorized;
    bool wake_consumed;
    uint32_t suspend_cycles;
    uint32_t matching_frames;
    uint32_t rejected_frames;
    uint32_t remote_wakeup_requests;
    uint32_t duplicate_suppressed;
    wake_extension_result_t last_result;
    uint32_t last_wake_offset_us;
    uint8_t last_frame_length;
    uint8_t last_frame[CEC_INJECTOR_CEC_MAX_FRAME_OCTETS];
} wake_extension_status_t;

typedef bool (*wake_extension_remote_wake_fn)(void *context);

typedef struct {
    wake_extension_status_t _status;
    wake_extension_remote_wake_fn _request_remote_wake;
    void *_delivery_context;
    uint64_t _suspend_start_us;
    uint64_t _earliest_delivery_us;
    bool _match_pending;
} wake_extension_t;

void wake_extension_initialize(wake_extension_t *wake);
void wake_extension_session_reset(wake_extension_t *wake);
void wake_extension_set_delivery(wake_extension_t *wake,
                                 wake_extension_remote_wake_fn request,
                                 void *context);

bool wake_extension_physical_address_valid(uint16_t physical_address);
bool wake_extension_set_config(wake_extension_t *wake,
                               bool enabled,
                               uint16_t physical_address,
                               uint8_t mode);
bool wake_extension_arm(wake_extension_t *wake);
void wake_extension_disarm(wake_extension_t *wake);
void wake_extension_clear_last_reason(wake_extension_t *wake);

void wake_extension_transport_configured(wake_extension_t *wake);
void wake_extension_transport_unconfigured(wake_extension_t *wake);
void wake_extension_suspend_begin(wake_extension_t *wake,
                                  bool remote_wake_authorized,
                                  uint64_t timestamp_us);
void wake_extension_set_remote_wake_authorized(wake_extension_t *wake,
                                               bool authorized);
void wake_extension_resume(wake_extension_t *wake);

void wake_extension_offer_frame(wake_extension_t *wake,
                                const cec_rx_frame_t *frame);
void wake_extension_poll(wake_extension_t *wake, uint64_t now_us);

uint8_t wake_extension_derived_state(const wake_extension_t *wake);
wake_extension_status_t wake_extension_status(const wake_extension_t *wake);

#ifdef CEC_INJECTOR_TESTING
void wake_extension_test_set_counters(wake_extension_t *wake,
                                      uint32_t suspend_cycles,
                                      uint32_t matching_frames,
                                      uint32_t rejected_frames,
                                      uint32_t remote_wakeup_requests,
                                      uint32_t duplicate_suppressed);
#endif
