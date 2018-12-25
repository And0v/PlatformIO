#ifndef _auto_tune_pid_h
#define _auto_tune_pid_h

extern double input, output, setpoint;
extern double kp,ki,kd;


void setupPID();
void loopPID();


#endif // _auto_tune_pid_h
