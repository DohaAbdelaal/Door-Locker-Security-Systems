/*
 * main.c
 *
 *  Created on: May 3, 2023
 *      Author: Doha
 */


#include "lcd.h"
#include "keypad.h"
#include<util/delay.h>
#include"std_types.h"
#include<avr/io.h>
#include "uart.h"
#include"timer1.h"
#include<avr/interrupt.h>

/*************************************************************************************************
 *                   				   Preprocessor Macros            		                    *
 ************************************************************************************************/
#define PASS_SIZE 6/*password size*/
/*UART messages*/
#define CONFIRMED 1
#define DOOR_OPENING 2
#define repeat 0x0F
#define stop 0xFF
#define OpentheDoor 0xF1
#define ChangePass 0xF2
/*Macros of Timer calculations*/
#define TicksNUM_PerONE_SEC 20	//Number of ticks in one compare matches per second
#define DoorISDone TicksNUM_PerONE_SEC*15 //Time to stop opening or closing the Door
#define TimeToClose_Door DoorISDone+(3*TicksNUM_PerONE_SEC)	//Duration of opening the Door and holding it for 3 seconds

/****************************************Global Variables*****************************************************/
uint8 key;
char str1[6];
char str2[6];
volatile uint16 g_tick=0;
Timer1_ConfigType TIMER_Data;
/*************************************************************************************************************/

/****************************************Function Declaration*****************************************************/
void start();
void step1_createPass();
void step2_mainOptions();
void step3_openDoor(uint8 choice);
void count_to_Num();
void enterPass();
void getPass( char str[6]);
void SendPass(char str[PASS_SIZE]);
/***************************************************************************************************************/

int main()
{
	/*************************************initialization**********************************/
	/*LCD  initialization*/
	LCD_init();

	/*UART initialization
	 * Configure UART to work with baud rate 9600, No parity check, send/receive 8 bits at a time, with 1 stopBit*/
	UART_ConfigType UART_Data;
	UART_Data.baud_rate=9600;
	UART_Data.bit_data=MODE_8_BIT;
	UART_Data.parity=Disabled;
	UART_Data.stop_bit=MODE_1_STOPBIT;

	UART_init(&UART_Data);

	/*Timer initialization
	 * F_CPU=8 ,pre_scaler=8 F_timer=8/8= 1 MH, Timer tick =1 us, time of 1 sec= 1/50000* 10^(-6)=20 ticks
	 * */


	TIMER_Data.initial_value=0;
	TIMER_Data.compare_value=50000;
	TIMER_Data.prescaler=PRESCALER_8;
	TIMER_Data.mode=CTC;



	/********************************************************************************************/
	SREG  |= (1<<7);              // Enable global interrupts in MC.
	start();


	while(1)
	{

	}

}
/*Ask the user to create password and confirm it*/
void start()
{
	/*Send the 2 passwords to MCU2 and waits for its confirmation*/
	step1_createPass();
	if(UART_recieveByte())//if confirmed
	{
		step2_mainOptions();/*Enable the user to open the door or change the password*/
	}else
	{
		start();	/*Restart the system*/
	}
}
/*Enable the user to enter the password and its confirmation and sends them to MCU2*/
void step1_createPass()
{
	enterPass();

	/*Get the first pass word then clear the screen*/
	LCD_displayStringRowColumn(0,0,"PLZ re-enter the");
	LCD_displayStringRowColumn(1,0,"same pass: ");
	getPass(str2);
	while(KEYPAD_getPressedKey()!='='){_delay_ms(500);}/*wait for Enter(=) press*/
	_delay_ms(500);
	LCD_clearScreen();

	SendPass(str1);
	SendPass(str2);

}
/*Depends on the user choice to open the door or to change password it sends the suitable message to MCU2
 * and check if user still has the chance to try to enter password
 * */
void step3_openDoor(uint8 choice)
{
	static uint8 disconfirmCounter=0;/*chances counter*/
	enterPass();/*Enable the user to enter password*/
	SendPass(str1);/*store pass in the global array*/
	if(UART_recieveByte()==CONFIRMED)	/*If MCU2 confirmed that it is the correct pass save in the EEPROM*/
	{
		disconfirmCounter=0;/*recount the chances from zero */
		if(choice==OpentheDoor) /*In case user chose + to open the Door*/
		{
			/*MCU2 started opening the door and Opening messages should be displayed*/
				if(UART_recieveByte()==DOOR_OPENING)
				{
					LCD_displayStringRowColumn(0,0,"  Door is ");
					LCD_displayStringRowColumn(1,0," unlocking");

				}
				Timer1_setCallBack(count_to_Num);//Pass count_to_Num function address to be called when ISR works
				Timer1_init(&TIMER_Data);    // Start the timer.

		}else if(choice==ChangePass)/*In case user chose - to change password*/
		{
			start();/*restart the system*/
		}
	}
	else
	{
		disconfirmCounter++;
		if(disconfirmCounter<3)//if user entered password incorrectly for 3 times
		{
			/*send repeat message to MCU2 and enable user to log in as another chance*/
			UART_sendByte(repeat);
			step3_openDoor(choice);
		}else
		{
			LCD_clearScreen();
			LCD_displayStringRowColumn(0,0,"WRONG PASS!!");
			UART_sendByte(stop);/*send stop message to MCU2 to stop the system from receiving passwords*/
			_delay_ms(60000);/*Stop the system for 1 minute no inputs or any interaction*/
			step2_mainOptions();/*show the previous options again*/
		}
	}
}
/*It is executed each compare match to organize the messages time of opening or closing the door*/
void count_to_Num()
{
	g_tick++;/*Increment the counter tick each compare match*/
	/*show message After the opening time and the holding time*/
	if(g_tick==TimeToClose_Door)
	{
		LCD_displayStringRowColumn(0,0,"  Door is ");
		LCD_displayStringRowColumn(1,0," locking");

	}
	/*when the 15 secs of closing the door are counted stop the timer and clear the screen*/
	if(g_tick==TimeToClose_Door+DoorISDone)
	{
		Timer1_deInit();
		LCD_clearScreen();
	}
}
/*It communicates with user to take the passwords and organize the screen then sotre it in str1 array*/
void enterPass()
{
		LCD_displayStringRowColumn(0,0,"PLZ enter pass");
		LCD_moveCursor(1,0); /* Move the cursor to the second row */

		/*Get the first pass word then clear the screen*/
		getPass(str1);
		while(KEYPAD_getPressedKey()!='='){_delay_ms(500);}/*wait for Enter(=) press*/
		_delay_ms(500);
		LCD_clearScreen();
}
/*Enable the user to open the door or to change the pass and send the user choice to MCU2*/
void step2_mainOptions()
{
	LCD_displayStringRowColumn(0,0," + : Open Door");
	LCD_displayStringRowColumn(1,0," - : Change Pass");
	if(KEYPAD_getPressedKey()=='+')
	{	UART_sendByte(OpentheDoor);/*Send the user request to MCU2*/
		LCD_clearScreen();//Open the Door if entered password is correct
		step3_openDoor(OpentheDoor);/*check the correction of the pass*/
	}
	else if(KEYPAD_getPressedKey()=='-')
	{
		UART_sendByte(0xF2); /*Send the user request to MCU2 to change pass*/
		LCD_clearScreen();
		step3_openDoor(ChangePass);/*check the correction of the pass*/

	}

}

/*get the password from the keypad*/
void getPass( char str[6])
{
	_delay_ms(500);
	uint8 i;
	for(i=0 ; i<5 ;i++)
	{

		key = KEYPAD_getPressedKey();
		LCD_displayCharacter('*'); /* display the pressed keypad switch */
		str[i]=(char)key;

		_delay_ms(500);
	}
}
/*Send password to MCU2 byte by Byte*/
void SendPass(char str[PASS_SIZE])
{
	uint8 i;
	for(i=0 ;i<PASS_SIZE ;i++)
	{
		UART_sendByte(str[i]);
	}
}

