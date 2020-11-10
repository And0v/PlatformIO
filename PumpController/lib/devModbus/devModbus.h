#ifndef _dev_modbus_h
#define _dev_modbus_h

#include "aoModbusRTU.h"

#define SENSORS_VALUES_IREG 100
//REGISTERS from 100 to 105 (2 * SENSORS_COUNT)
#define SENSORS_STATUS_IREG  110
//REGISTERS from 110 to 113
#define PID_REINIT_HREG   200
#define PID_MOD_HREG      201
#define PID_AT_HREG       202
#define PID_INDEX_HREG    203

#define PID_OUTPUT_HREG   302
#define PID_KP_HREG       304
#define PID_KI_HREG       306
#define PID_KD_HREG       308
#define PID_SETPNT_HREG   310

extern ModbusRTU mb;

void setupModbus();
void loopModbus();



#endif //_dev_modbus_h
