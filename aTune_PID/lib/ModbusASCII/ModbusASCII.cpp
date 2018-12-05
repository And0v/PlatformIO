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
    byte lrc = (frame[_len - 3]);

    //Slave Check
    if ((address != 0xFF)&&(address != getSlaveId())) {
		return false;
	}

    //CRC Check
    if (lrc != this->calcLRC(_frame+1, _len-4)) {
		return false;
    }
    frame[0] = address;
    address = 1;
    while (address < _len -3){
       frame[address] = ascii2byte(frame+address);
       address+=2;
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
    //Send slaveId
    // (*_port).write(_slaveId);

    //Send PDU
    strncpy((char *)&_buff[1], (const char *)pduframe, _len);
    // byte i;
    // for (i = 0 ; i < _len ; i++) {
    //     (*_port).write(pduframe[i]);
    // }

    //Send CRC
    word crc = calcCrc(_slaveId, _frame, _len);
    // (*_port).write(crc >> 8);
    // (*_port).write(crc & 0xFF);
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
	while (usDataLen--)		/* pass through message  */ 
		uchLRC += *auchMsg++ ;	/* buffer add buffer byte*/ 
										/* without carry         */
	return ((unsigned char)(-uchLRC));
										/* return twos complemen */
}
byte ModbusASCII::ascii2byte(byte *auchMsg){
    byte i = 1;
    byte rst = 0;
    do{
        rst <<= 4;
        if ((auchMsg[i] <= 0x39)&&(auchMsg[i]>=0x30)){
            rst += auchMsg[i] & 0x0F;
        }else if ((auchMsg[i] <= 'F')&&(auchMsg[i] >= 'A')){
            rst += (auchMsg[i]-'A'+0x0A);
        }
    }while (i-- != 255);
    return rst;
}

