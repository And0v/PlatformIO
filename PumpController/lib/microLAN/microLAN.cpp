
#include <Arduino.h>
#include <OneWire.h>
#include "microLAN.h"

#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

#define SENSOR_INSIDE 0
#define SENSOR_PIPE 1
#define SENSOR_OUTSIDE 2

DeviceAddress pipeThermometer;
DeviceAddress outsideThermometer;

#define SENSOR_STATUS_OK 1
#define SENSOR_STATUS_ERROR 0

typedef  struct{
    DeviceAddress address;
    float value;
    byte status;
} SensorDef;

SensorDef sensorsList[3];

void setupOneWire(){
// Start up the library
  sensors.begin();

  // search for devices on the bus and assign based on an index
  if (!sensors.getAddress(sensorsList[SENSOR_INSIDE].address, 0)) {
    Serial.println("Unable to find address for Device inside");
  }
  if (!sensors.getAddress(sensorsList[SENSOR_PIPE].address, 1)) {
    Serial.println("Unable to find address for Device pipe");
  }
  if (!sensors.getAddress(sensorsList[SENSOR_OUTSIDE].address, 2)) {
    Serial.println("Unable to find address for Device outside");
  }

  sensors.setCheckForConversion(false);
  sensors.requestTemperatures();
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
  if (sensors.isConversionComplete())
  {
    Serial.print(" Inside temp: ");
    Serial.println(getTemp(SENSOR_INSIDE).value);
    Serial.print("  Pipe temp: ");
    Serial.println(getTemp(SENSOR_PIPE).value);
    Serial.print("Outside temp: ");
    Serial.println(getTemp(SENSOR_OUTSIDE).value);

    sensors.requestTemperatures();
  }
}
