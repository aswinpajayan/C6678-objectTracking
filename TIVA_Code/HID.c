#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "driverlib/gpio.h"
#include "math.h"

#include "HID.h"

static uint8_t cursor_pos = 0x00;
char CharArray[17];

/**********User Switch**********/
void UserSwitchInit(void)
{
	//GPIOPinTypeGPIOInput(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);										//SwX+|SwX-|SwY+|SwY- as input pins
	//GPIOPadConfigSet(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);	//SwX+,SwX-,SwY+,SwY- are configured as Weak Pull-up

	HWREG(GPIO_PORTF_BASE+GPIO_O_LOCK) = GPIO_LOCK_KEY;																		//To unlock PF0 pin
	HWREG(GPIO_PORTF_BASE+GPIO_O_CR) |= GPIO_PIN_0;
	GPIOPinTypeGPIOInput(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4);															//SW2 and SW1 as input pins
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD_WPU);						//SW2 and SW1 with input pullup
}

uint8_t UserSwitchRead(uint8_t mode)
{
	uint8_t switchVal;
	switch(mode)
	{
	case 0:
		//switchVal = GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0|GPIO_PIN_4);								//Read SW2, SW1
		//switchVal = (switchVal|switchVal<<5)&0x30;
		//switchVal |= GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5)>>2;	//Read SwX+,SwX-,SwY+,SwY-//0 0 SW2 SW1	SwY- SwY+ SwX- SwX+
		break;
	case 1://switchVal = GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_2)>>2;									//Read SwX+
		break;
	case 2://switchVal = GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_3)>>3;									//Read SwX-
		break;
	case 3://switchVal = GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_4)>>4;									//Read SwY+
		break;
	case 4://switchVal = GPIOPinRead(GPIO_PORTA_BASE,GPIO_PIN_5)>>5;									//Read SwY-
		break;
	case 5:switchVal = GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_4)>>4;									//Read SW1
		break;
	case 6:switchVal = GPIOPinRead(GPIO_PORTF_BASE,GPIO_PIN_0);										//Read SW2
		break;
	}
	return(switchVal);
}

/**********RGB LED**********/
/*void LEDInit(void)
{
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);												//Red, Blue and Green led as output pin
	GPIOPadConfigSet(GPIO_PORTF_BASE,GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3,GPIO_STRENGTH_2MA,GPIO_PIN_TYPE_STD);					//LED pins as configured as Push-Pull with 2mA drive strength
	LED_val(0);
}*/

/**********16x2 LCD**********/
void LCDInit(void)
{
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,0xFF);				//LCD port pins as output
	GPIOPinTypeGPIOOutput(GPIO_PORTC_BASE,0x70);				//LCD RS,RW,EN pins as output
	LCDWritecmd(0x38);											//8-bit, 2lines, 5x8
	LCDWritecmd(0x38);											//8-bit, 2lines, 5x8
	LCDWritecmd(0x0C);											//Display on, cursor off
	LCDWritecmd(0x06);											//Cursor auto increment
	LCDWritecmd(CLR_DISP);										//Clear display, set cursor at home
}

void LCDWritecmd(char cmd)
{
	LCD_RS(0);
	LCD_RW(0);
	LCD_EN(1);
	LCD_Port_val(cmd);
	LCD_EN(0);
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE,0xFF);
	LCD_RW(1);
	LCD_EN(1);
	do
	{
		cursor_pos = GPIOPinRead(GPIO_PORTB_BASE,0xFF);
	}
	while((cursor_pos&0x80)>>7);
	LCD_EN(0);
	LCD_RW(0);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,0xFF);
}

void LCDWritedata(char data[])
{
	int i,length = 0;
	while(data[length]!='\0')
		length++;
	for(i=0;i<length;i++)
	{
		LCD_RS(1);
		LCD_RW(0);
		LCD_EN(1);
		LCD_Port_val(data[i]);
		LCD_EN(0);
		GPIOPinTypeGPIOInput(GPIO_PORTB_BASE,0xFF);
		LCD_RS(0);
		LCD_RW(1);
		LCD_EN(1);
		do
		{
			cursor_pos = GPIOPinRead(GPIO_PORTB_BASE,0xFF);
		}
		while((cursor_pos&0x80)>>7);
		LCD_EN(0);
		LCD_RW(0);
		GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,0xFF);
	}
}

void LCDWritechar(char data)
{
	LCD_RS(1);
	LCD_RW(0);
	LCD_EN(1);
	LCD_Port_val(data);
	LCD_EN(0);
	GPIOPinTypeGPIOInput(GPIO_PORTB_BASE,0xFF);
	LCD_RS(0);
	LCD_RW(1);
	LCD_EN(1);
	do
	{
		cursor_pos = GPIOPinRead(GPIO_PORTB_BASE,0xFF);
	}
	while((cursor_pos&0x80)>>7);
	LCD_EN(0);
	LCD_RW(0);
	GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE,0xFF);
}

void LCDSetcursor(char row,char col)							//row => 0 to 1; col => 0 to 39
{
	if(row>1)
		row = 1;
	if(col>39)
		col = 39;
	cursor_pos = row*0x40 + col;
	LCDWritecmd(cursor_pos|0x80);
}

char *InTtoChar(uint32_t num)
{
	int i,length;
	if(num>0)
		length = log10(num);
	else
		length = 0;
    for (i=length;i>=0;i--)
    {
    	CharArray[i] = (num % 10) + '0';
    	num /= 10;
    }
    CharArray[length+1] = '\0';
    return CharArray;
}
