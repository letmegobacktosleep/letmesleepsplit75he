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
    adcsample_t sampleBuffer6[MAX_MUXES_PER_ADC];
    adcsample_t sampleBuffer7[MAX_MUXES_PER_ADC];
    adcsample_t sampleBuffer8[MAX_MUXES_PER_ADC];
    volatile int completedConversions;
    semaphore_t sem;
} ADCManager;

// Extern ADCManager instance
extern ADCManager adcManager;

// Function Prototypes
void initADCGroups(ADCManager *adcManager);
msg_t adcStartAllConversions(ADCManager *adcManager, uint8_t current_direct_pin);
void adcErrorCallback(ADCDriver *adcp, adcerror_t err);
adcsample_t getADCSample(const ADCManager *adcManager, uint8_t current_row, uint8_t current_direct_pin);