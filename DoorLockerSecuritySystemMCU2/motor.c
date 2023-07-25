/*
 * motor.c
 *
 *  Created on: May 18, 2023
 *      Author: Doha
 */

#include "motor.h"


/*Configure motor pins as output pins and stop the motor at the begging */
void DcMotor_Init(void)
{
	GPIO_setupPinDirection(INPUTPORT,INPUT1_PIN,PIN_OUTPUT);
	GPIO_setupPinDirection(INPUTPORT,INPUT2_PIN,PIN_OUTPUT);

	//STOP the motor at the begging
	GPIO_writePin(INPUTPORT,INPUT1_PIN, LOGIC_LOW);
	GPIO_writePin(INPUTPORT,INPUT2_PIN, LOGIC_LOW);

}
/*Change motor speed depend on the percentage of compare match from the maximum of the timer0 counter*/
 void DcMotor_Rotate(DcMotor_State state,uint8 speed)
 {

	 speed=(double)speed/100*255;/*calculate the value of compare match as a percentage of 255
	  the maximum of the timer0 counter*/
	 Timer0_PWM_Init(speed);
	 /*change the motor direction according to the state*/
	 if(state==CW)/*Clockwise*/
	 {
		 GPIO_writePin(INPUTPORT,INPUT1_PIN, LOGIC_LOW);
		 GPIO_writePin(INPUTPORT,INPUT2_PIN, LOGIC_HIGH);
	 }
	 else if(state==A_CW)/*Anti-Clockwise*/
	 {
		 GPIO_writePin(INPUTPORT,INPUT1_PIN, LOGIC_HIGH);
		 GPIO_writePin(INPUTPORT,INPUT2_PIN, LOGIC_LOW);
	 }
	 else if(state==STOP)/*stop*/
	 {
		 GPIO_writePin(INPUTPORT,INPUT1_PIN, LOGIC_LOW);
		 GPIO_writePin(INPUTPORT,INPUT2_PIN, LOGIC_LOW);
	 }



 }
