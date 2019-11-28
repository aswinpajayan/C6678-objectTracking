#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "driverlib/pwm.h"

//#include "MasterControl.h"
#include "Servo.h"

bool ServoEn = 0;

static int32_t cur_angle1 = 90,fin_angle1 = 90,cur_angle2 = 90,fin_angle2 = 90;

void ServoInit(void)																//Initialize Servo motor
{
	GPIOPinTypePWM(GPIO_PORTA_BASE,GPIO_PIN_6|GPIO_PIN_7);							//Servo1 signal(PA6)(M1PWM2),Servo2 signal(PA7)(M1PWM3)
	GPIOPinConfigure(GPIO_PA6_M1PWM2);												//PA6 mux set for M1PWM2
	GPIOPinConfigure(GPIO_PA7_M1PWM3);												//PA7 mux set for M1PWM3

	PWMGenConfigure(PWM1_BASE,PWM_GEN_1,PWM_GEN_MODE_DOWN|PWM_GEN_MODE_NO_SYNC|PWM_GEN_MODE_DBG_RUN);	//PWM module 1,generator 1,Count down mode,No sync
	PWMGenPeriodSet(PWM1_BASE,PWM_GEN_1,2500000/Servo_Frequency);					//50Hz pwm frequency to generator 1

	ServoEn = 0;

	TimerConfigure(TIMER0_BASE,TIMER_CFG_A_PERIODIC);									//Timer0_A in down counting periodic mode
	IntEnable(INT_TIMER0A);																//Enable Timer0_A interrupt
	TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);										//Clear Timer0_A timeout flag
	TimerIntEnable(TIMER0_BASE,TIMER_TIMA_TIMEOUT);										//Timer0_A overflow interrupt
	TimerLoadSet(TIMER0_BASE,TIMER_A,SysCtlClockGet()*Servo_sample_prd/1000);			//Timer overflows after every Servo_sample_prd mSec
	TimerEnable(TIMER0_BASE,TIMER_A);													//Start Timer0_A
}

void ServoEnable(void)																//Enable Servo motor
{
	PWMGenEnable(PWM1_BASE,PWM_GEN_1);												//Enable PWM 1 generator 1(starts counter)
	PWMOutputState(PWM1_BASE,PWM_OUT_2_BIT|PWM_OUT_3_BIT,1);						//Enable PWM 1 output at PA6(PWM_OUT_2BIT),PA7(PWM_OUT_3BIT)
	ServoEn = 1;
}

void ServoDisable(void)																//Disable Servo motor
{
	PWMOutputState(PWM1_BASE,PWM_OUT_2_BIT|PWM_OUT_3_BIT,0);						//Disable PWM output at PA6(PWM_OUT_2BIT),PA7(PWM_OUT_3BIT)
	PWMGenDisable(PWM1_BASE,PWM_GEN_1);												//Disable PWM generator 1(stops counter)
	ServoEn = 0;
}

void ServoAngle(int32_t servo_num,int32_t angle_val)								//Set servo angle
{
	/*if(angle_val<Servo_minAngle)													//Constrain=> Servo_minAngle<angle_val<Servo_maxAngle
		angle_val = Servo_minAngle;
	else if(angle_val>Servo_maxAngle)
		angle_val = Servo_maxAngle;*/
	uint32_t onTime = (Servo_minPulse+(angle_val-Servo_minAngle)*Servo_sensitivity);//Calculate on time of pulse
	switch(servo_num)
	{
	case 1:
		PWMPulseWidthSet(PWM1_BASE,PWM_OUT_2,onTime*25);							//Set Servo1 pulse width
		break;
	case 2:
		PWMPulseWidthSet(PWM1_BASE,PWM_OUT_3,onTime*25);							//Set Servo2 pulse width
		break;
	default:break;
	}

}

void ServoMov(int32_t servo_num,int32_t angle_val)
{
	ServoBusy = 1;
	if(angle_val%2!=0)
		angle_val-=1;
	if(angle_val<Servo_minAngle)													//Constrain=> Servo_minAngle<angle_val<Servo_maxAngle
		angle_val = Servo_minAngle;
	else if(angle_val>Servo_maxAngle)
		angle_val = Servo_maxAngle;
	switch(servo_num)
	{
	case 1:
		fin_angle1 = angle_val;
		break;
	case 2:
		fin_angle2 = angle_val;
		break;
	default:break;
	}
}

void ServoControl(void)																//Stepper interrupt handler (Timer0A)
{
	TimerIntClear(TIMER0_BASE,TIMER_TIMA_TIMEOUT);										//Clear Timer0_A timeout flag
	uint8_t Servo_status = 0x03;
	if(cur_angle1<fin_angle1){cur_angle1+=2;ServoAngle(Servo1,cur_angle1);/*servo1hst[s1_ind++]=cur_angle1;*/}
	else if(cur_angle1>fin_angle1){cur_angle1-=2;ServoAngle(Servo1,cur_angle1);/*servo1hst[s1_ind++]=cur_angle1;*/}
	else{Servo_status&=0xFE;}
	if(cur_angle2<fin_angle2){cur_angle2+=2;ServoAngle(Servo2,cur_angle2);/*servo2hst[s2_ind++]=cur_angle2;*/}
	else if(cur_angle2>fin_angle2){cur_angle2-=2;ServoAngle(Servo2,cur_angle2);/*servo2hst[s2_ind++]=cur_angle2;*/}
	else{Servo_status&=0xFD;}
	if(Servo_status == 0x00)
		ServoBusy = 0;


	//x ^= 0x04;
	//GPIOPinWrite(GPIO_PORTF_BASE,0x04,x);
}
