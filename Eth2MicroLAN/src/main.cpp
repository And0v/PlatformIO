#include <Arduino.h>
#include "main.h"
#include "udp2modbus.h"
#include "microLAN.h"

word Events;

unsigned long mSeconds;
unsigned long Seconds;

void setup() {
  Serial.begin(38400);
  Serial.println(F("\n[EtherCard Client]"));
  setupU2MB();
  setupOneWire();
}


unsigned long time = 0;
void * p_Value = NULL;

void loop() {
  unsigned long mSec = millis();

  if (mSec > mSeconds){
    mSeconds += 1000;
    Events |= EV_TIMER_SEC;
    if (++Seconds % 30 == 0){
      Events |= EV_REQUEST_CONVERSION;
    }
    Events |= EV_PID;
  }
  loopOneWire();
  loopU2MB();
}

void printBuff(byte * buff, uint16_t len)
{
  for (uint8_t i = 0; i < len; ++i)
  {
    if (buff[i] < 16) Serial.print("0");
    Serial.print(buff[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}
