#include <Arduino.h>
#include "aoCalc.h"

bool addCalcValue(CalcDef *adcDef, int16_t value, int16_t min, int16_t max)
{
    byte status = CALC_STATE_OK;
    if (value <= min)
    {
        status = CALC_STATE_LOW;
    }
    else if (value >= max)
    {
        status = CALC_STATE_HIGH;
    }

    adcDef->adc0 = value;

    if (adcDef->state != status)
    {
        if (status == CALC_STATE_OK)
        {
            adcDef->state = CALC_STATE_OK;
            adcDef->adcSum = ((int32_t)value) << adcDef->smooth;
            adcDef->adc = value;
        }
        else
        {
            adcDef->state = status;
        }
    }
    else
    {
        if (status == CALC_STATE_OK)
        {
            adcDef->adcSum -= adcDef->adc;
            adcDef->adcSum += value;
            adcDef->adc = adcDef->adcSum >> adcDef->smooth;
        }
    }
    return (status == CALC_STATE_OK);
}