#include <Arduino.h>

#include "main.h"

#include "aoCalc.h"
#include "auto_tune_pid.h"
#include "devModbus.h"
#include "microLAN.h"


word Events;

unsigned long mSeconds;
unsigned long mSec200;
unsigned long Seconds;
void setup() {

  Events = 0;

  Serial.begin(38400);

  setupOneWire();
  setupModbus();
  setupPID();

  Serial.println("Setup compelete!");
  mSeconds = millis();
  Seconds = 0;
  mSec200 = 0;
}

void loop() {

  long mSec = millis();
  if ((mSec - mSeconds) > 200) {
    mSeconds += 200;
    if (++mSec200 % 5 == 0) {
      Events |= EV_TIMER_SEC;
      if (++Seconds % 10 == 0) {
        Events |= EV_REQUEST_CONVERSION;
      }
    }
    Events |= EV_PID;
  }
  loopOneWire();
  loopModbus();
  loopPID();
}
