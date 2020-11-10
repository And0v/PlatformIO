#include <Arduino.h>
#include "EEPROM.h"
#include "aoEEPROM.h"

void rwEEMEM(byte mode, word offset, byte * data, word len){
    if ( mode == EE_READ ) {
      for(byte i = 0; i < len;++i){
        data[i] = EEPROM.read(offset+i);
      }
    }else{
      for(byte i = 0; i < len;++i){
        EEPROM.write(offset+i, data[i]);
      }
    }
}