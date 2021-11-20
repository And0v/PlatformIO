#include <Arduino.h>
#include "main.h"
#include "devModbus.h"
#include "microLAN.h"

ModbusSerial mb;

void setupModbus(){
  mb.config(&Serial1, 38400, SERIAL_8N1);
  mb.setSlaveId(11);
  word startAddr = SENSORS_VALUES_IREG;
  for (byte i = 0; i < SENSORS_COUNT;++i){
    mb.addIreg(startAddr++);
    mb.addIreg(startAddr++);
  }
  startAddr = SENSORS_STATUS_IREG;
  for (byte i = 0; i < SENSORS_COUNT;++i){
    mb.addIreg(startAddr++);
  }
  mb.addHreg(PID_UPDATE_HREG);
  mb.addHreg(PID_MOD_HREG);
  mb.addHreg(PID_AT_HREG);
  mb.addHreg(PID_INDEX_HREG);

  mb.addHreg(PID_INPUT_HREG);
  mb.addHreg(PID_INPUT_HREG+1);
  mb.addHreg(PID_OUTPUT_HREG);
  mb.addHreg(PID_OUTPUT_HREG+1);
  mb.addHreg(PID_KP_HREG);
  mb.addHreg(PID_KP_HREG+1);
  mb.addHreg(PID_KI_HREG);
  mb.addHreg(PID_KI_HREG+1);
  mb.addHreg(PID_KD_HREG);
  mb.addHreg(PID_KD_HREG+1);
  mb.addHreg(PID_SETPOINT_HREG);
  mb.addHreg(PID_SETPOINT_HREG+1);
  mb.addHreg(PID_PWR_SUM_HREG);
  mb.addHreg(PID_PWR_SUM_HREG+1);
}
void loopModbus(){
  if (Events & EV_CONVERSION_COMPLETE){
    Events &= ~EV_CONVERSION_COMPLETE;
    word sensAddr = SENSORS_VALUES_IREG;
    word sensStAddr = SENSORS_STATUS_IREG;
    for(byte i = 0; i < SENSORS_COUNT;++i){
      mb.Ireg(sensStAddr++, sensorsList[i].status);
      if (sensorsList[i].status == SENSOR_STATUS_OK){
        word * reg = (word*)&sensorsList[i].value;
        mb.Ireg(sensAddr++, reg[0]);
        mb.Ireg(sensAddr++, reg[1]);
      }else{
        sensAddr += 2;
      }
    }

  }
  mb.task();
}
void mb_Hreg(word offset, word value){
  mb.Hreg(offset, value);
}
void mb_Hreg(word offset, float value){
  word * reg = (word*)&value;
  mb.Hreg(offset++, reg[0]);
  mb.Hreg(offset++, reg[1]);
}
float mb_Hreg(word offset){
  float rst;
  word * reg = (word*)&rst;
  reg[0] = mb.Hreg(offset++);
  reg[1] = mb.Hreg(offset++);
  return rst;
}
