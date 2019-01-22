
#include <Arduino.h>
#include "main.h"
#include <OneWire.h>
#include "microLAN.h"
#include "EEPROM.h"

#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

// const DeviceAddress sensorsAddresses[SENSORS_COUNT] PROGMEM = {
//   {0x28,0xE8,0x27,0x4C,0x05,0x00,0x00,0xF8},
//   {0x28,0x37,0x0A,0x46,0x05,0x00,0x00,0x17},
//   {0x28,0x17,0xF2,0xCC,0x00,0x00,0x00,0xCE},
//   {0x28,0x89,0x44,0xCD,0x00,0x00,0x00,0x30},
//   {0x28,0x95,0x45,0xCD,0x00,0x00,0x00,0xDB},
//   {0x28,0x15,0x7B,0xCD,0x00,0x00,0x00,0x17},
//   {0x28,0x7D,0x37,0xCD,0x00,0x00,0x00,0xBD},
//   {0x28,0xB0,0xC1,0xCC,0x00,0x00,0x00,0xDD}
// };
int16_t sensorsList[SENSORS_COUNT];

void printAddress(DeviceAddress deviceAddress);
bool readAddress(byte idx, DeviceAddress * deviceAddress);

char e2str[] EEMEM = "Hello, world!\n";
void setupOneWire(){

  uint8_t *ptr = reinterpret_cast<uint8_t *>(e2str);
  while (true)
  {
      char c = eeprom_read_byte(ptr++);
      if (c == '\0') break;
      Serial.write(c);
  }


// Start up the library
  sensors.begin();

  // search for devices on the bus and assign based on an index
  for (byte i = 0; i < SENSORS_COUNT; ++i){
    DeviceAddress addr;
    readAddress(i, &addr);
    printAddress(addr);
    if (!sensors.isConnected(addr)) {
      Serial.print(" Unable to find address for Device ");
      Serial.println(i);
    }else{
      Serial.println(" Found address!");
    }
  }
  sensors.setCheckForConversion(false);
  Events |= EV_REQUEST_CONVERSION;
  //sensors.requestTemperatures();
}

int16_t getTemp(byte sensorIdx)
{
  DeviceAddress addr;
  readAddress(sensorIdx, &addr);
  int16_t tempC = sensors.getTemp(addr);
  sensorsList[sensorIdx] = tempC;
  return tempC;
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

    Serial.print("----------- Time ");
    Serial.println(millis());
    Serial.print("BOILER_OUT: ");
    Serial.println(getTemp(BOILER_OUT));
    Serial.print("BOILER_IN: ");
    Serial.println(getTemp(BOILER_IN));
    Serial.print("HYDROSREAM: ");
    Serial.print(getTemp(HYDROSREAM_1));
    Serial.print(" ");
    Serial.print(getTemp(HYDROSREAM_2));
    Serial.print(" ");
    Serial.println(getTemp(HYDROSREAM_3));
    Serial.print("FEED_PIPE: ");
    Serial.println(getTemp(FEED_PIPE));
    Serial.print("RETURN_PIPE: ");
    Serial.println(getTemp(RETURN_PIPE));
    Serial.print("OUTSIDE: ");
    Serial.println(getTemp(OUTSIDE));
    Events |= EV_CONVERSION_COMPLETE;
  }
}
// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

bool readAddress(byte idx, DeviceAddress * addr)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    (*addr)[i] = EEPROM.read(EEPROM_ADDRESSES+idx*8+i);
  }
  return true;
}
