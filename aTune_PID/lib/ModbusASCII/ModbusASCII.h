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
        virtual bool receive(byte* frame);
        virtual void sendPDU(byte* pduframe);
        void send(byte* frame);
		byte calcLRC(byte *auchMsg, unsigned short usDataLen);
		byte ascii2byte(byte *auchMsg);
    void byte2ascii(byte data, byte *auchMsg);   
};

#endif //MODBUSRTU_H
