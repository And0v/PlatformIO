
#include "Arduino.h"
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include "ModbusASCII.h"

#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);

byte ATuneModeRemember=2;
double input=0, output=0, setpoint=30;
double kp=2,ki=0.5,kd=2;

const double kpmodel=1.5, taup=100;
const double outputStart=5;
const double aTuneStep=25, aTuneNoise=1, aTuneStartValue=50;
const unsigned int aTuneLookBack=20;

boolean tuning = false ;
unsigned long  modelTime;

PID myPID(&input, &output, &setpoint,kp,ki,kd, DIRECT);
PID_ATune aTune(&input, &output);

void AutoTuneHelper(boolean start);
void SerialSend(Stream * port);
void SerialReceive(Stream * port);
void changeAutoTune();

/////////////////////////////////////////////////////////

DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

/////////////////////
const int powerPin = 13;
unsigned long powerPeriod = 10000;
unsigned long powerTime = 0;
/////////////////////////////////////////////////////

const int SENSOR_IREG = 100; //101


void setupOneWire(){
// Start up the library
  sensors.begin();

  // search for devices on the bus and assign based on an index
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0");

  sensors.setCheckForConversion(false);
  // set alarm ranges
//  sensors.setHighAlarmTemp(insideThermometer, 27);
//  sensors.setLowAlarmTemp(insideThermometer, 22);


  // attach alarm handler
//  sensors.setAlarmHandler(&newAlarmHandler);
  sensors.requestTemperatures();
}
#include "WiFiEsp.h"


char ssid[] = "VARZARA109";            // your network SSID (name)
char pass[] = "+380934540989";        // your network password
int status = WL_IDLE_STATUS;
WiFiEspServer server(333);

byte setupWiFi(byte step)
{
  // check for the presence of the shield
  if ((step ==0)&&(WiFi.status() == WL_NO_SHIELD)) {
    Serial.println("WiFi shield not present");
    // don't continue
    return 0;
  }
  if (step == 0) {step = 1;}
  // attempt to connect to WiFi network
  if ((step == 1)&&(status != WL_CONNECTED)) {
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
    if (status != WL_CONNECTED){
      return 1;
    }
  }
  if (step == 1) {step = 2;}

  Serial.println("You're connected to the network");
  // start the server on port 333
  server.begin();
  return 2;
}

void setup()
{
  //Setup the pid
  myPID.SetMode(AUTOMATIC);

  if(tuning)
  {
    tuning=false;
    changeAutoTune();
    tuning=true;
  }

  setupOneWire();

  Serial.begin(9600);
  WiFi.init(&Serial);    // initialize ESP module

   // Set ledPin mode
  pinMode(powerPin, OUTPUT);
  Serial.println("Setup compelete!");
}
float getTemp(DeviceAddress deviceAddress)
{

  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == DEVICE_DISCONNECTED_C){
    tempC = NAN;
  }
  return tempC;
}
void loopOneWire()
{


  if (sensors.isConversionComplete())
  {
    // just print out the current temperature
    input = getTemp(insideThermometer);
    word * reg = (word*)&input;
    sensors.requestTemperatures();
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
  double time = (now-powerTime);

  if (time < output/100.0*powerPeriod){
    digitalWrite(powerPin, 1);
  }else{
    digitalWrite(powerPin, 0);
  }

}
void loopWiFi();

void loopPID(){
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
//  else myPID.Compute();

  {
//     analogWrite(0,output);
      setOutputPower();
  }
}

byte wifiInited = 0;

void loop()
{
  if (wifiInited < 2){
    wifiInited = setupWiFi(wifiInited);
  }else{

    loopOneWire();
    loopPID();
    loopWiFi();
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
    output = 0.0;
  }
}

void AutoTuneHelper(boolean start)
{
  if(start)
    ATuneModeRemember = myPID.GetMode();
  else
    myPID.SetMode(ATuneModeRemember);
}


void SerialSend(Stream * port)
{
  (*port).println("------");
//  (*port).println(setpoint);
  (*port).println(input); 
  (*port).println(output);
  if(tuning){
    (*port).println("tuning mode");
  } else {
    (*port).println(myPID.GetKp(), 4);
    (*port).println(myPID.GetKi(), 4);
    (*port).println(myPID.GetKd(), 4);
  }
}
void SerialReceive(Stream * port)
{
  if((*port).available())
  {
   char b = (*port).read();
   (*port).flush();
   if((b=='1' && !tuning) || (b!='1' && tuning))changeAutoTune();
  }
}

WiFiEspClient * client = NULL;

void loopWiFi()
{
  if (client == NULL){
    client = server.available();  // listen for incoming clients
  }else{
    if (client->connected()) {              // loop while the client's connected
      SerialReceive(client);
      SerialSend(client);
    }else{
    // close the connection
      client->stop();
      free(client);
      client = NULL;
    }
  }
}
