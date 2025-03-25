#pragma once

#include "config.h"
#include "hal.h"

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