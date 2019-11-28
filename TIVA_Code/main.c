#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/gpio.c"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/pwm.h"
#include "driverlib/uart.h"
#include "math.h"

//#include "MasterControl.h"
#include "SerialPort.h"
#include "Servo.h"
#include "HID.h"

#define res_X_2 160.0
#define res_Y_2 120.0
#define img_AR res_Y_2/res_X_2

bool ServoBusy = 0;
uint32_t X_cord = 0,Y_cord = 0;
bool CordRdy = 0;
//uint16_t s1_ind=0,s2_ind=0;
//int servo1hst[1000],servo2hst[1000];

void Track_cord(uint32_t,uint32_t);
void delayms(unsigned int);

int main(void)
{
	SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ);	//System clock=PLL(400MHz)/2.5/2=80MHz
	/**********Configure peripherals**********/
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);										//Enable PortA peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);										//Enable PortB peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);										//Enable PortC peripheral
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);										//Enable PortD peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);										//Enable PortE peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);										//Enable PortF peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);										//Enable Timer0 peripheral(Servo control)
	TimerClockSourceSet(TIMER0_BASE,TIMER_CLOCK_SYSTEM);								//Clock source to Timer0(Servo control)
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);										//Enable Timer1 peripheral(System Timer)
	//TimerClockSourceSet(TIMER1_BASE,TIMER_CLOCK_SYSTEM);								//Clock source to Timer1(System Timer)
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);										//Enable UART0 Peripheral(Serial Port)
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);										//Enable UART7 Peripheral(Serial Port)
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);											//Enable PWM1 peripheral(Servo)
	SysCtlPWMClockSet(SYSCTL_PWMDIV_32);												//80MHz/32=2.5MHz to PWM module(Servo)

	IntMasterEnable();																	//Global interrupt enable
	IntPrioritySet(INT_UART7,0x00);														//Priority=0 for limit switches
	IntPrioritySet(INT_TIMER0A,0x20);													//Priority=1 for Timer0A(Stepper motor)
	//IntPrioritySet(INT_TIMER1A,0x40);													//Priority=2 for Timer1A(System timer)

	ServoInit();
	LCDInit();
	UserSwitchInit();
	ServoEnable();
	ServoAngle(Servo1,90);
	ServoAngle(Servo2,90);
	delayms(2000);
	SerialPortInit(115200);
	SerialPortEnable();

	static uint8_t Sw_state = 0;

	while(1)
	{
		switch(Sw_state)
		{
		case 0:
			Track_cord(res_X_2,res_Y_2);
			X_cord = res_X_2;Y_cord = res_Y_2;
			LCDWritecmd(CLR_DISP);
			LCDWritedata("    Reset!!!    ");
			LCDSetcursor(1,0);
			LCDWritedata("S1:Cont");
			Sw_state = 3;
			break;
		case 1:
			LCDWritecmd(CLR_DISP);
			LCDWritedata("    Paused!!    ");
			LCDSetcursor(1,0);
			LCDWritedata("S1:Cont   S2:Rst");
			Sw_state = 5;
			break;
		case 2:
			if(CordRdy)
			{
				CordRdy = 0;
				Track_cord(X_cord,Y_cord);
				LCDWritecmd(CLR_DISP);
				LCDWritedata("X:");LCDWritedata(InTtoChar(X_cord));
				LCDSetcursor(0,8);
				LCDWritedata("Y:");LCDWritedata(InTtoChar(Y_cord));
				LCDSetcursor(1,0);
				LCDWritedata("S1:Pause  S2:Rst");
				Sw_state = 4;
			}
			else
				Sw_state = 4;
			break;
		case 3:
			while(UserSwitchRead(Sw_1));
			delayms(20);
			while(!UserSwitchRead(Sw_1));
			delayms(20);
			LCDWritecmd(CLR_DISP);
			Sw_state = 2;
			break;
		case 4:
			if(!UserSwitchRead(Sw_2))
			{
				delayms(20);
				while(!UserSwitchRead(Sw_2));
				delayms(20);
				Sw_state = 0;
			}
			else if(!UserSwitchRead(Sw_1))
			{
				delayms(20);
				while(!UserSwitchRead(Sw_1));
				delayms(20);
				Sw_state = 1;
			}
			else
				Sw_state = 2;
			break;
		case 5:
			if(!UserSwitchRead(Sw_2))
			{
				delayms(20);
				while(!UserSwitchRead(Sw_2));
				delayms(20);
				Sw_state = 0;
			}
			else if(!UserSwitchRead(Sw_1))
			{
				delayms(20);
				while(!UserSwitchRead(Sw_1));
				delayms(20);
				LCDWritecmd(CLR_DISP);
				Sw_state = 2;
			}
			else
				Sw_state = 5;
			break;
		}




		/*if(Rst_flag)
		{
			Rst_flag = 0;
			Track_cord(500,500);
			LCDWritecmd(CLR_DISP);
			LCDWritedata("    Reset!!!    ");
			LCDSetcursor(1,0);
			LCDWritedata("S1:Cont");
			while(UserSwitchRead(Sw_1));
			delayms(20);
			while(!UserSwitchRead(Sw_1));
			LCDWritecmd(CLR_DISP);
		}
		else if(!UserSwitchRead(Sw_1))
		{
			LCDWritecmd(CLR_DISP);
			LCDWritedata("    Paused!!    ");
			LCDSetcursor(1,0);
			LCDWritedata("S1:Cont   S2:Rst");
			while(UserSwitchRead(Sw_1));
			LCDWritecmd(CLR_DISP);
		}
		else if(CordRdy)
		{
			CordRdy = 0;
			Track_cord(X_cord,Y_cord);
			LCDWritecmd(CLR_DISP);
			LCDWritedata("X:");LCDWritedata(InTtoChar(X_cord));
			LCDSetcursor(0,8);
			LCDWritedata("Y:");LCDWritedata(InTtoChar(Y_cord));
			LCDSetcursor(1,0);
			LCDWritedata("S1:Pause  S2:Rst");
		}*/
	}

	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2);

	ServoMov(Servo1,45);
	ServoMov(Servo2,45);
	while(ServoBusy);
	ServoMov(Servo1,135);
	while(ServoBusy);
	ServoMov(Servo2,135);
	while(ServoBusy);
	ServoMov(Servo1,45);
	while(ServoBusy);
	ServoMov(Servo2,45);
	while(ServoBusy);
	ServoMov(Servo1,90);
	ServoMov(Servo2,90);
	while(ServoBusy);

	//SetSysTimer(5);
	while(1);
}

void Track_cord(uint32_t xcord,uint32_t ycord)//Asumption: turret is at centre of 1mx1m frame and at 1m distance and xcord and ycord are in mm
{
	int32_t s1_angle,s2_angle;
	s1_angle = 90.0 + atanf(1.0-xcord/res_X_2)*57.2957795;
	s2_angle = 90.0 + atanf(1.0-ycord/res_Y_2)*57.2957795*img_AR;
	ServoMov(Servo1,s1_angle);
	ServoMov(Servo2,s2_angle);
}

void delayms(unsigned int time)
{
	unsigned int i,j;
	for(i=0;i<time;i++)
	for(j=0;j<4500;j++);
}
