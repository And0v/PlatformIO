
#include <Arduino.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include "main.h"
#include "microLAN.h"
#include "devModbus.h"


byte ATuneModeRemember=2;
double input=0, output=0, setpoint=55;
double kp=0.5,ki=0.1,kd=.05;

double kpmodel=1.5, taup=100, theta[50];
double outputStart=5;
double aTuneStep=30, aTuneNoise=1, aTuneStartValue=50;
unsigned int aTuneLookBack=20;

boolean tuning = false;
unsigned long  modelTime, serialTime;

PID myPID(&input, &output, &setpoint,kp,ki,kd, DIRECT);
PID_ATune aTune(&input, &output);

word inputIndex;


void AutoTuneHelper(boolean start);
void changeAutoTune();

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
  myPID.SetOutputLimits(30, 100);
  myPID.SetSampleTime(30000);

  if(tuning)
  {
    tuning=false;
    changeAutoTune();
    tuning=true;
  }
  pinMode(powerPin, OUTPUT);

  mb_Hreg(PID_AT_HREG, (word)(tuning?1:0));
  mb_Hreg(PID_MOD_HREG, (word)myPID.GetMode());
  mb_Hreg(PID_OUTPUT_HREG, (float)output);
  mb_Hreg(PID_KI_HREG, (float)myPID.GetKi());
  mb_Hreg(PID_KP_HREG, (float)myPID.GetKp());
  mb_Hreg(PID_KD_HREG, (float)myPID.GetKd());
  mb_Hreg(PID_SETPOINT_HREG, (float)setpoint);
  serialTime = 0;
}
void SerialSend();
void SerialReceive();
void setOutputPower();

void readPIDParams(){

  Serial.print("readPIDParams: update = ");
  word update = mb.Hreg(PID_UPDATE_HREG);
  Serial.println(update);
  if (update != 0){
    Serial.println("Updating PID");
    if (update & PID_UPADTE_AT){
      Serial.print("- AT: ");
      word value = mb.Hreg(PID_AT_HREG);
      Serial.print(value);
      if ((value == 1)&&(!tuning)){
        changeAutoTune();
        Serial.println(" - OK");
      }else if ((value != 1)&&(tuning)){
        changeAutoTune();
        Serial.println(" - OK");
      }else{
        Serial.println(" - Error");
      }
    }
    if (update & PID_UPADTE_MOD){
      Serial.print("- MODE: ");
      word value = mb.Hreg(PID_MOD_HREG);
      Serial.print(value);
      if (((value & ~1)==0)&&((word)myPID.GetMode() != value)){
        myPID.SetMode(value);
        Serial.println(" - OK");
      }else{
        Serial.println(" - Error");
      }
    }
    if (update & PID_UPADTE_IDX){
      Serial.print("- INDEX: ");
      word value = mb.Hreg(PID_INDEX_HREG);
      Serial.print(value);
      if ((value >=0)&&(value< SENSORS_COUNT)){
        inputIndex = value;
        Serial.println(" - OK");
      }else{
        Serial.println(" - Error");
      }
    }
    if (update & PID_UPADTE_OUT){
      Serial.print("- OUTPUT: ");
      float fValue = mb_Hreg(PID_OUTPUT_HREG);
      Serial.print(fValue);
      if ((fValue != NAN)&&(fValue >=0)&&(fValue<=100)){
        output = fValue;
        Serial.println(" - OK");
      }else{
        Serial.println(" - Error");
      }
    }
    if (update & PID_UPADTE_SET){
      Serial.print("- SETPOINT: ");
      float fValue = mb_Hreg(PID_SETPOINT_HREG);
      Serial.print(fValue);
      if ((fValue != NAN)&&(fValue >=30)&&(fValue<=80)){
        setpoint = fValue;
        Serial.println(" - OK");
      }else{
        Serial.println(" - Error");
      }
    }

    byte pidUpdate = 0;
    if (update & PID_UPADTE_KP){
      Serial.print("- KP: ");
      float fValue = mb_Hreg(PID_KP_HREG);
      Serial.print(fValue, 3);
      if ((fValue != NAN)&&(fValue > 0.0)){
        kp = fValue;
        ++pidUpdate;
        Serial.println(" - OK");
      }else{
        Serial.println(" - Error");
      }
    }
    if (update & PID_UPADTE_KI){
      Serial.print("- KI: ");
      float fValue = mb_Hreg(PID_KI_HREG);
      Serial.print(fValue, 3);
      if ((fValue != NAN)&&(fValue > 0.0)){
        ki = fValue;
        ++pidUpdate;
        Serial.println(" - OK");
      }else{
        Serial.println(" - Error");
      }
    }
    if (update & PID_UPADTE_KD){
      Serial.print("- KD: ");
      float fValue = mb_Hreg(PID_KD_HREG);
      Serial.print(fValue, 3);
      if ((fValue != NAN)&&(fValue > 0.0)){
        ki = fValue;
        ++pidUpdate;
        Serial.println(" - OK");
      }else{
        Serial.println(" - Error");
      }
    }
    if (pidUpdate != 0){
      myPID.SetTunings(kp,ki,kd);
    }
    mb_Hreg(PID_UPDATE_HREG, (word)0);
  }
}


void loopPID()
{
  if(!(Events & EV_PID)){
    return;
  }
  Events &= ~EV_PID;

  readPIDParams();

  if ((inputIndex <0)&&(inputIndex>=SENSORS_COUNT)){
    Serial.print("Incorrect inputIndex: ");
    Serial.println(inputIndex);
    output = 50;
    setOutputPower();
    return;
  }

  SensorDef sensor = sensorsList[inputIndex];
  if ( sensor.status != SENSOR_STATUS_OK){
    Serial.print("Input sensor error! Sensor Index: ");
    Serial.println(inputIndex);
    output = 50;
    setOutputPower();
    return;
  }
  input = sensor.value;
  mb_Hreg(PID_INPUT_HREG, (float)input);

  if(tuning)
  {
    byte val = (aTune.Runtime());
    if (val!=0)
    {
      tuning = false;
    }
    if(!tuning)
    { //we're done, set the tuning parameters
      kp = aTune.GetKp();
      ki = aTune.GetKi();
      kd = aTune.GetKd();
      myPID.SetTunings(kp,ki,kd);
      mb_Hreg(PID_KI_HREG, (float)myPID.GetKi());
      mb_Hreg(PID_KP_HREG, (float)myPID.GetKp());
      mb_Hreg(PID_KD_HREG, (float)myPID.GetKd());
      AutoTuneHelper(false);
    }
    mb_Hreg(PID_OUTPUT_HREG, (float)output);
  }
  else {
    myPID.Compute();
    if (myPID.GetMode() == AUTOMATIC){
      mb_Hreg(PID_OUTPUT_HREG, (float)output);
    }
  }
  setOutputPower();

  //send-receive with processing if it's time
  if(millis()>serialTime)
  {
    SerialReceive();
    SerialSend();
    serialTime+=500;
  }
}

void changeAutoTune()
{
 if(!tuning)
  {
    //Set the output to the desired starting frequency.
    output=aTuneStartValue;
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
  mb_Hreg(PID_AT_HREG, (word)(tuning?1:0));
}

void AutoTuneHelper(boolean start)
{
  if(start)
    ATuneModeRemember = myPID.GetMode();
  else{
    myPID.SetMode(ATuneModeRemember);
    mb_Hreg(PID_MOD_HREG, (word)myPID.GetMode());
  }
}

void SerialSend()
{
  Serial.print("setpoint: ");Serial.print(setpoint); Serial.print(" ");
  Serial.print("input: ");Serial.print(input); Serial.print(" ");
  Serial.print("output: ");Serial.print(output); Serial.print(" ");
  if(tuning){
    Serial.println("tuning mode");
  } else {
    Serial.print("kp: ");Serial.print(myPID.GetKp());Serial.print(" ");
    Serial.print("ki: ");Serial.print(myPID.GetKi());Serial.print(" ");
    Serial.print("kd: ");Serial.print(myPID.GetKd());Serial.println();
  }
}

void SerialReceive()
{
  if(Serial.available())
  {
   char b = Serial.read();
   Serial.flush();
   if((b=='1' && !tuning) || (b!='1' && tuning))changeAutoTune();
  }
}

void setOutputPower(){

  unsigned long now = millis();


  if (powerTime == 0){
    powerTime = now;
  }
  if ((now - powerTime)>powerPeriod){
    powerTime += powerPeriod;
  }
  float time = (float)(now-powerTime);

  if (time < output/100.0*powerPeriod){
    digitalWrite(powerPin, 1);
  }else{
    digitalWrite(powerPin, 0);
  }

}
