#ifndef _microLAN_h
#define _microLAN_h

#include <DallasTemperature.h>

#define SENSOR_INSIDE 0
#define SENSOR_PIPE 1
#define SENSOR_OUTSIDE 2

#define SENSOR_STATUS_OK 1
#define SENSOR_STATUS_ERROR 0


#define SENSORS_COUNT 3
typedef  struct{
    DeviceAddress address;
    float value;
    byte status;
} SensorDef;

extern SensorDef sensorsList[SENSORS_COUNT];

float getTemp(DeviceAddress & deviceAddress);

void setupOneWire();
void loopOneWire();


#endif //_microLAN_h
