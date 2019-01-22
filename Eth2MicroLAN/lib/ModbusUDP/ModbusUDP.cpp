/*
    ModbusRTU.cpp - Source for Modbus Serial Library
    Copyright (C) 2014 André Sarmento Barbosa
*/
#include "ModbusUDP.h"
#include "main.h"

ModbusUDP::ModbusUDP() {

}

bool ModbusUDP::setSlaveId(byte slaveId){
    _slaveId = slaveId;
    return true;
}

byte ModbusUDP::getSlaveId() {
    return _slaveId;
}

bool ModbusUDP::receive(byte* frame) {

//first byte of frame = address
    byte address = frame[0];
    //Last two bytes = crc
    u_int crc = ((frame[_len - 2] << 8) | frame[_len - 1]);

    //Slave Check
    if (address != 0xFF && address != this->getSlaveId()) {
		return false;
	}

    //CRC Check
    u_int crcCalc = this->calcCrc(_frame[0], _frame+1, _len-3);

    if (crc != crcCalc) {
		  return false;
    }

    //PDU starts after first byte
    //framesize PDU = framesize - address(1) - crc(2)
    this->receivePDU(frame+1);
    //No reply to Broadcasts
    if (address == 0xFF) _reply = MB_REPLY_OFF;

    return true;
}

void ModbusUDP::send(byte* frame) {
    memcpy((char *)&buff[0], (const char *)frame, _len);
}

void ModbusUDP::sendPDU(byte* pduframe) {
    buff[0] = _slaveId;
    memcpy((char *)&buff[1], (const char *)pduframe, _len);
    //Send CRC
    word crc = calcCrc(_slaveId, pduframe, _len);
    buff[_len+1] =(crc >> 8);
    buff[_len+2] =(crc & 0xFF);
}
uint16_t ModbusUDP::getAnswerLen() {
  return _len;
}
byte * ModbusUDP::getAnswerBuff() {
  return buff;
}

uint16_t ModbusUDP::task(byte * frame, uint16_t len) {
    _len = len;
    if (_len == 0) return _len;

    _frame = malloc(len);
    memcpy((void *)_frame, (const void*)frame, len);

    if (this->receive(_frame)) {
        if (_reply == MB_REPLY_NORMAL){
            this->sendPDU(_frame);
            _len += 3;
        }else{
            if (_reply == MB_REPLY_ECHO){
                this->send(_frame);
                _len += 3;
            }else{
              _len = 0;
            }
        }
    }else{
      _len = 0;
    }
    free(_frame);

    return _len;
}

word ModbusUDP::calcCrc(byte address, byte* pduFrame, byte pduLen) {
	byte CRCHi = 0xFF, CRCLo = 0x0FF, Index;

    Index = CRCHi ^ address;
    CRCHi = CRCLo ^ pgm_read_byte(&_auchCRCHi[Index]);
    CRCLo = pgm_read_byte(&_auchCRCLo[Index]);

    while (pduLen--) {
        Index = CRCHi ^ *pduFrame++;
        CRCHi = CRCLo ^ pgm_read_byte(&_auchCRCHi[Index]);
        CRCLo = pgm_read_byte(&_auchCRCLo[Index]);
    }

    return (CRCHi << 8) | CRCLo;
}
