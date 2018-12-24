#include <Arduino.h>

#include "auto_tune.h"
#include "microLAN.h"
/*
word * reg = (word*)&insideTemp;
mb.Ireg(SENSOR_IREG, reg[0]);
mb.Ireg(SENSOR_IREG+1, reg[1]);
word * reg = (word*)&insideTemp;
mb.Ireg(SENSOR_IREG, reg[0]);
mb.Ireg(SENSOR_IREG+1, reg[1]);
*/
void setup() {
  Serial.begin(38400);

  // setupAT();
  setupOneWire();

  Serial.println("Setup compelete!");
}

void loop() {
  // loopAT();
  loopOneWire();
}
