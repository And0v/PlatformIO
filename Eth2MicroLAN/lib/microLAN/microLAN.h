#ifndef _microLAN_h
#define _microLAN_h

#include <DallasTemperature.h>

#define BOILER_OUT 0
#define BOILER_IN 1
#define HYDROSREAM_1 2
#define HYDROSREAM_2 3
#define HYDROSREAM_3 4
#define FEED_PIPE 5
#define RETURN_PIPE 6
#define OUTSIDE 7

#define SENSORS_COUNT 8

#define EEPROM_ADDRESSES 0

int16_t getTemp(DeviceAddress & deviceAddress);

extern int16_t sensorsList[SENSORS_COUNT];
// extern DeviceAddress sensorsAddresses[SENSORS_COUNT];

void setupOneWire();
void loopOneWire();


#endif //_microLAN_h
