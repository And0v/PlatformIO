#include <Arduino.h>
#include "main.h"
#include "devModbus.h"
#include "microLAN.h"

#define SENSORS_VALUES_IREG 100

ModbusSerial mb;

void setupModbus(){
  mb.config(&Serial1, 38400, SERIAL_8N1);
  mb.setSlaveId(11);
  word startAddr = SENSORS_VALUES_IREG;
  for (byte i = 0; i < SENSORS_COUNT;++i){
    mb.addIreg(startAddr++);
    mb.addIreg(startAddr++);
  }

}
void loopModbus(){
  if (Events & EV_CONVERSION_COMPLETE){
    Events &= ~EV_CONVERSION_COMPLETE;
    word sensAddr = SENSORS_VALUES_IREG;
    for(byte i = 0; i < SENSORS_COUNT;++i){
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
