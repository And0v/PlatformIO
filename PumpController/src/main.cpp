#include <Arduino.h>
#include "main.h"
#include "auto_tune_pid.h"
#include "microLAN.h"
#include "devModbus.h"

word Events;

unsigned long mSeconds;
unsigned long Seconds;
void setup() {

  Events = 0;

  Serial.begin(38400);

  setupOneWire();
  setupModbus();
  setupPID();

  Serial.println(sizeof(float));
  Serial.println(sizeof(double));
  Serial.println("Setup compelete!");
  mSeconds = millis();
  Seconds = 0;
}



void loop() {

  long mSec = millis();
  if ((mSec - mSeconds) > 1000){
    mSeconds += 1000;
    Events |= EV_TIMER_SEC;
    if (++Seconds % 5 == 0){
      Events |= EV_REQUEST_CONVERSION;
    }
    Events |= EV_PID;
  }
  loopOneWire();
  loopModbus();
  loopPID();
}
