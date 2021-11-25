#ifndef _microLAN_h
#define _microLAN_h

#include <DallasTemperature.h>

#define SENSOR_PIPE_SUPPLY 0
#define SENSOR_PIPE_RETURN 1
#define SENSOR_BOILER 2
#define SENSOR_OUTSIDE 3

#define SENSOR_STATUS_OK 1
#define SENSOR_STATUS_ERROR 0
#include "aoCalc.h"

#define SENSORS_COUNT 4
typedef  struct{
    float value0;
    float value;
} SensorDef;

extern SensorDef sensorsValues[SENSORS_COUNT];
extern CalcDef sensorsCalc[SENSORS_COUNT];

// float getTemp(DeviceAddress & deviceAddress);

void setupOneWire();
void loopOneWire();


#endif //_microLAN_h
