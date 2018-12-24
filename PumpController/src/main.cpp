#include <Arduino.h>
#include "main.h"
#include "auto_tune.h"
#include "microLAN.h"
#include "devModbus.h"

word Events;

unsigned long mSeconds;
unsigned long Seconds;
void setup() {

  Events = 0;

  Serial.begin(38400);

  // setupAT();
  setupOneWire();
  setupModbus();

  Serial.println("Setup compelete!");
  mSeconds = millis();
  Seconds = 0;
}



void loop() {

  long mSec = millis();
  if ((mSec - mSeconds) > 1000){
    mSeconds += 1000;
    Events |= EV_TIMER_SEC;
    if (++Seconds % 10 == 0){
      Events |= EV_TIMER_10SEC;
      Events |= EV_REQUEST_CONVERSION;
    }
  }

  // loopAT();
  loopOneWire();
  loopModbus();
}
