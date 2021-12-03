/*
    ModbusRTU.cpp - Source for Modbus Serial Library
    Copyright (C) 2014 André Sarmento Barbosa
*/
#include "aoModbusRTU.h"

ModbusRTU::ModbusRTU() { this->_port = NULL; }

bool ModbusRTU::setSlaveId(byte slaveId) {
  _slaveId = slaveId;
  return true;
}

byte ModbusRTU::getSlaveId() { return _slaveId; }

bool ModbusRTU::config(Stream *port) {
  this->_port = port;
  return true;
}

bool ModbusRTU::receive(byte *frame) {

  // first byte of frame = address
  byte address = frame[0];
  // Last two bytes = crc
  u_int crc = ((frame[_len - 2] << 8) | frame[_len - 1]);
  // Slave Check
  if (address != 0xFF && address != this->getSlaveId()) {
    return false;
  }

  // CRC Check
  if (crc != this->calcCrc(_frame[0], _frame + 1, _len - 3)) {
    return false;
  }

  // PDU starts after first byte
  // framesize PDU = framesize - address(1) - crc(2)
  this->receivePDU(frame + 1);
  // No reply to Broadcasts
  if (address == 0xFF)
    _reply = MB_REPLY_OFF;
  return true;
}

void ModbusRTU::send(byte *frame) {
  byte i;

  for (i = 0; i < _len; i++) {
    (*_port).write(frame[i]);
  }
  (*_port).flush();
}

void ModbusRTU::sendPDU(byte *pduframe) {
  buff[0] = _slaveId;
  memcpy((char *)&buff[1], (const char *)pduframe, _len);
  // Send CRC
  word crc = calcCrc(_slaveId, pduframe, _len);
  buff[_len + 1] = (crc >> 8);
  buff[_len + 2] = (crc & 0xFF);
  (*_port).write(buff, _len + 3);
  (*_port).flush();
}

void ModbusRTU::task() {
  _len = 0;
  unsigned long mkSec = micros() + (_timeout << 1);
  int available;
  while (((available = (*_port).available()) > _len) ||
         ((micros() - mkSec) < _timeout)) {
    if (_len != available) {
      mkSec = micros();
      _len = available;
    }
  }

  if (_len < 8) {
    return;
  }

  byte i;
  _frame = (byte *)malloc(_len);

  for (i = 0; i < _len; i++) {
    _frame[i] = (*_port).read();
  }

  if (this->receive(_frame)) {
    if (_reply == MB_REPLY_NORMAL) {
      this->sendPDU(_frame);
    } else {
      if (_reply == MB_REPLY_ECHO) {
      }
      this->send(_frame);
    }
  }
  free(_frame);
  _len = 0;
}

word ModbusRTU::calcCrc(byte address, byte *pduFrame, byte pduLen) {
  byte CRCHi = 0xFF, CRCLo = 0x0FF, Index;

  Index = CRCHi ^ address;
  CRCHi = CRCLo ^ _auchCRCHi[Index];
  CRCLo = _auchCRCLo[Index];

  while (pduLen--) {
    Index = CRCHi ^ *pduFrame++;
    CRCHi = CRCLo ^ _auchCRCHi[Index];
    CRCLo = _auchCRCLo[Index];
  }

  return (CRCHi << 8) | CRCLo;
}
