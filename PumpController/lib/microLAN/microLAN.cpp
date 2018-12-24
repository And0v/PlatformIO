
#include <Arduino.h>
#include "main.h"
#include <OneWire.h>
#include "microLAN.h"

#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

SensorDef sensorsList[SENSORS_COUNT];

void setupOneWire(){
// Start up the library
  sensors.begin();

  // search for devices on the bus and assign based on an index
  for (byte i = 0; i < SENSORS_COUNT; ++i){
    if (!sensors.getAddress(sensorsList[i].address, i)) {
      Serial.print("Unable to find address for Device ");
      Serial.println(i);
    }
  }
  sensors.setCheckForConversion(false);
  Events |= EV_REQUEST_CONVERSION;
  //sensors.requestTemperatures();
}

SensorDef & getTemp(byte sensorIdx)
{
  SensorDef & sensor = sensorsList[sensorIdx];

  float tempC = sensors.getTempC(sensor.address);
  if (tempC == DEVICE_DISCONNECTED_C){
    tempC = NAN;
  }
  switch (sensor.status) {
    case SENSOR_STATUS_ERROR:
      if (tempC != NAN){
        sensor.value = tempC;
        sensor.status = SENSOR_STATUS_OK;
      }
    break;
    case SENSOR_STATUS_OK:
      if (tempC != NAN){
        sensor.value = (tempC+sensor.value)/2;
        sensor.status = SENSOR_STATUS_OK;
      }else{
        sensor.status = SENSOR_STATUS_ERROR;
      }
    break;
  }

  return sensor;
}

void loopOneWire()
{
  if (Events & EV_REQUEST_CONVERSION){
    Events &= ~EV_REQUEST_CONVERSION;
    sensors.requestTemperatures();
    Events |= EV_START_CONVERSION;
  }
  if ((Events & EV_START_CONVERSION)&&(sensors.isConversionComplete())){
    Events &= ~EV_START_CONVERSION;
    Serial.print(" Inside temp: ");
    Serial.println(getTemp(SENSOR_INSIDE).value);
    Serial.print("  Pipe temp: ");
    Serial.println(getTemp(SENSOR_PIPE).value);
    Serial.print("Outside temp: ");
    Serial.println(getTemp(SENSOR_OUTSIDE).value);
    Events |= EV_CONVERSION_COMPLETE;
  }
}
