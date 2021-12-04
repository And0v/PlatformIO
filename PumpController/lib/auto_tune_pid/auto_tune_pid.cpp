
#include <Arduino.h>

#include "main.h"

#include "aoCalc.h"
#include "aoEEPROM.h"
#include "devModbus.h"
#include "microLAN.h"
#include <PID_AutoTune_v0.h>
#include <PID_v1.h>

byte ATuneModeRemember = 2;
double input = 0, output = 0, setpoint = 34;
double kp = 1.0, ki = 0.01, kd = 0.0001;

double kpmodel = 1.5, taup = 100, theta[50];
double outputStart = 5;
double aTuneStep = 30, aTuneNoise = 1, aTuneStartValue = 50;
unsigned int aTuneLookBack = 20;

boolean tuning = false;
unsigned long modelTime, serialTime;

PID myPID(&input, &output, &setpoint, kp, ki, kd, DIRECT);
PID_ATune aTune(&input, &output);

void AutoTuneHelper(boolean start);
void changeAutoTune();
void printBytesAsHex(byte *data, byte cnt);

/////////////////////
const int powerPin = 13;
unsigned long powerPeriod = 60000;
unsigned long powerTime = 0;
unsigned long powerTimeLast = 0;
float powerSum = 0.0;
float powerSumReg = 0.0;
/////////////////////////////////////////////////////
void readEEPROMData();

void setupPID() {
  // Setup the pid
  myPID.SetOutputLimits(10, 150);
  myPID.SetSampleTime(powerPeriod);
  readEEPROMData();
  // myPID.SetMode(AUTOMATIC);
  // myPID.SetTunings(kp, ki, kd);

  if (tuning) {
    tuning = false;
    changeAutoTune();
    tuning = true;
  }
  digitalWrite(powerPin, 1);
  pinMode(powerPin, OUTPUT);

  serialTime = 0;
}
void SerialSend();
void SerialReceive();
void setOutputPower(float);

void loopPID() {
  if (!(Events & EV_PID)) {
    return;
  }
  Events &= ~EV_PID;

  SensorDef pipe = sensorsValues[SENSOR_PIPE_SUPPLY];
  word inputPort = sensorsCalc[SENSOR_PIPE_RETURN].port;

  if ((inputPort < 0) && (inputPort >= SENSORS_COUNT)) {
    Serial.print(F("Incorrect inputIndex: "));
    Serial.println(SENSOR_PIPE_RETURN);
    output = 50;
    setOutputPower(pipe.value);
    return;
  }

  CalcDef sensor = sensorsCalc[SENSOR_PIPE_RETURN];
  if (sensor.state != CALC_STATE_OK) {
    if (sensor.state == CALC_STATE_INIT) {
      Serial.print(F("Waiting for calc! Sensor Index: "));
    } else {
      Serial.print(F("Input sensor error! Sensor Index: "));
    }
    Serial.println(SENSOR_PIPE_RETURN);
    output = 20;
    setOutputPower(90);
    return;
  }
  CalcDef sensorS = sensorsCalc[SENSOR_PIPE_SUPPLY];
  if (sensorS.state != CALC_STATE_OK) {
    if (sensorS.state == CALC_STATE_INIT) {
      Serial.print(F("Waiting for calc! Sensor Index: "));
    } else {
      Serial.print(F("Input sensor error! Sensor Index: "));
    }
    Serial.println(SENSOR_PIPE_SUPPLY);
    setOutputPower(90);
    return;
  }

  input = sensorsValues[SENSOR_PIPE_RETURN].value;
  // mb_Hreg(PID_INPUT_HREG, (float)input);

  if (tuning) {
    byte val = (aTune.Runtime());
    if (val != 0) {
      tuning = false;
    }
    if (!tuning) { // we're done, set the tuning parameters
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
  } else {
    myPID.Compute();
    if (myPID.GetMode() == AUTOMATIC) {
      // mb_Hreg(PID_OUTPUT_HREG, (float)output);
    }
  }
  setOutputPower(pipe.value+input);
  // mb_Hreg(PID_PWR_SUM_HREG, (float)powerSumReg);

  // send-receive with processing if it's time
  if (millis() > serialTime) {
    SerialReceive();
    SerialSend();
    serialTime += 500;
  }
}

void changeAutoTune() {
  if (!tuning) {
    // Set the output to the desired starting frequency.
    output = aTuneStartValue;
    aTune.SetNoiseBand(aTuneNoise);
    aTune.SetOutputStep(aTuneStep);
    aTune.SetLookbackSec((int)aTuneLookBack);
    AutoTuneHelper(true);
    tuning = true;
  } else { // cancel autotune
    aTune.Cancel();
    tuning = false;
    AutoTuneHelper(false);
  }
  // mb_Hreg(PID_AT_HREG, (word)(tuning?1:0));
}

void AutoTuneHelper(boolean start) {
  if (start)
    ATuneModeRemember = myPID.GetMode();
  else {
    myPID.SetMode(ATuneModeRemember);
    // mb_Hreg(PID_MOD_HREG, (word)myPID.GetMode());
  }
}

void SerialSend() {
  // Serial.print("setpoint: ");Serial.print(setpoint); Serial.print(" ");
  // Serial.print("input: ");Serial.print(input); Serial.print(" ");
  // Serial.print("output: ");Serial.print(output); Serial.print(" ");
  if (tuning) {
    Serial.println(F("tuning mode"));
  } else {
    // Serial.print("ISum: "); Serial.print(myPID.getISum());Serial.println();

    // Serial.print("kp: ");Serial.print(myPID.GetKp());Serial.print(" ");
    // Serial.print("ki: ");Serial.print(myPID.GetKi());Serial.print(" ");
    // Serial.print("kd: ");Serial.print(myPID.GetKd());Serial.println();
  }
}

void SerialReceive() {
  if (Serial.available()) {
    char b = Serial.read();
    Serial.flush();
    if ((b == '1' && !tuning) || (b != '1' && tuning))
      changeAutoTune();
  }
}

void setOutputPower(float pipe) {

  unsigned long now = millis();

  if (powerTime == 0) {
    powerTime = now;
  }
  if ((now - powerTime) > powerPeriod) {
    powerTime += powerPeriod;
  }
  unsigned long powerTimeNow = now - powerTime;
  if (powerTimeLast > powerTimeNow) {
    powerTimeLast = 0;
    powerSum = 0;
  }

  powerSum +=
      ((float)(powerTimeNow - powerTimeLast)) * pipe / ((float)powerPeriod);

  if (powerSum < output) {
    powerSumReg = powerSum;
    digitalWrite(powerPin, 0);
  } else {
    powerSumReg = 0.0;
    digitalWrite(powerPin, 1);
  }
  powerTimeLast = powerTimeNow;
}

bool rwModeHreg(byte mode, word offset, byte *data, word len) {
  Serial.print(F("- rwMODE: "));
  if (len != 2) {
    return false;
  } else if ((mode == EE_READ)) {
    int md = myPID.GetMode();
    Serial.print(F("read, "));
    Serial.println(md);
    *((word *)data) = md;
  } else if (mode == EE_WRITE) {
    Serial.print(F("write, "));
    word value = *((word *)data);
    Serial.print(value);
    if (((value & ~1) == 0) && ((word)myPID.GetMode() != value)) {
      myPID.SetMode(value);
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(F(" - OK"));
    } else {
      Serial.println(F(" - Error"));
      return false;
    }
  }
  return true;
}

bool rwOutputHreg(byte mode, word offset, byte *data, word len) {
  Serial.print(F("- rwOUTPUT: "));
  if (len != 4) {
    return false;
  } else if ((mode == EE_READ)) {
    Serial.print(F("read, "));
    Serial.println(output, 3);
    *((float *)data) = output;
  } else if (mode == EE_WRITE) {
    Serial.print(F("write, "));
    float value = *((float *)data);
    Serial.print(value, 3);
    if ((value != NAN) && (value >= 0) && (value <= 100)) {
      output = value;
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(F(" - OK"));
    } else {
      Serial.println(F(" - Error"));
      return false;
    }
  }
  return true;
}

bool rwSetPointHreg(byte mode, word offset, byte *data, word len) {
  Serial.print(F("- rwSETPOINT: "));
  if (len != 4) {
    return false;
  } else if ((mode == EE_READ)) {
    Serial.print(F("read, "));
    Serial.println(setpoint, 3);
    *((float *)data) = setpoint;
  } else if (mode == EE_WRITE) {
    Serial.print(F("write, "));
    float value = *((float *)data);
    Serial.print(value, 3);
    if ((value != NAN) && (value >= 25) && (value <= 80)) {
      setpoint = value;
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(F(" - OK"));
    } else {
      Serial.println(F(" - Error"));
      return false;
    }
  }
  return true;
}

bool rwKpHreg(byte mode, word offset, byte *data, word len) {
  Serial.print(F("- rwKP: "));
  if (len != 4) {
    return false;
  } else if ((mode == EE_READ)) {
    Serial.print(F("read, "));
    Serial.println(kp, 3);
    *((float *)data) = kp;
  } else if (mode == EE_WRITE) {
    Serial.print(F("write, "));
    float value = *((float *)data);
    Serial.print(value, 3);
    if ((value != NAN) && (value > 0.0)) {
      kp = value;
      myPID.SetTunings(kp, ki, kd);
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(F(" - OK"));
    } else {
      Serial.println(F(" - Error"));
      return false;
    }
  }
  return true;
}
bool rwKiHreg(byte mode, word offset, byte *data, word len) {
  Serial.print(F("- rwKI: "));
  if (len != 4) {
    return false;
  } else if ((mode == EE_READ)) {
    Serial.print(F("read, "));
    Serial.println(ki, 3);
    *((float *)data) = ki;
  } else if (mode == EE_WRITE) {
    Serial.print(F("write, "));
    float value = *((float *)data);
    Serial.print(value, 3);
    if ((value != NAN) && (value >= 0.0)) {
      ki = value;
      myPID.SetTunings(kp, ki, kd);
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(F(" - OK"));
    } else {
      Serial.println(F(" - Error"));
      return false;
    }
  }
  return true;
}
bool rwKdHreg(byte mode, word offset, byte *data, word len) {
  Serial.print(F("- rwKD: "));
  if (len != 4) {
    return false;
  } else if ((mode == EE_READ)) {
    Serial.print(F("read, "));
    Serial.print(kd, 3);
    *((float *)data) = kd;
    Serial.print(F(", "));
    printBytesAsHex(data, 4);
    Serial.println();
  } else if (mode == EE_WRITE) {
    Serial.print(F("write, "));
    float value = *((float *)data);
    Serial.print(value, 3);
    Serial.print(F(", "));
    printBytesAsHex(data, 4);
    if ((value != NAN) && (value >= 0.0)) {
      kd = value;
      myPID.SetTunings(kp, ki, kd);
      rwEEMEM(EE_WRITE, offset, data, len);
      Serial.println(F(" - OK"));
    } else {
      Serial.println(F(" - Error"));
      return false;
    }
  }
  return true;
}
bool rwSmoothHreg(byte mode, word offset, byte *data, word len) {
  word *smooth = (word *)offset;
  Serial.print(F("- rwSmooth: "));
  if (len != 2) {
    return false;
  } else if ((mode == EE_READ)) {
    Serial.print(F("read, "));
    Serial.println(*smooth, 3);
    *((word *)data) = *smooth;
  } else if (mode == EE_WRITE) {
    word ee_offset = 65535;
    for (byte i = 0; i < SENSORS_COUNT; ++i) {
      if (sensorsCalc[i].smooth == offset) {
        ee_offset = EE_SMOOTH + i * 2;
      }
    }
    Serial.print(F("write, "));
    word value = *((word *)data);
    Serial.print(value);
    if ((value < 10) && (ee_offset != 65535)) {
      *smooth = value;
      rwEEMEM(EE_WRITE, ee_offset, data, len);
      Serial.println(F(" - OK"));
    } else {
      Serial.println(F(" - Error"));
      return false;
    }
  }
  return true;
}

bool rwIndexHreg(byte mode, word offset, byte *data, word len) {
  word *index = (word *)offset;
  Serial.print(F("- rwIndex: "));
  if (len != 2) {
    return false;
  } else if ((mode == EE_READ)) {
    Serial.print(F("read, "));
    Serial.println(*index, 3);
    *((word *)data) = *index;
  } else if (mode == EE_WRITE) {
    word ee_offset = 65535;
    for (byte i = 0; i < SENSORS_COUNT; ++i) {
      if (sensorsCalc[i].port == offset) {
        ee_offset = EE_INDEX + i * 2;
      }
    }
    Serial.print(F("write, "));
    word value = *((word *)data);
    Serial.print(value);
    if ((value < 10) && (ee_offset != 65535)) {
      *index = value;
      rwEEMEM(EE_WRITE, ee_offset, data, len);
      Serial.println(F(" - OK"));
    } else {
      Serial.println(F(" - Error"));
      return false;
    }
  }
  return true;
}

bool rwUpdateHreg(byte mode, word offset, byte *data, word len) {
  Serial.print(F("- rwREINIT: "));
  if (len != 2) {
    return false;
  } else if ((mode == EE_READ)) {
    Serial.print(F("read, "));
    Serial.println(0);
    *((word *)data) = 0;
  } else if (mode == EE_WRITE) {
    Serial.print(F("write, "));
    word value = *((word *)data);
    Serial.print(value);
    Serial.print(F(", "));
    printBytesAsHex(data, 2);
    if (value == 0x55AA) {
      readEEPROMData();
      Serial.println(F(" - OK"));
    } else {
      Serial.println(F(" - Error"));
      return false;
    }
  }
  return true;
}
void readEEPROMData() {
  Serial.println(F("readEEPROMData"));
  word value;

  for (byte i = 0; i < SENSORS_COUNT; ++i) {
    rwEEMEM(EE_READ, EE_INDEX, (byte *)&value, 2);
    if (value >= SENSORS_COUNT) {
      value = i;
    }
    Serial.print(F("sensorsCalc["));
    Serial.print(i);
    Serial.print(F("].port: "));
    Serial.println(value);
    sensorsCalc[i].port = value;
  }

  rwEEMEM(EE_READ, EE_MODE, (byte *)&value, 2);
  if (value > 1) {
    value = 0;
  }
  Serial.print(F("PID mode: "));
  Serial.println(value);
  myPID.SetMode(value);


  rwEEMEM(EE_READ, EE_SETPNT, (byte *)&setpoint, 4);
  if ((isnan(setpoint)) || (setpoint < 30) || (setpoint > 60)) {
    setpoint = 34;
  }
  Serial.print(F("setpoint: "));
  Serial.println(setpoint);  
  
  rwEEMEM(EE_READ, EE_OUTPUT, (byte *)&output, 4);
  if ((isnan(output)) || (output < 0) || (output > 100)) {
    output = 20;
  }
  Serial.print(F("output: "));
  Serial.println(output);

  rwEEMEM(EE_READ, EE_KP, (byte *)&kp, 4);
  if ((isnan(kp)) || (kp <= 0)) {
    kp = 1;
  }
  Serial.print(F("PID kP: "));
  Serial.println(kp, 4);  

  rwEEMEM(EE_READ, EE_KI, (byte *)&ki, 4);
  if ((isnan(ki)) || (ki < 0)) {
    ki = 0;
  }
  Serial.print(F("PID kI: "));
  Serial.println(ki, 4);  

  rwEEMEM(EE_READ, EE_KD, (byte *)&kd, 4);
  if ((isnan(kd)) || (kd < 0)) {
    kd = 0;
  }
  Serial.print(F("PID kD: "));
  Serial.println(kd, 4);  

  myPID.SetTunings(kp, ki, kd);
}

void printBytesAsHex(byte *data, byte cnt) {
  for (int i = 0; i < cnt; ++i) {
    byte b = data[i];
    if (b < 0x10) {
      Serial.print("0");
    }
    Serial.print(b, 16);
    Serial.print(" ");
  }
}