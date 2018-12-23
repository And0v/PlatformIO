
#include "Arduino.h"
#include <PID_v1.h>
#include <PID_AutoTune_v0.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#include "ModbusASCII.h"

#define ONE_WIRE_BUS 8
OneWire oneWire(ONE_WIRE_BUS);

double input=0, output=0, setpoint=5;
double kp=10,ki=0.1,kd=0.01;

const double kpmodel=1.5, taup=100;
const double outputStart=5;

unsigned long  modelTime;

PID myPID(&input, &output, &setpoint,kp,ki,kd, DIRECT);


/////////////////////////////////////////////////////////

DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

/////////////////////
const int powerPin = 13;
unsigned long powerPeriod = 20000;
unsigned long powerTime = 0;
/////////////////////////////////////////////////////

const int SENSOR_IREG = 100; //+1
const int OUTPUT_HREG = 200; //+1

ModbusASCII mb;

void setupModbus(){
  mb.setSlaveId(10);
  mb.addIreg(SENSOR_IREG);
  mb.addIreg(SENSOR_IREG+1);
  mb.addHreg(OUTPUT_HREG);
  mb.addHreg(OUTPUT_HREG+1);
}

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
  myPID.SetOutputLimits(0, 100);

  setupOneWire();

  Serial.begin(9600);
  WiFi.init(&Serial);    // initialize ESP module

  setupModbus();
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
    mb.Ireg(SENSOR_IREG, reg[0]);
    mb.Ireg(SENSOR_IREG+1, reg[1]);
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
  myPID.Compute();
  setOutputPower();
  word * reg = (word*)&output;
  mb.Hreg(OUTPUT_HREG, reg[0]);
  mb.Hreg(OUTPUT_HREG+1, reg[1]);

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

 WiFiEspClient * client = NULL;


void  loopWiFi()
{
  if (client == NULL){
    client = server.available();  // listen for incoming clients
  }else{
    mb.config(client);
    if (client->connected()) {              // loop while the client's connected
      mb.task();
    }else{
    // close the connection
      client->stop();
      free(client);
      client = NULL;
    }
  }
}
