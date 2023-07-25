/*
 * timer1.h
 *
 *  Created on: Jun 11, 2023
 *      Author: Doha
 */

#ifndef TIMER1_H_
#define TIMER1_H_
#include"common_macros.h"
#include <avr/io.h>
#include "std_types.h"
typedef enum
{
	No_CLOCK,NO_prescaling,PRESCALER_8,PRESCALER_64,PRESCALER_256,
	EXTERNAL_CLOCK_FALLING_EDGE,EXTERNAL_CLOCK_RISING_EDGE
}Timer1_Prescaler;
typedef enum
{
	NORMAL,CTC=4
}Timer1_Mode;
typedef struct {
 uint16 initial_value;
 uint16 compare_value; // it will be used in compare mode only.
 Timer1_Prescaler prescaler;
 Timer1_Mode mode;
} Timer1_ConfigType;
void Timer1_init(const Timer1_ConfigType * Config_Ptr);
void Timer1_setCallBack(void(*a_ptr)(void));
void Timer1_deInit(void);


#endif /* TIMER1_H_ */
