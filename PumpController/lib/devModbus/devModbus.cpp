#include <Arduino.h>

#include "main.h"

#include "aoCalc.h"
#include "aoEEPROM.h"
#include "aoModbusRTU.h"
#include "auto_tune_pid.h"
#include "devModbus.h"
#include "microLAN.h"

#if MODBUSRTU_VERSION	>= 20201031
ModbusRTU mb;
#endif

const TRegister ModbusAO::registers[] PROGMEM = {
//  IREGs
    IREG2(100, (word *)(&sensorsValues[0].value)),
    IREG2(102, (word *)(&sensorsValues[1].value)),

    IREG2(200, (word *)(&sensorsValues[0].value0)),
    IREG2(202, (word *)(&sensorsValues[1].value0)),

    IREG2(300, (word *)(&input)),
    IREG2(302, (word *)(&output)),

    IREG(400, &sensorsCalc[0].state),
    IREG(401, &sensorsCalc[1].state),

    IREG(500, (word *)(&statistics[0].valueAvg)),
    IREG(501, (word *)(&statistics[0].divUp)),
    IREG(502, (word *)(&statistics[0].divDown)),
    IREG(503, (word *)(&statistics[0].moveUpDown)),


    IREG(600, (word *)(&statistics[1].valueAvg)),
    IREG(601, (word *)(&statistics[1].divUp)),
    IREG(602, (word *)(&statistics[1].divDown)),
    IREG(603, (word *)(&statistics[1].moveUpDown)),

    // HREGs

    HREG_f(SENSOR_INDEX_HREG + 0, &sensorsCalc[0].port, &rwIndexHreg),
    HREG_f(SENSOR_INDEX_HREG + 1, &sensorsCalc[1].port, &rwIndexHreg),

    HREG_f(CALC_SMOOTH_HREG + 0, &sensorsCalc[0].smooth, rwSmoothHreg),
    HREG_f(CALC_SMOOTH_HREG + 1, &sensorsCalc[1].smooth, rwSmoothHreg),

    HREG_f(PID_REINIT_HREG, NULL, &rwUpdateHreg),
    HREG_EEPROM(PID_MOD_HREG, (word *)EE_MODE, &rwModeHreg),
    
    HREG2_EEPROM(PID_OUTPUT_HREG, (word *)EE_OUTPUT, &rwOutputHreg),
    HREG2_EEPROM(PID_KP_HREG, (word *)EE_KP, &rwKpHreg),
    HREG2_EEPROM(PID_KI_HREG, (word *)EE_KI, &rwKiHreg),
    HREG2_EEPROM(PID_KD_HREG, (word *)EE_KD, &rwKdHreg),
    HREG2_EEPROM(PID_SETPNT_HREG, (word *)EE_SETPNT, &rwSetPointHreg),

};

void setupModbus()
{

  Serial.println(F("Modbus Serial1 38400 8n1"));
  Serial.println(F("Slave address 11"));
  Serial1.begin(38400, SERIAL_8N1);
  mb.begin(
    sizeof(ModbusAO::registers) / sizeof(TRegister),
    912// 3.5 symbols for 38400 bps
  );
  mb.config(&Serial1);
  mb.setSlaveId(11);
  mb.swapMode(true);
}

void loopModbus()
{
  mb.task();
}
