#ifndef SERVO_H_
#define SERVO_H_
#endif

#define Servo_minPulse 120			//Minimum pulse width=Servo_minPulse*10(uS)//DEF:80
#define Servo_maxPulse 180			//Maximum pulse width=Servo_minPulse*10(uS)//DEF:180
#define Servo_minAngle 44			//Min Servo angle(deg)
#define Servo_maxAngle 136			//Max Servo angle(deg)
#define Servo_Frequency 40			//Servo refresh rate(Hz) default = 40
#define Servo_sample_prd 25.6		//Servo sample period(mS) default = 25.6
#define Servo_sensitivity (Servo_maxPulse-Servo_minPulse)/(Servo_maxAngle-Servo_minAngle)//(uS/deg)
#define Servo1 1
#define Servo2 2

void ServoInit(void);
void ServoEnable(void);
void ServoDisable(void);
void ServoAngle(int32_t,int32_t);
void ServoMov(int32_t,int32_t);

extern bool ServoBusy;
//extern uint16_t s1_ind,s2_ind;
//extern int servo1hst[1000],servo2hst[1000];
