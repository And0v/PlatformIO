/*
    ModbusSerial.h - Header for ModbusSerial Library
    Copyright (C) 2014 Andr� Sarmento Barbosa
*/
#include <Arduino.h>
#include <ModbusRTU.h>

#ifndef MODBUSASCII_H
#define MODBUSASCII_H



class ModbusASCII : public ModbusRTU {
    public:
        ModbusASCII();
        bool receive(byte* frame);
        void sendPDU(byte* pduframe);
        void send(byte* frame);
		byte calcLRC(byte *auchMsg, unsigned short usDataLen);
		byte ascii2byte(byte *auchMsg);
};

#endif //MODBUSRTU_H
