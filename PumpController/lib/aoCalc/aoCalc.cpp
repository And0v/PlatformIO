#include <Arduino.h>

#include "aoCalc.h"

bool addCalcValue(CalcDef *adcDef, int16_t value, int16_t min, int16_t max) {
  byte status = CALC_STATE_OK;
  if (value <= min) {
    status = CALC_STATE_LOW;
    Serial.print(" low ");
  } else if (value >= max) {
    status = CALC_STATE_HIGH;
    Serial.print(" high ");
  }

  adcDef->adc0 = value;

  if (adcDef->state != status) {
    if (status == CALC_STATE_OK) {
      adcDef->state = CALC_STATE_OK;
      adcDef->adcSum = ((int32_t)value) << adcDef->smooth;
      adcDef->adc = value;
    } else {
      adcDef->state = status;
    }
  } else {
    if (status == CALC_STATE_OK) {
      adcDef->adcSum -= adcDef->adc;
      adcDef->adcSum += value;
      adcDef->adc = adcDef->adcSum >> adcDef->smooth;
    }
  }
  return (status == CALC_STATE_OK);
}

void calcStatistics(CalcDef &calc, StaticticsDef &stat, bool init) {
  if (init) {
    stat.value = calc.adc;
    stat.prevValue = stat.value;
    stat.valueSum = ((int32_t)stat.value) << stat.smooth;
    stat.valueAvg = stat.value;
  } else {
    int16_t delta = calc.adc - stat.value;
    if (delta > 0) {
      if (stat.value > stat.prevValue) {
        stat.moveUpDown++;
      } else {
        stat.moveUpDown = 0;
      }
    } else if (delta < 0) {
      if (stat.value < stat.prevValue) {
        stat.moveUpDown--;
      } else {
        stat.moveUpDown = 0;
      }
    }
    stat.valueSum -= stat.valueAvg;
    stat.valueSum += calc.adc;
    stat.valueAvg = stat.valueSum >> stat.smooth;

    int16_t diviation = calc.adc - stat.valueAvg;

    if (diviation > 0) {
      stat.divUp = (stat.divUp + diviation) >> 1;
    } else if (diviation < 0) {
      stat.divDown = (stat.divDown + diviation) >> 1;
    }

    stat.prevValue = stat.value;
    stat.value = calc.adc;
  }
}