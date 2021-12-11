#ifndef _auto_tune_pid_h
#define _auto_tune_pid_h

extern double input, output, setpoint;
extern double periodPowerSum;
extern unsigned long periodPowerTime;
extern double kp,ki,kd;


void setupPID();
void loopPID();

bool rwCalcSmoothHreg(byte mode, word offset, byte *data, word len);
bool rwStatisticsSmoothHreg(byte mode, word offset, byte *data, word len);
bool rwTuningHreg(byte mode, word offset, byte *data, word len);
bool rwUpdateHreg(byte mode, word offset, byte *data, word len);
bool rwModeHreg(byte mode, word offset, byte *data, word len);
bool rwOutputHreg(byte mode, word offset, byte *data, word len);
bool rwKpHreg(byte mode, word offset, byte *data, word len);
bool rwKiHreg(byte mode, word offset, byte *data, word len);
bool rwKdHreg(byte mode, word offset, byte *data, word len);
bool rwSetPointHreg(byte mode, word offset, byte *data, word len);
bool rwIndexHreg(byte mode, word offset, byte *data, word len);
bool rwInputHreg(byte mode, word offset, byte *data, word len);


#endif // _auto_tune_pid_h
