
#include <Arduino.h>
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>
#include "main.h"
#include "microLAN.h"

byte ATuneModeRemember=2;
double input=80, output=50, setpoint=70;
double kp=2,ki=0.5,kd=2;

double kpmodel=1.5, taup=100, theta[50];
double outputStart=5;
double aTuneStep=30, aTuneNoise=1, aTuneStartValue=100;
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
unsigned long powerPeriod = 20000;
unsigned long powerTime = 0;
/////////////////////////////////////////////////////


void setupPID()
{
  inputIndex = 0;
  //Setup the pid
  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(0, 100);

  if(tuning)
  {
    tuning=false;
    changeAutoTune();
    tuning=true;
  }
  pinMode(powerPin, OUTPUT);

  serialTime = 0;
}
void SerialSend();
void SerialReceive();
void DoModel();
void setOutputPower();

void loopPID()
{
  if(!(Events & EV_PID)){
    return;
  }
  Events &= ~EV_PID;

//  unsigned long now = millis();

  SensorDef sensor = sensorsList[inputIndex];
  if ( sensor.status != SENSOR_STATUS_OK){
    return;
  }
  input = sensor.value;

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
      AutoTuneHelper(false);
    }
  }
  else myPID.Compute();


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
}

void AutoTuneHelper(boolean start)
{
  if(start)
    ATuneModeRemember = myPID.GetMode();
  else
    myPID.SetMode(ATuneModeRemember);
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

void DoModel()
{
  //cycle the dead time
  for(byte i=0;i<49;i++)
  {
    theta[i] = theta[i+1];
  }
  //compute the input
  input = (kpmodel / taup) *(theta[0]-outputStart) + input*(1-1/taup) + ((float)random(-10,10))/100;

}

void setOutputPower(){

  unsigned long now = millis();


  if (powerTime == 0){
    powerTime = now;
  }
  if ((now - powerTime)>powerPeriod){
    powerTime += powerPeriod;
  }
  double time = (now-powerTime);

  if (time < output/100.0*powerPeriod){
    digitalWrite(powerPin, 1);
  }else{
    digitalWrite(powerPin, 0);
  }

}
