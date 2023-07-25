/*
 * buzzer.h
 *
 *  Created on: Jun 7, 2023
 *      Author: Doha
 */

#ifndef BUZZER_H_
#define BUZZER_H_
#include "GPIO.h"
#include"std_types.h"
#define BUZZER_PIN PIN7_ID
#define BUZZER_PORT PORTD_ID

 void Buzzer_init();
 void Buzzer_on(void);
 void Buzzer_off(void);

#endif /* BUZZER_H_ */
