#ifndef AO_EEPROM_H
#define AO_EEPROM_H

#define EE_READ     0
#define EE_WRITE    1

// #define EE_AT       0
#define EE_MODE     2
#define EE_OUTPUT   6
#define EE_SETPNT   10
#define EE_KP       14
#define EE_KI       18
#define EE_KD       22

#define EE_INDEX    24  // 4*2
#define EE_SMOOTH   32  // 4*2

void rwEEMEM(byte mode, word offset, byte * data, word len);

#endif //AO_EEPROM_H