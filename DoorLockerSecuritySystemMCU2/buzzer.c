/*
 * buzzer.c
 *
 *  Created on: Jun 7, 2023
 *      Author: Doha
 */

#include"buzzer.h"
/*Set Direction for buzzer bin and close it by default*/
 void Buzzer_init()
 {
	 GPIO_setupPinDirection(BUZZER_PORT,BUZZER_PIN,PIN_OUTPUT);
	 GPIO_writePin(BUZZER_PORT,BUZZER_PIN,LOGIC_LOW);
 }
 /*Open the buzzer by writing 1 on the buzzer pin*/
 void Buzzer_on(void)
 {
	 GPIO_writePin(BUZZER_PORT,BUZZER_PIN,LOGIC_HIGH);
 }
 /*Open the buzzer by writing zero on the buzzer pin*/
 void Buzzer_off(void)
 {
	 GPIO_writePin(BUZZER_PORT,BUZZER_PIN,LOGIC_LOW);
 }
