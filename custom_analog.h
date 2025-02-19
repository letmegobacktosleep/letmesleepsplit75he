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
void initADCGroups(ADCManager *adcManager);
void adcErrorCallback(ADCDriver *adcp, adcerror_t err);
msg_t adcStartAllConversions(ADCManager *adcManager, uint8_t current_col);
msg_t adcWaitForConversions(ADCManager *adcManager);
adcsample_t getADCSample(const ADCManager *adcManager, uint8_t current_row);