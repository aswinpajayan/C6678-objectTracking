#ifndef SERIALPORT_H_
#define SERIALPORT_H_
#endif

void SerialPortInit(uint32_t);
void SerialPortEnable(void);
void SerialPortDisable(void);
uint32_t SerialPortGetData(void);

extern uint32_t X_cord,Y_cord;
extern bool CordRdy;
