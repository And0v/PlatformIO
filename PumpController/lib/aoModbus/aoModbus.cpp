/*
    Modbus.cpp - Source for Modbus Base Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
*/
#include <Arduino.h>

#include "aoModbus.h"

ModbusAO::ModbusAO() {
  _regs_size = 0;
  _swapWords = false;
}

void ModbusAO::begin(byte size, unsigned long timeout) {
  _regs_size = size;
  _timeout = timeout;
}

void ModbusAO::swapMode(bool swap) { _swapWords = swap; }

void ModbusAO::readRegistersInfo(byte *buff, const TRegister *addr) {
  uint16_t size = sizeof(*addr);
  for (uint16_t i = 0; i < size; ++i) {
    buff[i] = pgm_read_byte(((byte *)addr) + i);
  }
}

TRegister *ModbusAO::searchRegister(word address) {
  if (_regs_size == 0)
    return (0);
  // scan through the linked list until the end of the list or the register is
  // found. return the pointer.
  for (byte i = 0; i < _regs_size; ++i) {
    readRegistersInfo((byte *)&_curReg, &registers[i]);
    word addr2 = _curReg.address;
    if ((_curReg.type == REG_RAM2) || (_curReg.type == REG_EEMEM2)) {
      addr2++;
    }
    if ((_curReg.address == address) || (addr2 == address)) {
      return (&_curReg);
    }
  }
  memset(&_curReg, 0, sizeof(_curReg));
  return (0);
}

bool ModbusAO::Reg(word address, word value) {
  TRegister *reg;
  // search for the register address
  reg = this->searchRegister(address);
  // if found then assign the register value to the new value.
  if (reg == NULL) {
    return false;
  }
  if (reg->type == REG_RAM) {
    *(reg->pValue) = value;
  } else if (reg->type == REG_EEMEM) {
    return reg->callback(1, (word)reg->pValue, (byte *)&value, sizeof(value));
  } else {
    return false;
  }
  return true;
}

word ModbusAO::Reg(word address) {
  TRegister *reg;
  reg = this->searchRegister(address);
  if (reg == NULL) {
    return (0);
  }
  if (reg->type == REG_RAM) {
    return *(reg->pValue);
  } else if (reg->type == REG_EEMEM) {
    word data;
    reg->callback(0, (word)reg->pValue, (byte *)&data, sizeof(data));
    return data;
  }
  return (0);
}
byte ModbusAO::rReg(word address, byte *buff) {
  TRegister *reg;
  reg = this->searchRegister(address);
  if (reg == NULL) {
    return (0);
  }
  byte rst = 0;
  long value = 0;
  switch (reg->type) {
  case REG_RAM:
    if (reg->callback == NULL) {
      reverseCopy(buff, (reg->pValue), 2);
    } else {
      reg->callback(0, (word)reg->pValue, (byte *)&value, 2);
      reverseCopy(buff, &value, 2);
    }
    rst = 1;
    break;
  case REG_RAM2:
    if (reg->callback == NULL) {
      reverseCopy(buff, (reg->pValue), 4);
    } else {
      reg->callback(0, (word)reg->pValue, (byte *)&value, 4);
      reverseCopy(buff, &value, 4);
    }
    if (_swapWords) {
      swapWords(buff);
    }
    rst = 2;
    break;

  case REG_EEMEM:
    if (reg->callback == NULL) {
      rst = 0;
    } else {
      reg->callback(0, (word)reg->pValue, (byte *)&value, 2);
      reverseCopy(buff, &value, 2);
      rst = 1;
    }
    break;
  case REG_EEMEM2:
    if (reg->callback == NULL) {
      rst = 0;
    } else {
      reg->callback(0, (word)reg->pValue, (byte *)&value, 4);
      reverseCopy(buff, &value, 4);
      if (_swapWords) {
        swapWords(buff);
      }
      rst = 2;
    }
    break;
  }
  return rst;
}
byte ModbusAO::wReg(word address, byte *buff) {
  TRegister *reg;
  reg = this->searchRegister(address);
  if (reg == NULL) {
    return (0);
  }
  byte rst = 0;
  long value = 0;
  switch (reg->type) {
  case REG_RAM:
    if (reg->callback == NULL) {
      reverseCopy(reg->pValue, buff, 2);
    } else {
      reverseCopy(&value, buff, 2);
      reg->callback(1, (word)reg->pValue, (byte *)&value, 2);
    }
    rst = 1;
    break;
  case REG_RAM2:
    swapWords(buff);
    if (reg->callback == NULL) {
      reverseCopy(reg->pValue, buff, 4);
    } else {
      reverseCopy(&value, buff, 4);
      reg->callback(1, (word)reg->pValue, (byte *)&value, 4);
    }
    rst = 2;
    break;

  case REG_EEMEM:
    if (reg->callback == NULL) {
      rst = 0;
    } else {
      reverseCopy(&value, buff, 2);
      reg->callback(1, (word)reg->pValue, (byte *)&value, 2);
      rst = 1;
    }
    break;
  case REG_EEMEM2:
    swapWords(buff);
    if (reg->callback == NULL) {
      rst = 0;
    } else {
      reverseCopy(&value, buff, 4);
      reg->callback(1, (word)reg->pValue, (byte *)&value, 4);
      rst = 2;
    }
    break;
  }
  return rst;
}

bool ModbusAO::Hreg(word offset, word value) {
  return Reg(offset + OFFSET_HREG, value);
}

word ModbusAO::Hreg(word offset) { return Reg(offset + OFFSET_HREG); }

byte ModbusAO::rHreg(word offset, byte *buff) {
  return rReg(offset + OFFSET_HREG, buff);
}
byte ModbusAO::wHreg(word offset, byte *buff) {
  return wReg(offset + OFFSET_HREG, buff);
}

#ifndef USE_HOLDING_REGISTERS_ONLY
bool ModbusAO::Coil(word offset, bool value) {
  return Reg(offset + 1, value ? 0xFF00 : 0x0000);
}

bool ModbusAO::Ists(word offset, bool value) {
  return Reg(offset + OFFSET_ISTS, value ? 0xFF00 : 0x0000);
}

bool ModbusAO::Ireg(word offset, word value) {
  return Reg(offset + OFFSET_IREG, value);
}

bool ModbusAO::Coil(word offset) {
  if (Reg(offset + 1) == 0xFF00) {
    return true;
  } else
    return false;
}

bool ModbusAO::Ists(word offset) {
  if (Reg(offset + OFFSET_ISTS) == 0xFF00) {
    return true;
  } else
    return false;
}

// word ModbusAO::Ireg(word offset) {
//     return Reg(offset + OFFSET_IREG);
// }

byte ModbusAO::rIreg(word offset, byte *buff) {
  return rReg(offset + OFFSET_IREG, buff);
}
byte ModbusAO::wIreg(word offset, byte *buff) {
  return wReg(offset + OFFSET_IREG, buff);
}
#endif

void ModbusAO::receivePDU(byte *frame) {

  byte fcode = frame[0];
  word field1 = (word)frame[1] << 8 | (word)frame[2];
  word field2 = (word)frame[3] << 8 | (word)frame[4];

  switch (fcode) {

  case MB_FC_WRITE_REG:
    // field1 = reg, field2 = value
    this->writeSingleRegister(field1, field2);
    break;

  case MB_FC_READ_REGS:
    // field1 = startreg, field2 = numregs
    this->readRegisters(field1, field2);
    break;

  case MB_FC_WRITE_REGS:
    // field1 = startreg, field2 = status
    this->writeMultipleRegisters(frame, field1, field2, frame[5]);
    break;

#ifndef USE_HOLDING_REGISTERS_ONLY
  case MB_FC_READ_COILS:
    // field1 = startreg, field2 = numregs
    this->readCoils(field1, field2);
    break;

  case MB_FC_READ_INPUT_STAT:
    // field1 = startreg, field2 = numregs
    this->readInputStatus(field1, field2);
    break;

  case MB_FC_READ_INPUT_REGS:
    // field1 = startreg, field2 = numregs
    this->readInputRegisters(field1, field2);
    break;

  case MB_FC_WRITE_COIL:
    // field1 = reg, field2 = status
    this->writeSingleCoil(field1, field2);
    break;

  case MB_FC_WRITE_COILS:
    // field1 = startreg, field2 = numoutputs
    this->writeMultipleCoils(frame, field1, field2, frame[5]);
    break;

#endif
  default:
    this->exceptionResponse(fcode, MB_EX_ILLEGAL_FUNCTION);
  }
}

void ModbusAO::exceptionResponse(byte fcode, byte excode) {
  // Clean frame buffer
  free(_frame);
  _len = 2;
  _frame = (byte *)malloc(_len);
  _frame[0] = fcode + 0x80;
  _frame[1] = excode;

  _reply = MB_REPLY_NORMAL;
}

void ModbusAO::readRegisters(word startreg, word numregs) {
  // Check value (numregs)
  if (numregs < 0x0001 || numregs > 0x007D) {
    this->exceptionResponse(MB_FC_READ_REGS, MB_EX_ILLEGAL_VALUE);
    return;
  }

  // Check Address
  //*** See comments on readCoils method.
  if (!this->searchRegister(startreg + OFFSET_HREG)) {
    this->exceptionResponse(MB_FC_READ_REGS, MB_EX_ILLEGAL_ADDRESS);
    return;
  }

  // Clean frame buffer
  free(_frame);
  _len = 0;

  // calculate the query reply message length
  // for each register queried add 2 bytes
  _len = 2 + numregs * 2;

  _frame = (byte *)malloc(_len);
  if (!_frame) {
    this->exceptionResponse(MB_FC_READ_REGS, MB_EX_SLAVE_FAILURE);
    return;
  }

  _frame[0] = MB_FC_READ_REGS;
  _frame[1] = _len - 2; // byte count

  word i = 0;
  while (numregs) {
    // retrieve the value from the register bank for the current register
    byte regs = this->rHreg(startreg + i, &_frame[2 + i * 2]);
    if (regs == 0) {
      this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_SLAVE_FAILURE);
      return;
    }
    i += regs;
    if (numregs < regs) {
      this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_SLAVE_FAILURE);
      return;
    }
    numregs -= regs;
    // //retrieve the value from the register bank for the current register
    // val = this->Hreg(startreg + i);
    // //write the high byte of the register value
    // _frame[2 + i * 2] = val >> 8;
    // //write the low byte of the register value
    // _frame[3 + i * 2] = val & 0xFF;
    // i++;
  }

  _reply = MB_REPLY_NORMAL;
}

void ModbusAO::writeSingleRegister(word reg, word value) {
  // No necessary verify illegal value (EX_ILLEGAL_VALUE) - because using word
  // (0x0000 - 0x0FFFF) Check Address and execute (reg exists?)
  if (this->wHreg(reg, (byte *)&value) != 1) {
    this->exceptionResponse(MB_FC_WRITE_REG, MB_EX_ILLEGAL_ADDRESS);
    return;
  }

  // Check for failure
  if (this->Hreg(reg) != value) {
    this->exceptionResponse(MB_FC_WRITE_REG, MB_EX_SLAVE_FAILURE);
    return;
  }

  _reply = MB_REPLY_ECHO;
}

void ModbusAO::writeMultipleRegisters(byte *frame, word startreg,
                                      word numoutputs, byte bytecount) {
  // Check value
  if (numoutputs < 0x0001 || numoutputs > 0x007B ||
      bytecount != 2 * numoutputs) {
    this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_ILLEGAL_VALUE);
    return;
  }

  // Check Address (startreg...startreg + numregs)
  for (byte k = 0; k < numoutputs; k++) {
    if (!this->searchRegister(startreg + OFFSET_HREG + k)) {
      this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_ILLEGAL_ADDRESS);
      return;
    }
  }

  // Clean frame buffer
  free(_frame);
  _len = 5;
  _frame = (byte *)malloc(_len);
  if (!_frame) {
    this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_SLAVE_FAILURE);
    return;
  }

  _frame[0] = MB_FC_WRITE_REGS;
  _frame[1] = startreg >> 8;
  _frame[2] = startreg & 0x00FF;
  _frame[3] = numoutputs >> 8;
  _frame[4] = numoutputs & 0x00FF;

  // word val;
  word i = 0;
  while (numoutputs>0) {
    byte regs = wHreg(startreg + i, &frame[6 + i * 2]);
    if (regs == 0) {
      this->exceptionResponse(MB_FC_WRITE_REGS, MB_EX_SLAVE_FAILURE);
      return;
    }
    numoutputs-=regs;
    i += regs;
    // val = (word)frame[6 + i * 2] << 8 | (word)frame[7 + i * 2];
    // this->Hreg(startreg + i, val);
    // i++;
  }

  _reply = MB_REPLY_NORMAL;
}

void *ModbusAO::reverseCopy(void *dst, void *src, byte size) {
  if ((src == NULL) || (dst == NULL) || (size == 0)) {
    return NULL;
  }
  for (byte i = 0; i < size; ++i) {
    ((byte *)dst)[i] = ((byte *)src)[size - 1 - i];
  }
  return dst;
}
void ModbusAO::swapWords(void *data) {
  if (data == NULL) {
    return;
  }
  word swap = ((word *)data)[1];
  ((word *)data)[1] = ((word *)data)[0];
  ((word *)data)[0] = swap;
}

#ifndef USE_HOLDING_REGISTERS_ONLY
void ModbusAO::readCoils(word startreg, word numregs) {
  // Check value (numregs)
  if (numregs < 0x0001 || numregs > 0x07D0) {
    this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_VALUE);
    return;
  }

  // Check Address
  // Check only startreg. Is this correct?
  // When I check all registers in range I got errors in ScadaBR
  // I think that ScadaBR request more than one in the single request
  // when you have more then one datapoint configured from same type.
  if (!this->searchRegister(startreg + 1)) {
    this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);
    return;
  }

  // Clean frame buffer
  free(_frame);
  _len = 0;

  // Determine the message length = function type, byte count and
  // for each group of 8 registers the message length increases by 1
  _len = 2 + numregs / 8;
  if (numregs % 8)
    _len++; // Add 1 to the message length for the partial byte.

  _frame = (byte *)malloc(_len);
  if (!_frame) {
    this->exceptionResponse(MB_FC_READ_COILS, MB_EX_SLAVE_FAILURE);
    return;
  }

  _frame[0] = MB_FC_READ_COILS;
  _frame[1] = _len - 2; // byte count (_len - function code and byte count)

  byte bitn = 0;
  word totregs = numregs;
  word i;
  while (numregs--) {
    i = (totregs - numregs) / 8;
    if (this->Coil(startreg))
      bitSet(_frame[2 + i], bitn);
    else
      bitClear(_frame[2 + i], bitn);
    // increment the bit index
    bitn++;
    if (bitn == 8)
      bitn = 0;
    // increment the register
    startreg++;
  }

  _reply = MB_REPLY_NORMAL;
}

void ModbusAO::readInputStatus(word startreg, word numregs) {
  // Check value (numregs)
  if (numregs < 0x0001 || numregs > 0x07D0) {
    this->exceptionResponse(MB_FC_READ_INPUT_STAT, MB_EX_ILLEGAL_VALUE);
    return;
  }

  // Check Address
  //*** See comments on readCoils method.
  if (!this->searchRegister(startreg + OFFSET_COIL)) {
    this->exceptionResponse(MB_FC_READ_COILS, MB_EX_ILLEGAL_ADDRESS);
    return;
  }

  // Clean frame buffer
  free(_frame);
  _len = 0;

  // Determine the message length = function type, byte count and
  // for each group of 8 registers the message length increases by 1
  _len = 2 + numregs / 8;
  if (numregs % 8)
    _len++; // Add 1 to the message length for the partial byte.

  _frame = (byte *)malloc(_len);
  if (!_frame) {
    this->exceptionResponse(MB_FC_READ_INPUT_STAT, MB_EX_SLAVE_FAILURE);
    return;
  }

  _frame[0] = MB_FC_READ_INPUT_STAT;
  _frame[1] = _len - 2;

  byte bitn = 0;
  word totregs = numregs;
  word i;
  while (numregs--) {
    i = (totregs - numregs) / 8;
    if (this->Ists(startreg))
      bitSet(_frame[2 + i], bitn);
    else
      bitClear(_frame[2 + i], bitn);
    // increment the bit index
    bitn++;
    if (bitn == 8)
      bitn = 0;
    // increment the register
    startreg++;
  }

  _reply = MB_REPLY_NORMAL;
}

void ModbusAO::readInputRegisters(word startreg, word numregs) {
  // Check value (numregs)
  if (numregs < 0x0001 || numregs > 0x007D) {
    this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_ILLEGAL_VALUE);
    return;
  }

  // Check Address
  //*** See comments on readCoils method.
  if (!this->searchRegister(startreg + OFFSET_IREG)) {
    this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_ILLEGAL_ADDRESS);
    return;
  }

  // Clean frame buffer
  free(_frame);
  _len = 0;

  // calculate the query reply message length
  // for each register queried add 2 bytes
  _len = 2 + numregs * 2;

  _frame = (byte *)malloc(_len);
  if (!_frame) {
    this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_SLAVE_FAILURE);
    return;
  }

  _frame[0] = MB_FC_READ_INPUT_REGS;
  _frame[1] = _len - 2;

  word i = 0;
  while (numregs) {
    // retrieve the value from the register bank for the current register
    byte regs = this->rIreg(startreg + i, &_frame[2 + i * 2]);
    if (regs == 0) {
      this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_SLAVE_FAILURE);
      return;
    }
    i += regs;
    if (numregs < regs) {
      this->exceptionResponse(MB_FC_READ_INPUT_REGS, MB_EX_SLAVE_FAILURE);
      return;
    }
    numregs -= regs;
  }

  _reply = MB_REPLY_NORMAL;
}

void ModbusAO::writeSingleCoil(word reg, word status) {
  // Check value (status)
  if (status != 0xFF00 && status != 0x0000) {
    this->exceptionResponse(MB_FC_WRITE_COIL, MB_EX_ILLEGAL_VALUE);
    return;
  }

  // Check Address and execute (reg exists?)
  if (!this->Coil(reg, (bool)status)) {
    this->exceptionResponse(MB_FC_WRITE_COIL, MB_EX_ILLEGAL_ADDRESS);
    return;
  }

  // Check for failure
  if (this->Coil(reg) != (bool)status) {
    this->exceptionResponse(MB_FC_WRITE_COIL, MB_EX_SLAVE_FAILURE);
    return;
  }

  _reply = MB_REPLY_ECHO;
}

void ModbusAO::writeMultipleCoils(byte *frame, word startreg, word numoutputs,
                                  byte bytecount) {
  // Check value
  word bytecount_calc = numoutputs / 8;
  if (numoutputs % 8)
    bytecount_calc++;
  if (numoutputs < 0x0001 || numoutputs > 0x07B0 ||
      bytecount != bytecount_calc) {
    this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_VALUE);
    return;
  }

  // Check Address (startreg...startreg + numregs)
  for (byte k = 0; k < numoutputs; k++) {
    if (!this->searchRegister(startreg + 1 + k)) {
      this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_ILLEGAL_ADDRESS);
      return;
    }
  }

  // Clean frame buffer
  free(_frame);
  _len = 5;
  _frame = (byte *)malloc(_len);
  if (!_frame) {
    this->exceptionResponse(MB_FC_WRITE_COILS, MB_EX_SLAVE_FAILURE);
    return;
  }

  _frame[0] = MB_FC_WRITE_COILS;
  _frame[1] = startreg >> 8;
  _frame[2] = startreg & 0x00FF;
  _frame[3] = numoutputs >> 8;
  _frame[4] = numoutputs & 0x00FF;

  byte bitn = 0;
  word totoutputs = numoutputs;
  word i;
  while (numoutputs--) {
    i = (totoutputs - numoutputs) / 8;
    this->Coil(startreg, bitRead(frame[6 + i], bitn));
    // increment the bit index
    bitn++;
    if (bitn == 8)
      bitn = 0;
    // increment the register
    startreg++;
  }
  _reply = MB_REPLY_NORMAL;
}
#endif
