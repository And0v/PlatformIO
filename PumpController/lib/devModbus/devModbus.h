#ifndef _dev_modbus_h
#define _dev_modbus_h

#include "Modbus.h"
#include "ModbusSerial.h"


#define SENSORS_VALUES_IREG 100
//REGISTERS from 100 to 105 (2 * SENSORS_COUNT)
#define SENSORS_STATUS_IREG  110
//REGISTERS from 110 to 113
#define PID_UPDATE_HREG   200
#define PID_UPADTE_MOD    0x0001
#define PID_UPADTE_AT     0x0002
#define PID_UPADTE_IDX    0x0004

#define PID_UPADTE_OUT    0x0100
#define PID_UPADTE_KP     0x0200
#define PID_UPADTE_KI     0x0400
#define PID_UPADTE_KD     0x0800
#define PID_UPADTE_SET    0x1000

#define PID_MOD_HREG      201
#define PID_AT_HREG       202
#define PID_INDEX_HREG    203

#define PID_INPUT_HREG    300
#define PID_OUTPUT_HREG   302
#define PID_KP_HREG       304
#define PID_KI_HREG       306
#define PID_KD_HREG       308
#define PID_SETPOINT_HREG 310

extern ModbusSerial mb;


void setupModbus();
void loopModbus();
void mb_Hreg(word offset, word value);
void mb_Hreg(word offset, float pValue);
float mb_Hreg(word offset);


#endif //_dev_modbus_h
