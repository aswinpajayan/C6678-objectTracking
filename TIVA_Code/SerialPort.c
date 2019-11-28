#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"
#include "inc/hw_types.h"
#include "driverlib/fpu.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/uart.c"

//#include "MasterControl.h"
#include "SerialPort.h"

bool SerialPortEn = 0;
uint8_t data_state = 0;

void SerialPortInit(uint32_t Baud_Rate)									//Initialize Serial port
{
	GPIOPinTypeUART(GPIO_PORTA_BASE,GPIO_PIN_0|GPIO_PIN_1);				//UART0 pins RX|TX
	//GPIOPinTypeUART(GPIO_PORTE_BASE,GPIO_PIN_0|GPIO_PIN_1);				//UART7 pins RX|TX
	GPIOPinConfigure(GPIO_PA0_U0RX);									//PA0 mux set for U0RX
	GPIOPinConfigure(GPIO_PA1_U0TX);									//PA1 mux set for U0TX
	//GPIOPinConfigure(GPIO_PE0_U7RX);									//PE0 mux set for U7RX
	//GPIOPinConfigure(GPIO_PE1_U7TX);									//PE1 mux set for U7TX
	UARTConfigSetExpClk(UART0_BASE,SysCtlClockGet(),Baud_Rate,(UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));	//UART0,80MHz/5=16MHz,Baud rate(bps),(8bit data|1 Stop bit|No parity bit)
	//UARTConfigSetExpClk(UART7_BASE,SysCtlClockGet(),Baud_Rate,(UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));	//UART7,80MHz/5=16MHz,Baud rate(bps),(8bit data|1 Stop bit|No parity bit)
	//UARTFIFOEnable(UART0_BASE);
	//UARTFIFOLevelSet(UART0_BASE,UART_FIFO_TX4_8,UART_FIFO_RX4_8);
	IntEnable(INT_UART0);												//Enable UART0 interrupt
	UARTIntClear(UART0_BASE,UART_INT_RX);								//Clear UART0 Receive interrupt flag
	UARTIntEnable(UART0_BASE,UART_INT_RX);								//UART0 Receive interrupt
	//IntEnable(INT_UART7);												//Enable UART7 interrupt
	//UARTIntClear(UART7_BASE,UART_INT_RX);								//Clear UART7 Receive interrupt flag
	//UARTIntEnable(UART7_BASE,UART_INT_RX);								//UART7 Receive interrupt
	//SerialPortDisable();
}

void SerialPortEnable(void)
{
	UARTEnable(UART0_BASE);												//Enable UART0
	UARTFIFODisable(UART0_BASE);										//Disable UART0 FIFO
	//UARTEnable(UART7_BASE);												//Enable UART7
	//UARTFIFODisable(UART7_BASE);										//Disable UART7 FIFO
	SerialPortEn = 1;
}

void SerialPortDisable(void)
{
	UARTDisable(UART0_BASE);											//Disable UART0
	//UARTDisable(UART7_BASE);											//Disable UART7
	SerialPortEn = 0;
}

uint32_t SerialPortGetData(void)										//Receive data from serial port and store it in Xcord[] & Ycord[]; Returns length
{
	char Serial_data = 0;
	uint32_t length = 0;
	//uint32_t num;
	Serial_data = UARTCharGet(UART0_BASE);								//Read data in UART0 Rx buffer
	UARTCharPut(UART0_BASE,Serial_data);
	/*while(Serial_data != 'E')
	{
		if(Serial_data == 'X')
		{
			for(num=0;;)
			{
				Serial_data = UARTCharGet(UART0_BASE);
				UARTCharPut(UART0_BASE,Serial_data);
				if(Serial_data != 'Y')
				num = num*10+(Serial_data-'0');
				else
					break;
			}
			Xcord[length] = num;										//X-coordinate
			for(num=0;;)
			{
				Serial_data = UARTCharGet(UART0_BASE);
				UARTCharPut(UART0_BASE,Serial_data);
				if(Serial_data != '\r')
				num = num*10+(Serial_data-'0');
				else
					break;
			}
			Ycord[length] = num;										//Y-coordinate
			length++;
		}
		Serial_data = UARTCharGet(UART0_BASE);
		UARTCharPut(UART0_BASE,Serial_data);
	}*/
	return(length);
}

void UART0ISR(void)
{
	UARTIntClear(UART0_BASE,UART_INT_RX);								//Clear UART0 Receive interrupt flag
	char data = UARTCharGetNonBlocking(UART0_BASE);						//Read data in UART0 Rx buffer
	if(data == 'X'){data_state = 1;}
	else if(data == 'Y'){data_state = 3;}
	else if(data == '\r'){CordRdy = 1;data_state = 0;}
	switch(data_state)
	{
	case 1:
		X_cord = 0;data_state = 2;
		break;
	case 2:
		X_cord = X_cord*10+(data-'0');
		break;
	case 3:
		Y_cord = 0;data_state = 4;
		break;
	case 4:
		Y_cord = Y_cord*10+(data-'0');
		break;
	default:
		break;
	}
}

/*void UART7ISR(void)
{
	UARTIntClear(UART7_BASE,UART_INT_RX);								//Clear UART7 Receive interrupt flag
	char data = UARTCharGetNonBlocking(UART7_BASE);						//Read data in UART7 Rx buffer
	if(data == 'X'){data_state = 1;}
	else if(data == 'Y'){data_state = 3;}
	else if(data == '\r'){CordRdy = 1;data_state = 0;}
	switch(data_state)
	{
	case 1:
		X_cord = 0;data_state = 2;
		break;
	case 2:
		X_cord = X_cord*10+(data-'0');
		break;
	case 3:
		Y_cord = 0;data_state = 4;
		break;
	case 4:
		Y_cord = Y_cord*10+(data-'0');
		break;
	default:
		break;
	}
}*/
