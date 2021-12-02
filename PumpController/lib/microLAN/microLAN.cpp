#include <Arduino.h>
#include <OneWire.h>

#include "main.h"

#include "aoCalc.h"
#include "microLAN.h"

#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);
SensorDef sensorsValues[SENSORS_COUNT];
DeviceAddress sensorsPorts[SENSORS_COUNT];
StaticticsDef statistics[SENSORS_COUNT] = {
    // smooth, up, down, value, prev, sum, avg, dUp, dDown
    {4, 0, 0, 0, 0, 0, 0, 0},
    {4, 0, 0, 0, 0, 0, 0, 0}};
CalcDef sensorsCalc[SENSORS_COUNT] = {
    // port; status; smooth; adc0; adc; adcSum;
    // port - index of sensors
    {0, CALC_STATE_INIT, 4, 0, 0},
    {1, CALC_STATE_INIT, 4, 0, 0}};
// {2, CALC_STATE_INIT, 4, 0, 0},
// {3, CALC_STATE_INIT, 4, 0, 0}};

void setupOneWire() {
  // Start up the library
  sensors.begin();
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount());
  Serial.println(" sensors");

  // search for devices on the bus and assign based on an index
  for (byte i = 0; i < SENSORS_COUNT; ++i) {
    word port = sensorsCalc[i].port;
    if (port >= SENSORS_COUNT) {
      continue;
    }

    if (!sensors.getAddress(sensorsPorts[port], i)) {
      Serial.print("Unable to find address for Device ");
      Serial.println(i);
    } else {
      Serial.print(i);
      Serial.print(": port=");
      Serial.print(port);
      Serial.print(" ");
      for (size_t ch = 0; ch < sizeof(DeviceAddress); ch++)
      {
        uint8_t b8 = sensorsPorts[i][ch];
        if (b8 < 16){
          Serial.print("0");
        }
        Serial.print(b8, 16);
      }
      Serial.println();
      
    }
  }
  sensors.setResolution(12);

  sensors.setCheckForConversion(false);
  sensors.setWaitForConversion(false);

  Events |= EV_REQUEST_CONVERSION;
}

void loopOneWire() {
  if (Events & EV_REQUEST_CONVERSION) {
    Events &= ~EV_REQUEST_CONVERSION;
    sensors.requestTemperatures();
    Events |= EV_START_CONVERSION;
  }
  if ((Events & EV_START_CONVERSION) && (sensors.isConversionComplete())) {
    Events &= ~EV_START_CONVERSION;

    for (byte i = 0; i < SENSORS_COUNT; ++i) {
      Serial.print("Temperature ");
      Serial.print(i);
      Serial.print(": raw=");
      int16_t raw = sensors.getTemp(sensorsPorts[i]);
      Serial.print(raw);
      Serial.print(", ");
     
      word pState = sensorsCalc[i].state;
      addCalcValue(&sensorsCalc[i], raw, DEVICE_DISCONNECTED_RAW, 0x3ff8);
      if (sensorsCalc[i].state == CALC_STATE_OK) {
        sensorsValues[i].value0 = sensors.rawToCelsius(sensorsCalc[i].adc0);
        Serial.print(sensorsValues[i].value0, 2);
        sensorsValues[i].value = sensors.rawToCelsius(sensorsCalc[i].adc);
        Serial.print(", ");
        Serial.print(sensorsValues[i].value, 2);
        calcStatistics(sensorsCalc[i], statistics[i], pState != CALC_STATE_OK);
        Serial.print(", ");
        Serial.print(statistics[i].valueAvg);
        Serial.print(", ");
        Serial.print(statistics[i].divUp);
        Serial.print(", ");
        Serial.println(statistics[i].divDown);

      } else {
        Serial.println("error! ");
      }
    }
    Events |= EV_CONVERSION_COMPLETE;
  }
}
