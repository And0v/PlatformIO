#include <EtherCard.h>  // https://github.com/njh/EtherCard
#include "main.h"
#include "ModbusAO.h"
#include "ModbusUDP.h"
#include "microLAN.h"
#include "EEPROM.h"

void rwEEMEM(byte mode, word offset, byte * data, word len);
void printBuff(byte * buff, uint16_t len);

const TRegister ModbusAO::registers[] PROGMEM =
{
  IREG(100, &sensorsList[0]),
  IREG(101, &sensorsList[1]),
  IREG(102, &sensorsList[2]),
  IREG(103, &sensorsList[3]),
  IREG(104, &sensorsList[4]),
  IREG(105, &sensorsList[5]),
  IREG(106, &sensorsList[6]),
  IREG(107, &sensorsList[7]),
  //0
  HREG_EEPROM(200, (word*)0, &rwEEMEM),
  HREG_EEPROM(201, (word*)2, &rwEEMEM),
  HREG_EEPROM(202, (word*)4, &rwEEMEM),
  HREG_EEPROM(203, (word*)6, &rwEEMEM),
  //1
  HREG_EEPROM(204, (word*)8, &rwEEMEM),
  HREG_EEPROM(205, (word*)10, &rwEEMEM),
  HREG_EEPROM(206, (word*)12, &rwEEMEM),
  HREG_EEPROM(207, (word*)14, &rwEEMEM),
  //2
  HREG_EEPROM(208, (word*)16, &rwEEMEM),
  HREG_EEPROM(209, (word*)18, &rwEEMEM),
  HREG_EEPROM(210, (word*)20, &rwEEMEM),
  HREG_EEPROM(211, (word*)22, &rwEEMEM),
  //3
  HREG_EEPROM(212, (word*)24, &rwEEMEM),
  HREG_EEPROM(213, (word*)26, &rwEEMEM),
  HREG_EEPROM(214, (word*)28, &rwEEMEM),
  HREG_EEPROM(215, (word*)30, &rwEEMEM)

};

// Ethernet mac address - must be unique on your network
const byte myMac[] = { 0x70, 0x69, 0x69, 0x2D, 0x30, 0x31 };
const unsigned int NTP_LOCALPORT = 8333;             // Local UDP port to use
byte Ethernet::buffer[450];                          // Buffer must be 350 for DHCP to work

void udpReceivePacket(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *packetBuffer, uint16_t len);
void sendAnswerPacket();

uint8_t remote_ip[IP_LEN];
uint16_t remote_port;

ModbusUDP mb;

void setupU2MB() {
  // Change 'SS' to your Slave Select pin, if you arn't using the default pin
  if (ether.begin(sizeof Ethernet::buffer, myMac, SS) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);

  ether.udpServerListenOnPort(&udpReceivePacket, NTP_LOCALPORT);
  Serial.println("Started listening for response.");
  size_t regCount = sizeof(ModbusAO::registers)/sizeof(TRegister);

  Serial.print("Registers count ");
  Serial.println(regCount);

  mb.begin(regCount);
  mb.setSlaveId(11);
}

char textToSend[] = "test 123";
static uint32_t timer;

void loopU2MB() {
  // this must be called for ethercard functions to work.
  ether.packetLoop(ether.packetReceive());
  if (Events & EV_MODBUS_ANSWER){
    Events &= ~EV_MODBUS_ANSWER;
    sendAnswerPacket();
  }

  if (millis() > timer) {
    timer = millis() + 5000;
   //static void sendUdp (char *data,uint8_t len,uint16_t sport, uint8_t *dip, uint16_t dport);
    ether.sendUdp(textToSend, sizeof(textToSend), 4321, ether.hisip, 1234 );
  }
}

void udpReceivePacket(uint16_t dest_port, uint8_t src_ip[IP_LEN], uint16_t src_port, const char *packetBuffer, uint16_t len) {
  uint16_t answerLen = mb.task(packetBuffer, len);
  if (answerLen != 0){
     Events |= EV_MODBUS_ANSWER;
  }
}
#define PACKET_SIZE 10
// send an NTP request to the time server at the given address
void sendAnswerPacket() {
  byte * buff = mb.getAnswerBuff();
  uint16_t len = mb.getAnswerLen();
  ether.makeUdpReply(buff, len, NTP_LOCALPORT);

}

void rwEEMEM(byte mode, word offset, byte * data, word len){
    if ( mode == 0 ) {
      for(byte i = 0; i < len;++i){
        data[i] = EEPROM.read(offset+i);
      }
    }else{
      for(byte i = 0; i < len;++i){
        EEPROM.write(offset+i, data[i]);
      }
    }
}
