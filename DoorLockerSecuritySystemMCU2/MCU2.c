/*
 * MCU2.c
 *
 *  Created on: May 30, 2023
 *      Author: Doha
 */

#include "uart.h"
#include "external_eeprom.h"
#include "twi.h"
#include"motor.h"
#include "buzzer.h"
#include"timer1.h"
#include <string.h>
#include<util/delay.h>
#include<avr/io.h>

/*************************************************************************************************
 *                   				   Preprocessor Macros            		                    *
 ************************************************************************************************/
#define PASS_SIZE 6 /*password size*/
/*UART messages*/
#define confirmed 1
#define NOtconfirmed 0
#define DOOR_OPENING 2
#define repeat 0x0F
#define OpentheDoor 0xF1
#define ChangePass 0xF2
/* Macros of Timer calculation*/
#define TicksNUM_PerONE_SEC 20	//Number of ticks in one compare matches per second
#define DoorISDone TicksNUM_PerONE_SEC*15 //Duration of opening or closing the Door
#define TimeToClose_Door DoorISDone+(3*TicksNUM_PerONE_SEC) //Duration of opening the Door and holding it for 3 seconds

/***************************************Global variables*************************************************/
char str1[PASS_SIZE];	// Password1
char str2[PASS_SIZE];	// Password2 for confirmation
uint8 value=0;	//value taken or written in EEPROM
volatile uint16  g_tick=0;	//counter of compare matches
Timer1_ConfigType TIMER_Data; // structure that holds  timer data(pre-scaler/start,compare value/ Timer mode
/****************************************************************************************/

/*************************************Function Declaration****************************************/
void step1_createPass();
void checkPass_step3(uint8 choice);
void count_to_Num();
void recievePass(char str[PASS_SIZE]);
void getpass_againstMemory();
/*************************************************************************************************/
int main(void)
{

	/***************************************initialization***************************************************/
	/* Initialize the TWI/I2C Driver */
	TWI_ConfigType TWI_Data;
	TWI_Data.address=0x01;
	TWI_Data.prescaler=prescaler_1;
	TWI_Data.bit_rate=0x02;
	TWI_init(&TWI_Data);

	/*UART initialization
	 * Configure UART to work with baud rate 9600, No parity check, send/recieve 8 bits at a time, with 1 stopBit*/
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

	/*Motor initialization*/
	DcMotor_Init();

	/*Buzzer initialization*/
	Buzzer_init();
	/******************************************************************************************************/
	SREG  |= (1<<7);              // Enable global interrupts in MC.

	step1_createPass();	//start receiving created password with its confirmation from MCU1

    while(1)
    {

    }
}
/*Description:
 * checks the tow passwords received from MCU1 if they are equal, confirmation is sent to MCU1 and
 * password is stored in the EEPROM. if they are not equal repeat
 *  then it receives message from MCU1 includes the user request either to open the Door or change the pass
 *  in case they are equal
 * */
void step1_createPass()
{
	/*MCU2 is receiving the tow passwords through UART*/
	recievePass(str1);
	recievePass(str2);
	if(strcmp(str1, str2)==0) /*if the 2 passwords are equal*/
	{
		UART_sendByte(confirmed);//send confirmation to MCU1 to got to the main options stage
		/*Insert password into EEPROM*/
		uint8 i;
		for(i=0 ; i<PASS_SIZE-1 ; i++)
		{
			EEPROM_writeByte((0x0311)+i,str1[i]); /* Write 0x0F in the external EEPROM */
			_delay_ms(10);
		}
		/*check which request is received from the user Open the Door or Change password*/
		if(UART_recieveByte()==OpentheDoor)
		{
			checkPass_step3(OpentheDoor);
		}
		else
		{
			_delay_ms(2000);
			checkPass_step3(ChangePass);
		}

	}
	else
	{
		/*if 2 passwords are not equal send message NotConfirmed to MCU */
		UART_sendByte(NOtconfirmed);
		step1_createPass();/*repeat the same process again*/
	}
}
/*Description:
 * check the password the user logged in with and open the door if confirmed
 * In case the password is incorrect for 3 consecutive times , open the buzzer for 1 minute
 *  */
void checkPass_step3(uint8 choice)
{
	getpass_againstMemory();	//Revives the first pass from MCU1 and take restore the one from EEPROM
	if(strcmp(str1, str2)==0)//if they are equal, strcamp will not result more or less than 0
	{
		UART_sendByte(confirmed);//MCU2 sends confirmation to MCU1
		if(choice==OpentheDoor)	//if user chose to open the door
		{
			DcMotor_Rotate(CW,100);		//Open the Door by turning motor clockwise with maximum speed
			UART_sendByte(DOOR_OPENING);//Send to MCU1 that Door is opening to show on LCD

			Timer1_setCallBack(count_to_Num);//Pass count_to_Num function address to be called when ISR works
			Timer1_init(&TIMER_Data);    // Start the timer.

		}else if(choice==ChangePass)	//if user chose to change Password
		{
			step1_createPass();		//restart the system
		}
	}
	else
		{
			UART_sendByte(-1);		/*send disconfirmation to MCU1*/
			if(UART_recieveByte()==repeat)	/*In case user did not exceed 3 chances MCU2 recives confirmation from MCU1 */
			{
				checkPass_step3(choice);	/*repeat the log in process*/
			}else
			{	/*If the user exceeded the 3 chances buzzer is activated*/
				Buzzer_on();
				_delay_ms(60000);//wait a minute
				Buzzer_off();//deactivate the buzzer
			}
		}
}
/*count_to_Num is called when ISR is fired*/
void count_to_Num()
{
	g_tick++; //each compare match tick counter is incremented (each 1/20 second in this example)
	if(g_tick==DoorISDone)		//if spent 15 seconds opening
	{
		DcMotor_Rotate(STOP,0);	//Stop the Door motor
	}
	if(g_tick==TimeToClose_Door)//if spent 18 seconds (opening the Door(15) + holding it(3))
	{
		DcMotor_Rotate(A_CW,100);//close the Door
	}
	if(g_tick==TimeToClose_Door+DoorISDone)//if spent 15 seconds(closing the door)+ 18 seconds (opening+holding the door)
	{
		DcMotor_Rotate(STOP,0);	//Stop the Door motor
	}
}
/*Description:
 * receives the password from MCU1 and stores it in the passes global array
 * */
void recievePass(char str[PASS_SIZE])
{
	uint8 i;
	for(i=0 ;i<PASS_SIZE ;i++)
	{
		str[i]=UART_recieveByte();/*receives byte by byte*/
	}
}
/*Description:
 * receives the password from MCU1 and stores it in the passes global array and restore the other one from EEPROM memory
 * */
void getpass_againstMemory()
{
	recievePass(str1);/*store the first pass in str1 array */
	uint8 i;
	for(i=0 ; i<PASS_SIZE-1 ; i++)
	{
		EEPROM_readByte(0x0311+i, &value);/*get values in address 0x0311 and its following addresses */
	    _delay_ms(10);
	    str2[i]=value;/*store the values in str2 array */
	}
}
