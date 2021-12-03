/*
    Modbus.h - Header for Modbus Base Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
*/
#ifndef MODBUS_AO_H
#define MODBUS_AO_H
#define MODBUS_AO_VERSION 20211102

#define MAX_REGS 32
#define MAX_FRAME 128
//#define USE_HOLDING_REGISTERS_ONLY

#define OFFSET_COIL 00001
#define OFFSET_ISTS 10001
#define OFFSET_IREG 30001
#define OFFSET_HREG 40001

#define REG_RAM 0
#define REG_EEMEM 1
#define REG_RAM2 2
#define REG_EEMEM2 3

typedef bool (*ModbusCallback)(byte mode, word offset, byte *data, word len);

#define COIL(addr, ptr) OFFSET_COIL + addr, REG_RAM, ptr, (ModbusCallback)0
#define ISTS(addr, ptr) OFFSET_ISTS + addr, REG_RAM, ptr, (ModbusCallback)0
#define IREG(addr, ptr) OFFSET_IREG + addr, REG_RAM, ptr, (ModbusCallback)0
#define IREG2(addr, ptr) OFFSET_IREG + addr, REG_RAM2, ptr, (ModbusCallback)0
#define IREG2_f(addr, ptr, ptr_func) OFFSET_IREG + addr, REG_RAM2, ptr, ptr_func

#define HREG(addr, ptr) OFFSET_HREG + addr, REG_RAM, ptr, (ModbusCallback)0
#define HREG2(addr, ptr) OFFSET_HREG + addr, REG_RAM2, ptr, (ModbusCallback)0
#define HREG_f(addr, ptr, ptr_func) OFFSET_HREG + addr, REG_RAM, ptr, ptr_func
#define HREG2_f(addr, ptr, ptr_func) OFFSET_HREG + addr, REG_RAM2, ptr, ptr_func

#define HREG_EEPROM(addr, ptr, ptr_func)                                       \
  OFFSET_HREG + addr, REG_EEMEM, ptr, ptr_func
#define HREG2_EEPROM(addr, ptr, ptr_func)                                      \
  OFFSET_HREG + addr, REG_EEMEM2, ptr, ptr_func

typedef unsigned int u_int;

// Function Codes
enum {
  MB_FC_READ_COILS = 0x01,      // Read Coils (Output) Status 0xxxx
  MB_FC_READ_INPUT_STAT = 0x02, // Read Input Status (Discrete Inputs) 1xxxx
  MB_FC_READ_REGS = 0x03,       // Read Holding Registers 4xxxx
  MB_FC_READ_INPUT_REGS = 0x04, // Read Input Registers 3xxxx
  MB_FC_WRITE_COIL = 0x05,      // Write Single Coil (Output) 0xxxx
  MB_FC_WRITE_REG = 0x06,       // Preset Single Register 4xxxx
  MB_FC_WRITE_COILS = 0x0F,     // Write Multiple Coils (Outputs) 0xxxx
  MB_FC_WRITE_REGS = 0x10,      // Write block of contiguous registers 4xxxx
};

// Exception Codes
enum {
  MB_EX_ILLEGAL_FUNCTION = 0x01, // Function Code not Supported
  MB_EX_ILLEGAL_ADDRESS = 0x02,  // Output Address not exists
  MB_EX_ILLEGAL_VALUE = 0x03,    // Output Value not in Range
  MB_EX_SLAVE_FAILURE = 0x04,    // Slave Deive Fails to process request
};

// Reply Types
enum {
  MB_REPLY_OFF = 0x01,
  MB_REPLY_ECHO = 0x02,
  MB_REPLY_NORMAL = 0x03,
};

typedef struct TRegister {
  word address;
  word type;
  word *pValue;
  ModbusCallback callback;
} TRegister;

class ModbusAO {
private:
  uint8_t _regs_size;
  bool  _swapWords;
  TRegister _curReg;
  

  void readRegistersInfo(byte *buff, const TRegister *addr);

  void readRegisters(word startreg, word numregs);
  void writeSingleRegister(word reg, word value);
  void writeMultipleRegisters(byte *frame, word startreg, word numoutputs,
                              byte bytecount);
  void exceptionResponse(byte fcode, byte excode);
#ifndef USE_HOLDING_REGISTERS_ONLY
  void readCoils(word startreg, word numregs);
  void readInputStatus(word startreg, word numregs);
  void readInputRegisters(word startreg, word numregs);
  void writeSingleCoil(word reg, word status);
  void writeMultipleCoils(byte *frame, word startreg, word numoutputs,
                          byte bytecount);
#endif

  TRegister *searchRegister(word addr);

  //        void addReg(word address, word value = 0);
  bool Reg(word address, word value);
  word Reg(word address);
  byte rReg(word address, byte *buff);
  byte wReg(word address, byte *buff);

protected:
  byte *_frame;
  byte _len;
  byte _reply;
  unsigned long _timeout;
 
  void receivePDU(byte *frame);

public:
  static const TRegister registers[];
  ModbusAO();
  void begin(byte, unsigned long);
  void swapMode(bool);

  // void addHreg(word offset, word value = 0);
  bool Hreg(word offset, word value);
  word Hreg(word offset);
  byte rHreg(word offset, byte *buff);
  byte wHreg(word offset, byte *buff);

#ifndef USE_HOLDING_REGISTERS_ONLY
  bool Coil(word offset, bool value);
  bool Ists(word offset, bool value);
  bool Ireg(word offset, word value);

  bool Coil(word offset);
  bool Ists(word offset);
  // word Ireg(word offset);
  byte rIreg(word offset, byte *buff);
  byte wIreg(word offset, byte *buff);
#endif
  static void *reverseCopy(void *dst, void *src, byte size);
  static void swapWords(void *);
};

#endif // MODBUS_AO_H
