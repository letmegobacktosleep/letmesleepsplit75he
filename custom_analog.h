/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include "hal.h"

// Type Definitions
typedef struct {
    adcsample_t sampleBuffer0[MAX_MUXES_PER_ADC];
    adcsample_t sampleBuffer1[MAX_MUXES_PER_ADC];
    adcsample_t sampleBuffer2[MAX_MUXES_PER_ADC];
    adcsample_t sampleBuffer3[MAX_MUXES_PER_ADC];
    adcsample_t sampleBuffer4[MAX_MUXES_PER_ADC];
    adcsample_t sampleBuffer5[MAX_MUXES_PER_ADC];
    volatile int completedConversions;
    semaphore_t sem;
} ADCManager;

// Extern ADCManager instance
extern ADCManager adcManager;

// Function Prototypes
void initADCGroups(void);
void adcErrorCallback(ADCDriver *adcp, adcerror_t err);
msg_t adcStartAllConversions(uint8_t current_col);
msg_t adcWaitForConversions(void);
adcsample_t getADCSample(uint8_t current_row);