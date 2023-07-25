/*
 * timer1.c
 *
 *  Created on: Jun 11, 2023
 *      Author: Doha
 */
#include "timer1.h"
#include "avr/interrupt.h"
#include "GPIO.h"


/* Global variables to hold the address of the call back function in the application
 * pointer to void function and does not take an argument */
static void (*g_callBackPtr)(void) = NULL_PTR;

ISR(TIMER1_COMPA_vect)
{
	if(g_callBackPtr != NULL_PTR)
	{
		/* Call the Call Back function in the application after the edge is detected */
		(*g_callBackPtr)(); /* another method to call the function using pointer to function g_callBackPtr(); */
	}
}

/* Description:
 * For System Clock = 8Mhz and prescaler F_CPU/8.
 * Timer frequency will be around 1Khz, Ttimer = 1ms
 * For compare value equals to 50000 the timer will generate compare match interrupt every 1/20 s.
 * Compare interrupt will be generated every 1/20 s so we need two compare interrupts to count 1 second.
 */

void Timer1_init(const Timer1_ConfigType * Config_Ptr)
{
	TCNT1=Config_Ptr->initial_value;//counter start
	OCR1A=Config_Ptr->compare_value;
	GPIO_writePin(TCCR1A,FOC1A,LOGIC_HIGH);//FOC1A is set in Normal or Compare mode(Non PWM)

	/*To insert timer mode Normal/compare mode*/
	TCCR1A |=(TCCR1A & 0xFC)|(Config_Ptr->mode  & 0x03);//WGM01 WMG00
	TCCR1B |=(TCCR1B & 0xE7)|(((Config_Ptr->mode & 0x0C)>>2)<<WGM12);//WGM13 WG12


	TCCR1B |=(TCCR1B & 0xF8)|(Config_Ptr->prescaler);
	TIMSK=(1<<OCIE1A);	//Enable Output Compare A Match Interrupt
}
/*Enables ISR to get the function address*/
void Timer1_setCallBack(void(*a_ptr)(void))
{
	g_callBackPtr=a_ptr;
}

/*Disable timer clock*/
void Timer1_deInit(void)
{
	TCCR1B=0;
}
