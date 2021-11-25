
#include <Arduino.h>
#include "main.h"
#include <OneWire.h>
#include "microLAN.h"
#include "aoCalc.h"

#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
SensorDef sensorsValues[SENSORS_COUNT];
DeviceAddress sensorsPorts[SENSORS_COUNT];
CalcDef sensorsCalc[SENSORS_COUNT] = {
    // port; status; smooth; adc0; adc; adcSum;
    // port - index of sensors
    {0, CALC_STATE_INIT, 4, 0, 0},
    {1, CALC_STATE_INIT, 4, 0, 0},
    {2, CALC_STATE_INIT, 4, 0, 0},
    {3, CALC_STATE_INIT, 4, 0, 0}};

void setupOneWire()
{
  // Start up the library
  sensors.begin();

  // search for devices on the bus and assign based on an index
  for (byte i = 0; i < SENSORS_COUNT; ++i)
  {
    word port = sensorsCalc[i].port;
    if (port >= SENSORS_COUNT)
    {
      continue;
    }

    if (!sensors.getAddress(sensorsPorts[port], i))
    {
      Serial.print("Unable to find address for Device ");
      Serial.println(i);
    }
    else
    {
      memset(sensorsPorts[port], 0, sizeof(DeviceAddress));
    }
  }
  sensors.setCheckForConversion(false);
  sensors.setWaitForConversion(false);
  Events |= EV_REQUEST_CONVERSION;
}

void loopOneWire()
{
  if (Events & EV_REQUEST_CONVERSION)
  {
    Events &= ~EV_REQUEST_CONVERSION;
    sensors.requestTemperatures();
    Events |= EV_START_CONVERSION;
  }
  if ((Events & EV_START_CONVERSION) && (sensors.isConversionComplete()))
  {
    Events &= ~EV_START_CONVERSION;

    for (byte i = 0; i < SENSORS_COUNT; ++i)
    {
      Serial.print("Temperature ");
      Serial.print(i);
      Serial.print(": ");
      int16_t raw = sensors.getTemp(sensorsPorts[i]);
      addCalcValue(&sensorsCalc[i], raw, DEVICE_DISCONNECTED_RAW, 32768);
      if (sensorsCalc[i].state == CALC_STATE_OK)
      {
        sensorsValues[i].value0 = sensors.rawToCelsius(sensorsCalc[i].adc0);
        Serial.print(sensorsValues[i].value0, 2);
        sensorsValues[i].value = sensors.rawToCelsius(sensorsCalc[i].adc);
        Serial.print(", ");
        Serial.print(sensorsValues[i].value, 2);
        Serial.println(";");
      }
      else
      {
        Serial.println("error! ");
      }
    }
    Events |= EV_CONVERSION_COMPLETE;
  }
}
