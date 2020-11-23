
#include <Arduino.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include "main.h"
#include "auto_tune_pid.h"
#include "microLAN.h"
#include "devModbus.h"
#include "aoEEPROM.h"

byte ATuneModeRemember = 2;
double input = 0, output = 0, setpoint = 55;
double kp = 0.5, ki = 0.1, kd = .05;

double kpmodel = 1.5, taup = 100, theta[50];
double outputStart = 5;
double aTuneStep = 30, aTuneNoise = 1, aTuneStartValue = 50;
unsigned int aTuneLookBack = 20;

word tuning = false;
unsigned long modelTime, serialTime;

PID myPID(&input, &output, &setpoint, kp, ki, kd, DIRECT);
PID_ATune aTune(&input, &output);

word inputIndex;

void AutoTuneHelper(boolean start);
void changeAutoTune();
void readEEPROMData();

/////////////////////
const int powerPin = 13;
unsigned long powerPeriod = 30000;
unsigned long powerTime = 0;
/////////////////////////////////////////////////////

void setupPID()
{
  inputIndex = 0;
  //Setup the pid
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(5, 100);
  myPID.SetSampleTime(30000);

  if (tuning)
  {
    tuning = false;
    changeAutoTune();
    tuning = true;
  }
  digitalWrite(powerPin, 1);
  pinMode(powerPin, OUTPUT);

  readEEPROMData();

  serialTime = 0;
}
void SerialSend();
void SerialReceive();
void setOutputPower();



void loopPID()
{
  if (!(Events & EV_PID))
  {
    return;
  }
  Events &= ~EV_PID;

  if ((inputIndex < 0) && (inputIndex >= SENSORS_COUNT))
  {
    Serial.print("Incorrect inputIndex: ");
    Serial.println(inputIndex);
    output = 50;
    setOutputPower();
    return;
  }

  SensorDef sensor = sensorsList[inputIndex];
  if (sensor.status != SENSOR_STATUS_OK)
  {
    Serial.print("Input sensor error! Sensor Index: ");
    Serial.println(inputIndex);
    output = 50;
    setOutputPower();
    return;
  }
  input = sensor.value;
  // mb_Hreg(PID_INPUT_HREG, (float)input);

  if (tuning)
  {
    byte val = (aTune.Runtime());
    if (val != 0)
    {
      tuning = false;
    }
    if (!tuning)
    { //we're done, set the tuning parameters
      kp = aTune.GetKp();
      ki = aTune.GetKi();
      kd = aTune.GetKd();
      myPID.SetTunings(kp, ki, kd);
      // mb_Hreg(PID_KI_HREG, (float)myPID.GetKi());
      // mb_Hreg(PID_KP_HREG, (float)myPID.GetKp());
      // mb_Hreg(PID_KD_HREG, (float)myPID.GetKd());
      AutoTuneHelper(false);
    }
    // mb_Hreg(PID_OUTPUT_HREG, (float)output);
  }
  else
  {
    myPID.Compute();
    if (myPID.GetMode() == AUTOMATIC)
    {
      // mb_Hreg(PID_OUTPUT_HREG, (float)output);
    }
  }
  setOutputPower();

  //send-receive with processing if it's time
  if (millis() > serialTime)
  {
    SerialReceive();
    SerialSend();
    serialTime += 500;
  }
}

void changeAutoTune()
{
  if (!tuning)
  {
    //Set the output to the desired starting frequency.
    output = aTuneStartValue;
    aTune.SetNoiseBand(aTuneNoise);
    aTune.SetOutputStep(aTuneStep);
    aTune.SetLookbackSec((int)aTuneLookBack);
    AutoTuneHelper(true);
    tuning = true;
  }
  else
  { //cancel autotune
    aTune.Cancel();
    tuning = false;
    AutoTuneHelper(false);
  }
  // mb_Hreg(PID_AT_HREG, (word)(tuning ? 1 : 0));
}

void AutoTuneHelper(boolean start)
{
  if (start)
    ATuneModeRemember = myPID.GetMode();
  else
  {
    myPID.SetMode(ATuneModeRemember);
    // mb_Hreg(PID_MOD_HREG, (word)myPID.GetMode());
  }
}

void SerialSend()
{
  Serial.print("setpoint: ");
  Serial.print(setpoint);
  Serial.print(" ");
  Serial.print("input: ");
  Serial.print(input);
  Serial.print(" ");
  Serial.print("output: ");
  Serial.print(output);
  Serial.print(" ");
  if (tuning)
  {
    Serial.println("tuning mode");
  }
  else
  {
    Serial.print("kp: ");
    Serial.print(myPID.GetKp());
    Serial.print(" ");
    Serial.print("ki: ");
    Serial.print(myPID.GetKi());
    Serial.print(" ");
    Serial.print("kd: ");
    Serial.print(myPID.GetKd());
    Serial.println();
  }
}

void SerialReceive()
{
  if (Serial.available())
  {
    char b = Serial.read();
    Serial.flush();
    if ((b == '1' && !tuning) || (b != '1' && tuning))
      changeAutoTune();
  }
}

void setOutputPower()
{

  unsigned long now = millis();

  if (powerTime == 0)
  {
    powerTime = now;
  }
  if ((now - powerTime) > powerPeriod)
  {
    powerTime += powerPeriod;
  }
  float time = (float)(now - powerTime);

  if (time < output / 100.0 * powerPeriod)
  {
    digitalWrite(powerPin, 0);
  }
  else
  {
    digitalWrite(powerPin, 1);
  }
}

bool rwTuningHreg(byte mode, word offset, byte *data, word len)
{
  Serial.print("- rwAT: ");
  if (len != 2)
  {
    return false;
  }
  else if ((mode == EE_READ))
  {
    Serial.print("read, ");
    Serial.println(tuning);
    *((word *)data) = tuning;
  }
  else if (mode == EE_WRITE)
  {
    Serial.print("write, ");
    word value = *((word *)data);
    Serial.print(value);
    if (tuning != (bool)value)
    {
      changeAutoTune();
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(" - OK");
    }
    else
    {
      Serial.println(" - Error");
      return false;
    }
  }
  return true;
}
bool rwModeHreg(byte mode, word offset, byte *data, word len)
{
  Serial.print("- rwMODE: ");
  if (len != 2)
  {
    return false;
  }
  else if ((mode == EE_READ))
  {
    int md = myPID.GetMode();
    Serial.print("read, ");
    Serial.println(md);
    *((word *)data) = md;
  }
  else if (mode == EE_WRITE)
  {
    Serial.print("write, ");
    word value = *((word *)data);
    Serial.print(value);
    if (((value & ~1) == 0) && ((word)myPID.GetMode() != value))
    {
      myPID.SetMode(value);
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(" - OK");
    }
    else
    {
      Serial.println(" - Error");
      return false;
    }
  }
  return true;

}
bool rwIndexHreg(byte mode, word offset, byte *data, word len)
{
  Serial.print("- rwINDEX: ");
  if (len != 2)
  {
    return false;
  }
  else if ((mode == EE_READ))
  {
    Serial.print("read, ");
    Serial.println(inputIndex);
    *((word *)data) = inputIndex;
  }
  else if (mode == EE_WRITE)
  {
    Serial.print("write, ");
    word value = *((word *)data);
    Serial.print(value);
    if ((value >= 0) && (value < SENSORS_COUNT))
    {
      inputIndex = value;
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(" - OK");
    }
    else
    {
      Serial.println(" - Error");
      return false;
    }
  }
  return true;

}

bool rwOutputHreg(byte mode, word offset, byte *data, word len)
{
  Serial.print("- rwOUTPUT: ");
  if (len != 4)
  {
    return false;
  }
  else if ((mode == EE_READ))
  {
    Serial.print("read, ");
    Serial.println(output, 3);
    *((float *)data) = output;
  }
  else if (mode == EE_WRITE)
  {
    Serial.print("write, ");
    float value = *((float *)data);
    Serial.print(value, 3);
    if ((value != NAN) && (value >= 0) && (value <= 100))
    {
      output = value;
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(" - OK");
    }
    else
    {
      Serial.println(" - Error");
      return false;
    }
  }
  return true;

}

bool rwSetPointHreg(byte mode, word offset, byte *data, word len)
{
  Serial.print("- rwSETPOINT: ");
  if (len != 4)
  {
    return false;
  }
  else if ((mode == EE_READ))
  {
    Serial.print("read, ");
    Serial.println(setpoint, 3);
    *((float *)data) = setpoint;
  }
  else if (mode == EE_WRITE)
  {
    Serial.print("write, ");
    float value = *((float *)data);
    Serial.print(value, 3);
    if ((value != NAN) && (value >= 25) && (value <= 80))
    {
      setpoint = value;
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(" - OK");
    }
    else
    {
      Serial.println(" - Error");
      return false;
    }
  }
  return true;
}

bool rwKpHreg(byte mode, word offset, byte *data, word len)
{
  Serial.print("- rwKP: ");
  if (len != 4)
  {
    return false;
  }
  else if ((mode == EE_READ))
  {
    Serial.print("read, ");
    Serial.println(kp, 3);
    *((float *)data) = kp;
  }
  else if (mode == EE_WRITE)
  {
    Serial.print("write, ");
    float value = *((float *)data);
    Serial.print(value, 3);
    if ((value != NAN) && (value > 0.0))
    {
      kp = value;
      myPID.SetTunings(kp, ki, kd);
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(" - OK");
    }
    else
    {
      Serial.println(" - Error");
      return false;
    }
  }
  return true;
}
bool rwKiHreg(byte mode, word offset, byte *data, word len)
{
  Serial.print("- rwKI: ");
  if (len != 4)
  {
    return false;
  }
  else if ((mode == EE_READ))
  {
    Serial.print("read, ");
    Serial.println(ki, 3);
    *((float *)data) = ki;
  }
  else if (mode == EE_WRITE)
  {
    Serial.print("write, ");
    float value = *((float *)data);
    Serial.print(value, 3);
    if ((value != NAN) && (value >= 0.0))
    {
      ki = value;
      myPID.SetTunings(kp, ki, kd);
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(" - OK");
    }
    else
    {
      Serial.println(" - Error");
      return false;
    }
  }
  return true;
}
bool rwKdHreg(byte mode, word offset, byte *data, word len)
{
  Serial.print("- rwKD: ");
  if (len != 4)
  {
    return false;
  }
  else if ((mode == EE_READ))
  {
    Serial.print("read, ");
    Serial.println(kd, 3);
    *((float *)data) = kd;
  }
  else if (mode == EE_WRITE)
  {
    Serial.print("write, ");
    float value = *((float *)data);
    Serial.print(value, 3);
    if ((value != NAN) && (value >= 0.0))
    {
      kd = value;
      myPID.SetTunings(kp, ki, kd);
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(" - OK");
    }
    else
    {
      Serial.println(" - Error");
      return false;
    }
  }
  return true;
}

bool rwUpdateHreg(byte mode, word offset, byte *data, word len){
  Serial.print("- rwREINIT: ");
  if (len != 2)
  {
    return false;
  }
  else if ((mode == EE_READ))
  {
    Serial.print("read, ");
    Serial.println(0);
    *((word *)data) = 0;
  }
  else if (mode == EE_WRITE)
  {
    Serial.print("write, ");
    word value = *((word *)data);
    Serial.print(value);
    if (value == 0x55AA)
    {
      readEEPROMData();
      Serial.println(" - OK");
    }
    else
    {
      Serial.println(" - Error");
      return false;
    }
  }
  return true;
}
void readEEPROMData()
{
  Serial.print("readEEPROMData");
  word value;
  rwEEMEM(EE_READ, EE_AT, (byte *)&tuning, 2);
  rwEEMEM(EE_READ, EE_MODE, (byte *)&value, 2);
  myPID.SetMode(value);
  rwEEMEM(EE_READ, EE_INP_IDX, (byte *)&inputIndex, 2);
  rwEEMEM(EE_READ, EE_OUTPUT, (byte *)&output, 4);
  rwEEMEM(EE_READ, EE_SETPNT, (byte *)&setpoint, 4);
  rwEEMEM(EE_READ, EE_KP, (byte *)&kp, 4);
  rwEEMEM(EE_READ, EE_KI, (byte *)&ki, 4);
  rwEEMEM(EE_READ, EE_KD, (byte *)&kd, 4);
  myPID.SetTunings(kp, ki, kd);
}