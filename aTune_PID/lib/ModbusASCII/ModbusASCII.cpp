/*
    ModbusASCII.cpp - Source for Modbus Serial Library
    Copyright (C) 2014 André Sarmento Barbosa
*/
#include "ModbusASCII.h"

ModbusASCII::ModbusASCII() {

}

bool ModbusASCII::receive(byte* frame) {
    if (frame[0] != 0x3A) {
		    return false;
	  }
    //first byte of frame = address
    byte address = ascii2byte(frame+1);
    //Last one bytes = crc
    byte lrc = ascii2byte(&frame[_len - 4]);

    //Slave Check
    if ((address != 0xFF)&&(address != getSlaveId())) {
	 	  return false;
	  }


    frame[0] = address;
    address = 1;
    byte off = 3;
    while (off < _len-4){
       frame[address] = ascii2byte(frame+off);
       off+=2;
       ++address;
    }

    //CRC Check
    if (lrc != this->calcLRC(_frame, address)) {
  		return false;
    }
    //PDU starts after first byte
    //framesize PDU = framesize - address(1) - crc(2)
    this->receivePDU(frame+1);
    //No reply to Broadcasts
    if (address == 0xFF) _reply = MB_REPLY_OFF;
    return true;
}

void ModbusASCII::send(byte* frame) {
    byte i;

    for (i = 0 ; i < _len ; i++) {
        (*_port).write(frame[i]);
    }

    (*_port).flush();
}

void ModbusASCII::sendPDU(byte* pduframe) {

    _buff[0] = _slaveId;
    strncpy((char *)&_buff[1], (const char *)pduframe, _len);
    //Send CRC
    word crc = calcCrc(_slaveId, _frame, _len);
    _buff[_len+1] =(crc >> 8);
    _buff[_len+2] =(crc & 0xFF);
    (*_port).write(_buff, _len+3);
    (*_port).flush();
}

byte ModbusASCII::calcLRC(byte *auchMsg, unsigned short usDataLen)
/* message to calculate  */
/* LRC upon quantity of  */
										/* bytes in message      */
{
	byte uchLRC = 0 ;	/* LRC char initialized   */
	while (usDataLen--)		/* pass through message  */{
    byte b= *auchMsg++;
    Serial.write(b);
		uchLRC += b;//*auchMsg++ ;	/* buffer add buffer byte*/
										/* without carry         */
                  }
	return ((unsigned char)(-uchLRC));
										/* return twos complemen */
}
byte ModbusASCII::ascii2byte(byte *auchMsg){
    byte i = 0;
    byte rst = 0;
    do{
        rst <<= 4;
        if ((auchMsg[i] <= 0x39)&&(auchMsg[i]>=0x30)){
            rst += auchMsg[i] & 0x0F;
        }else if ((auchMsg[i] <= 'F')&&(auchMsg[i] >= 'A')){
            rst += (auchMsg[i]-'A'+0x0A);
        }
    }while (++i < 2);
    return rst;
}
void  ModbusASCII::byte2ascii(byte data, byte *auchMsg){
    byte i = 0;
    byte shift = 0;

    do{
        byte d = ((data >> shift) & 0x0F);

        if (d <= 9){
            auchMsg[i] = ('0'+d);
        }else {
            auchMsg[i] = ('A'+d);
        }
        shift += 4;
    }while (++i < 2);
}
