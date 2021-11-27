#ifndef _microLAN_h
#define _microLAN_h

#include <DallasTemperature.h>

#define SENSOR_PIPE_RETURN 0
#define SENSOR_PIPE_SUPPLY 1
#define SENSOR_BOILER 2
#define SENSOR_OUTSIDE 3


#define SENSORS_COUNT 2

typedef  struct{
    float value0;
    float value;
} SensorDef;

extern SensorDef sensorsValues[SENSORS_COUNT];
extern CalcDef sensorsCalc[SENSORS_COUNT];
extern StaticticsDef statistics[SENSORS_COUNT];
// float getTemp(DeviceAddress & deviceAddress);

void setupOneWire();
void loopOneWire();


#endif //_microLAN_h
