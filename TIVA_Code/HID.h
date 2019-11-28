#ifndef HID_H_
#define HID_H_
#endif

#define Sw_R 1
#define Sw_L 2
#define Sw_U 3
#define Sw_D 4
#define Sw_1 5
#define Sw_2 6

#define LED_val(x) GPIOPinWrite(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,x)
#define R 0x02
#define G 0x08
#define B 0x04

  /*LED_val(R|G);yellow
	LED_val(G|B);cyan
	LED_val(R|B);purple
	LED_val(R|G|B);white*/

#define LCD_RS(x) GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_4,x<<4)	//RS=1/0=data/cmd
#define LCD_RW(x) GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_5,x<<5)	//RW=1/0=read/write
#define LCD_EN(x) GPIOPinWrite(GPIO_PORTC_BASE,GPIO_PIN_6,x<<6)	//EN=0-1=latches data
#define LCD_Port_val(x) GPIOPinWrite(GPIO_PORTB_BASE,0xFF,x)	//Port B => lcd port
#define CLR_DISP 0x01
#define RET_HOME 0x02

void UserSwitchInit(void);
uint8_t UserSwitchRead(uint8_t);
//void LEDInit(void);
void LCDInit(void);
void LCDWritecmd(char);
void LCDWritedata(char*);
void LCDWritechar(char);
void LCDSetcursor(char,char);
char *InTtoChar(unsigned int);
