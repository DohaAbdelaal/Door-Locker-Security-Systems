/*
 * motor.h
 *
 *  Created on: May 18, 2023
 *      Author: Doha
 */

#ifndef MOTOR_H_
#define MOTOR_H_
#include"std_types.h"
#include "GPIO.h"
#include "common_macros.h"
#include "timer0.h"

#define INPUTPORT PORTB_ID
#define INPUT1_PIN PIN4_ID
#define INPUT2_PIN PIN5_ID
typedef enum
{
	CW,
	A_CW,
	STOP
}DcMotor_State;

 void DcMotor_Init(void);
 void DcMotor_Rotate(DcMotor_State state,uint8 speed);

#endif /* MOTOR_H_ */
