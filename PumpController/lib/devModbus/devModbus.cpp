#include <Arduino.h>

#include "main.h"

#include "aoCalc.h"
#include "aoEEPROM.h"
#include "aoModbusRTU.h"
#include "auto_tune_pid.h"
#include "devModbus.h"
#include "microLAN.h"

ModbusRTU mb;

const TRegister ModbusAO::registers[] PROGMEM = {
    IREG2(100, (word *)(&sensorsValues[0].value)),
    IREG2(102, (word *)(&sensorsValues[1].value)),

    IREG2(200, (word *)(&sensorsValues[0].value0)),
    IREG2(202, (word *)(&sensorsValues[1].value0)),

    IREG2(300, (word *)(&input)),
    IREG2(302, (word *)(&output)),

    IREG(400, &sensorsCalc[0].state),
    IREG(401, &sensorsCalc[1].state),

    IREG2(500, (word *)(&statistics[0].valueAvg)),
    IREG2(502, (word *)(&statistics[0].divUp)),
    IREG2(504, (word *)(&statistics[0].divDown)),
    IREG2(506, (word *)(&statistics[0].moveUp)),
    IREG2(508, (word *)(&statistics[0].moveDown)),

    IREG2(600, (word *)(&statistics[1].valueAvg)),
    IREG2(602, (word *)(&statistics[1].divUp)),
    IREG2(604, (word *)(&statistics[1].divDown)),
    IREG2(606, (word *)(&statistics[1].moveUp)),
    IREG2(608, (word *)(&statistics[1].moveDown)),
    // 0
    //  HREG(100, &sensorsCalc[0].port),
    //  HREG(101, &sensorsCalc[1].port),
    //  HREG(102, &sensorsCalc[2].port),
    //  HREG(103, &sensorsCalc[2].port),

    HREG_f(SENSOR_INDEX_HREG + 0, &sensorsCalc[0].port, &rwIndexHreg),
    HREG_f(SENSOR_INDEX_HREG + 1, &sensorsCalc[1].port, &rwIndexHreg),

    HREG_f(CALC_SMOOTH_HREG + 0, &sensorsCalc[0].smooth, rwSmoothHreg),
    HREG_f(CALC_SMOOTH_HREG + 1, &sensorsCalc[1].smooth, rwSmoothHreg),

    HREG_f(PID_REINIT_HREG, NULL, &rwUpdateHreg),

    HREG_EEPROM(PID_MOD_HREG, (word *)EE_MODE, &rwModeHreg),
    // HREG_EEPROM(PID_AT_HREG,        (word *)EE_AT, &rwTuningHreg),
    HREG2_EEPROM(PID_OUTPUT_HREG, (word *)EE_OUTPUT, &rwOutputHreg),
    HREG2_EEPROM(PID_KP_HREG, (word *)EE_KP, &rwKpHreg),
    HREG2_EEPROM(PID_KI_HREG, (word *)EE_KI, &rwKiHreg),
    HREG2_EEPROM(PID_KD_HREG, (word *)EE_KD, &rwKdHreg),
    HREG2_EEPROM(PID_SETPNT_HREG, (word *)EE_SETPNT, &rwSetPointHreg),

};

void setupModbus() {

  Serial1.begin(38400);
  mb.config(&Serial1);
  mb.setSlaveId(11);
}
void loopModbus() { mb.task(); }
