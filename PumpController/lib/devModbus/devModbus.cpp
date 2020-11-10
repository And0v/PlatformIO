#include <Arduino.h>
#include "main.h"
#include "aoModbusRTU.h"
#include "devModbus.h"
#include "microLAN.h"
#include "auto_tune_pid.h"
#include "aoEEPROM.h"


ModbusRTU mb;

const TRegister ModbusAO::registers[] PROGMEM = {
    IREG2(100, (word*)(&sensorsList[0].value)),
    IREG2(102, (word*)(&sensorsList[1].value)),
    IREG2(104, (word*)(&sensorsList[2].value)),
    IREG2(106, (word*)(&input)),
    
    IREG(110, &sensorsList[0].status),
    IREG(111, &sensorsList[1].status),
    IREG(112, &sensorsList[2].status),
    //0
    HREG_f(PID_REINIT_HREG,    NULL, &rwUpdateHreg),

    HREG_EEPROM(PID_MOD_HREG,       (word *)EE_MODE, &rwModeHreg),
    HREG_EEPROM(PID_AT_HREG,        (word *)EE_AT, &rwTuningHreg),
    HREG_EEPROM(PID_INDEX_HREG,     (word *)EE_INP_IDX, &rwIndexHreg),
    HREG2_EEPROM(PID_OUTPUT_HREG,    (word *)EE_OUTPUT, &rwOutputHreg),
    HREG2_EEPROM(PID_KP_HREG,        (word *)EE_KP, &rwKpHreg),
    HREG2_EEPROM(PID_KI_HREG,        (word *)EE_KI, &rwKiHreg),
    HREG2_EEPROM(PID_KD_HREG,        (word *)EE_KD, &rwKdHreg),
    HREG2_EEPROM(PID_SETPNT_HREG,    (word *)EE_SETPNT, &rwSetPointHreg),
    };


void setupModbus(){

  Serial1.begin(38400);
  mb.config(&Serial1);
  mb.setSlaveId(11);

}
void loopModbus(){
 mb.task();
}
