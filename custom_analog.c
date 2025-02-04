/* Copyright 2023 RephlexZero (@RephlexZero)
SPDX-License-Identifier: GPL-2.0-or-later */

#include "custom_analog.h"
#include "print.h"

// Define the global ADC manager instance
ADCManager adcManager;

// External definitions
extern row_pins[ROWS_PER_HAND];

static void adcCompleteCallback(ADCDriver *adcp) {
    (void)adcp; // Unused parameter
    osalSysLockFromISR();
    adcManager.completedConversions++;
    if (adcManager.completedConversions == 4) {
        chSemSignalI(&adcManager.sem); // Signal the semaphore
    }
    osalSysUnlockFromISR();
}

void adcErrorCallback(ADCDriver *adcp, adcerror_t err) {
    (void)adcp; // Unused parameter
    osalSysLockFromISR();
    switch (err) {
        case ADC_ERR_DMAFAILURE:
            uprintf("ADC ERROR: DMA failure.\n");
            break;
        case ADC_ERR_OVERFLOW:
            uprintf("ADC ERROR: Overflow.\n");
            break;
        case ADC_ERR_AWD1:
            uprintf("ADC ERROR: Watchdog 1 triggered.\n");
            break;
        case ADC_ERR_AWD2:
            uprintf("ADC ERROR: Watchdog 2 triggered.\n");
            break;
        case ADC_ERR_AWD3:
            uprintf("ADC ERROR: Watchdog 3 triggered.\n");
            break;
        default:
            uprintf("ADC ERROR: Unknown error.\n");
            break;
    }
    osalSysUnlockFromISR();
}

static const ADCConversionGroup adcConversionGroup1 = { // Channel 1 only
    .circular     = false,
    .num_channels = 1U,
    .end_cb       = adcCompleteCallback,
    .error_cb     = adcErrorCallback,
    .cfgr         = ADC_RESOLUTION,
    .tr1          = ADC_TR_DISABLED,
    .tr2          = ADC_TR_DISABLED,
    .tr3          = ADC_TR_DISABLED,
    .awd2cr       = 0U,
    .awd3cr       = 0U,
    .smpr         = {
        ADC_SMPR1_SMP_AN1(ADC_SAMPLING_TIME),
    },
    .sqr          = {
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1),
    }
};

static const ADCConversionGroup adcConversionGroup2 = { // Channel 2 only
    .circular     = false,
    .num_channels = 1U,
    .end_cb       = adcCompleteCallback,
    .error_cb     = adcErrorCallback,
    .cfgr         = ADC_RESOLUTION,
    .tr1          = ADC_TR_DISABLED,
    .tr2          = ADC_TR_DISABLED,
    .tr3          = ADC_TR_DISABLED,
    .awd2cr       = 0U,
    .awd3cr       = 0U,
    .smpr         = {
        ADC_SMPR1_SMP_AN2(ADC_SAMPLING_TIME),
    },
    .sqr          = {
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN2),
    }
};

static const ADCConversionGroup adcConversionGroup3 = { // Channel 3 only
    .circular     = false,
    .num_channels = 1U,
    .end_cb       = adcCompleteCallback,
    .error_cb     = adcErrorCallback,
    .cfgr         = ADC_RESOLUTION,
    .tr1          = ADC_TR_DISABLED,
    .tr2          = ADC_TR_DISABLED,
    .tr3          = ADC_TR_DISABLED,
    .awd2cr       = 0U,
    .awd3cr       = 0U,
    .smpr         = {
        ADC_SMPR1_SMP_AN3(ADC_SAMPLING_TIME),
    },
    .sqr          = {
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN3),
    }
};

static const ADCConversionGroup adcConversionGroup4 = { // Channel 4 only
    .circular     = false,
    .num_channels = 1U,
    .end_cb       = adcCompleteCallback,
    .error_cb     = adcErrorCallback,
    .cfgr         = ADC_RESOLUTION,
    .tr1          = ADC_TR_DISABLED,
    .tr2          = ADC_TR_DISABLED,
    .tr3          = ADC_TR_DISABLED,
    .awd2cr       = 0U,
    .awd3cr       = 0U,
    .smpr         = {
        ADC_SMPR1_SMP_AN4(ADC_SAMPLING_TIME),
    },
    .sqr          = {
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN4),
    }
};

/* Example of scanning multiple ADC channels
static const ADCConversionGroup adcConversionGroup5 = { // Channels 1,2,3,4,5
    .circular     = false,
    .num_channels = 5U,
    .end_cb       = adcCompleteCallback,
    .error_cb     = adcErrorCallback,
    .cfgr         = ADC_RESOLUTION,
    .tr1          = ADC_TR_DISABLED,
    .tr2          = ADC_TR_DISABLED,
    .tr3          = ADC_TR_DISABLED,
    .awd2cr       = 0U,
    .awd3cr       = 0U,
    .smpr         = {
        ADC_SMPR1_SMP_AN1(ADC_SAMPLING_TIME) | ADC_SMPR1_SMP_AN2(ADC_SAMPLING_TIME) | ADC_SMPR1_SMP_AN3(ADC_SAMPLING_TIME) | ADC_SMPR1_SMP_AN4(ADC_SAMPLING_TIME) | ADC_SMPR1_SMP_AN5(ADC_SAMPLING_TIME),
    },
    .sqr          = {
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1) | ADC_SQR1_SQ1_N(ADC_CHANNEL_IN2) | ADC_SQR1_SQ1_N(ADC_CHANNEL_IN3) | ADC_SQR1_SQ1_N(ADC_CHANNEL_IN4) | ADC_SQR1_SQ1_N(ADC_CHANNEL_IN5),
    }
}; */

void initADCGroups(ADCManager *adcManager) {
    adcManager->completedConversions = 0;
    chSemObjectInit(&adcManager->sem, 0); // Initialize semaphore with a count of 0

#ifdef DIRECT_PINS
    const pin_t direct_pins[MATRIX_DIRECT] = DIRECT_PINS;
    if (is_keyboard_left()){
        for (uint8_t i = 0; i < MATRIX_DIRECT; i++){
            palSetLineMode(direct_pins[i], PAL_MODE_INPUT_ANALOG);
        }
    }
#endif
#ifdef DIRECT_PINS_RIGHT
    const pin_t direct_pins_right[MATRIX_DIRECT_RIGHT] = DIRECT_PINS_RIGHT;
    if (!is_keyboard_left()){
        for (uint8_t i = 0; i < MATRIX_DIRECT_RIGHT; i++){
            palSetLineMode(direct_pins_right[i], PAL_MODE_INPUT_ANALOG);
        }
    }
#endif

    // Set input mode of pins to analog
    for (uint8_t i = 0; i < ROWS_PER_HAND; i++) {
        if (row_pins[i] != NO_PIN){ // row pins are set in matrix_init
            palSetLineMode(row_pins[i], PAL_MODE_INPUT_ANALOG);
        }
    }
    
    // Start ADCs
    if (is_keyboard_left()){ // Left side uses 1,2,4
        adcStart(&ADCD1, NULL);
        adcStart(&ADCD2, NULL);
        adcStart(&ADCD4, NULL);
    }
    else { // Right side uses 1,3,4
        adcStart(&ADCD1, NULL);
        adcStart(&ADCD3, NULL);
        adcStart(&ADCD4, NULL);
    }
}

msg_t adcStartAllConversions(ADCManager *adcManager, uint8_t current_direct_pin) {
    osalSysLock();
    adcManager->completedConversions = 0;

    // Start conversion groups
    if (is_keyboard_left()){
        // Scan multiplexers
        adcStartConversionI(&ADCD1, &adcConversionGroup1, adcManager->sampleBuffer0, 1);
        adcStartConversionI(&ADCD4, &adcConversionGroup4, adcManager->sampleBuffer1, 1);
        // Scan direct pin channels
        switch (current_direct_pin){
            case 0: // W
                adcStartConversionI(&ADCD2, &adcConversionGroup3, adcManager->sampleBuffer2, 1);
                break;
            case 1: // A
                adcStartConversionI(&ADCD2, &adcConversionGroup4, adcManager->sampleBuffer6, 1);
                break;
            case 2: // S
                adcStartConversionI(&ADCD2, &adcConversionGroup2, adcManager->sampleBuffer7, 1);
                break;
            case 3: // D
                adcStartConversionI(&ADCD2, &adcConversionGroup1, adcManager->sampleBuffer8, 1);
                break;
            default:
                break;
        }
    }
    else {
        adcStartConversionI(&ADCD1, &adcConversionGroup4, adcManager->sampleBuffer3, 1);
        adcStartConversionI(&ADCD3, &adcConversionGroup1, adcManager->sampleBuffer4, 1);
        adcStartConversionI(&ADCD4, &adcConversionGroup3, adcManager->sampleBuffer5, 1);
    }
    
    osalSysUnlock();

    chSemWaitTimeout(&adcManager->sem, TIME_INFINITE);

    return MSG_OK;
}

adcsample_t getADCSample(const ADCManager *adcManager, uint8_t current_row, uint8_t current_direct_pin) {
    switch (current_row) {

        // Left
        case 0:
            return adcManager->sampleBuffer0[0];
        case 1:
            return adcManager->sampleBuffer1[0];
        case 2:
            switch (current_direct_pin){
                case 0: // W
                    return adcManager->sampleBuffer2[0];
                case 1: // A
                    return adcManager->sampleBuffer6[0];
                case 2: // S
                    return adcManager->sampleBuffer7[0];
                case 3: // D
                    return adcManager->sampleBuffer8[0];
            }
        case 3:
            return 2047; // This row is used for DKS

        // Right
        case 4:
            return adcManager->sampleBuffer3[0];
        case 5:
            return adcManager->sampleBuffer4[0];
        case 6:
            return adcManager->sampleBuffer5[0];
        case 7:
            return 2047; // This row is used for DKS

        // Invalid index
        default:
            return 2047;
    }
}