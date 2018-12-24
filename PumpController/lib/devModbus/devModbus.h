#ifndef _dev_modbus_h
#define _dev_modbus_h

#include "Modbus.h"
#include "ModbusSerial.h"

extern ModbusSerial mb;


void setupModbus();
void loopModbus();


#endif //_dev_modbus_h
