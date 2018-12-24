#ifndef _microLAN_h
#define _microLAN_h

#include <DallasTemperature.h>

extern DeviceAddress insideThermometer;
extern DeviceAddress pipeThermometer;
extern DeviceAddress outsideThermometer;

float getTemp(DeviceAddress & deviceAddress);

void setupOneWire();
void loopOneWire();


#endif //_microLAN_h
