/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "print.h"

#include "config.h"
#include "custom_matrix.h"
#include "custom_analog.h"

// Declare the ADC manager instance
__attribute__((section(".ram0")))
static ADCManager adcManager;

// External definitions
extern SPLIT_MUTABLE_ROW pin_t row_pins[ROWS_PER_HAND];

// called whenever an adc conversion is completed
static void adcCompleteCallback(ADCDriver *adcp) {
    (void)adcp; // Unused parameter
    osalSysLockFromISR();
    adcManager.completedConversions++;

    if (
        ( is_keyboard_left() && adcManager.completedConversions >= N_ADCS_SCANNED) || 
        (!is_keyboard_left() && adcManager.completedConversions >= N_ADCS_SCANNED_RIGHT)
    )
    {
        chSemSignalI(&adcManager.sem);
    }
    
    osalSysUnlockFromISR();
}

// initialise adc pins and start the adcs
void initADCGroups(void) {
    adcManager.completedConversions = 0;
    chSemObjectInit(&adcManager.sem, 0);

    // Set input mode of pins to analog
    for (uint8_t i = 0; i < ROWS_PER_HAND; i++) {
        if (row_pins[i] != NO_PIN){ // row pins are set in matrix_init
            palSetLineMode(row_pins[i], PAL_MODE_INPUT_ANALOG);
        }
    }

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
    
    /* Start ADCs
    Left uses 1,2,4
    Right uses 1,3,4 */
    adcStart(&ADCD1, NULL);
    adcStart(&ADCD2, NULL);
    adcStart(&ADCD3, NULL);
    adcStart(&ADCD4, NULL);

    return;
}

// start converting (non-continuous)
msg_t adcStartAllConversions(uint8_t current_col){
    osalSysLock();
    adcManager.completedConversions = 0;

    // Start conversion groups
    if (is_keyboard_left()){
        // Scan multiplexers
        adcStartConversionI(&ADCD1, &adcConversionGroup1, adcManager.sampleBuffer0, 1);
        adcStartConversionI(&ADCD4, &adcConversionGroup4, adcManager.sampleBuffer1, 1);
        // Scan direct pins - only scan one channel, regardless of how many channels are scanned on other ADCs
        switch (current_col){
            case 0: // W
                adcStartConversionI(&ADCD2, &adcConversionGroup3, adcManager.sampleBuffer2, 1);
                break;
            case 1: // A
                adcStartConversionI(&ADCD2, &adcConversionGroup4, adcManager.sampleBuffer2, 1);
                break;
            case 2: // S
                adcStartConversionI(&ADCD2, &adcConversionGroup2, adcManager.sampleBuffer2, 1);
                break;
            case 3: // D
                adcStartConversionI(&ADCD2, &adcConversionGroup1, adcManager.sampleBuffer2, 1);
                break;
            default: // increment completedConversions without doing an ADC conversion
                adcManager.sampleBuffer2[0] = ANALOG_RAW_MAX_VALUE;
                adcManager.completedConversions++;
                break;
        }
    }
    else {
        adcStartConversionI(&ADCD1, &adcConversionGroup4, adcManager.sampleBuffer3, 1);
        adcStartConversionI(&ADCD3, &adcConversionGroup1, adcManager.sampleBuffer4, 1);
        adcStartConversionI(&ADCD4, &adcConversionGroup3, adcManager.sampleBuffer5, 1);
    }
    
    osalSysUnlock();
    return MSG_OK;
}

// wait until the semaphore is triggered
msg_t adcWaitForConversions(void){
    
    chSemWait(&adcManager.sem);

    return MSG_OK;
}

// retrieve an adc sample
adcsample_t getADCSample(uint8_t current_row) {
    switch (current_row) {

        // Left
        case 0:
            return adcManager.sampleBuffer0[0];
        case 1:
            return adcManager.sampleBuffer1[0];
        case 2:
            return adcManager.sampleBuffer2[0];
        case 3:
            return ANALOG_RAW_MAX_VALUE; // This row is used for DKS

        // Right
        case 4:
            return adcManager.sampleBuffer3[0];
        case 5:
            return adcManager.sampleBuffer4[0];
        case 6:
            return adcManager.sampleBuffer5[0];
        case 7:
            return ANALOG_RAW_MAX_VALUE; // This row is used for DKS

        // Invalid index
        default:
            return ANALOG_RAW_MAX_VALUE;
    }
}

// print errors to the console
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

static const ADCConversionGroup adcConversionGroup4 = { // Channel 5 only
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
        ADC_SMPR1_SMP_AN5(ADC_SAMPLING_TIME),
    },
    .sqr          = {
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN5),
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
        ADC_SMPR1_SMP_AN1(ADC_SAMPLING_TIME) | 
        ADC_SMPR1_SMP_AN2(ADC_SAMPLING_TIME) | 
        ADC_SMPR1_SMP_AN3(ADC_SAMPLING_TIME) | 
        ADC_SMPR1_SMP_AN4(ADC_SAMPLING_TIME) | 
        ADC_SMPR1_SMP_AN5(ADC_SAMPLING_TIME),
    },
    .sqr          = {
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN1) | 
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN2) | 
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN3) | 
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN4) | 
        ADC_SQR1_SQ1_N(ADC_CHANNEL_IN5),
    }
}; */