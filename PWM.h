/*
 * PWM.c
 *
 *
 * 		PWM module controls the device's four PWM outputs through Timer_A and Timer_B output modules
 *  by changing their CCR1 and CCR2 values respectively. The timers are configured to work with 16 MHz
 *  clock signal which divides into 128 kHZ PWM frequency as the CCR0 values are set as 128. Thus CCR1
 *  and CCR2 values would reach 100% at 128 but as it would corrupt the PWM principles the maximum limit
 *  will be 125.
 *
 * 		At the moment a test startup mode for one panel (panel 4) is implemented. It starts PWM output
 * 	when panel 4 has a voltage that is 1.5 V more than what the battery has. Currently, due to a fault
 * 	in the hardware design, current isn't being charged to the battery and the PWM control development
 * 	is halted.
 *
 	   Part of: Charger project
 *  Created on: 31.8.2015
 *      Author: Teppo Uimonen
 */


#ifndef CHARGER_PWM_H_
#define CHARGER_PWM_H_


/****************************************************************************************************
 * 											HEADERS
 ****************************************************************************************************/


#include <stdint.h>


/****************************************************************************************************
 * 										   CONSTANTS
 ****************************************************************************************************/


/*
 * At the moment these two constants define charging state. TODO: In the future when more modes
 * are available it should be thaught to change it into an enum.
 */
#define WRONG_BATTERY_VOLTAGE  -1
#define START_UP				0


/****************************************************************************************************
 *         								GLOBAL FUNCTIONS
 ****************************************************************************************************/


inline int8_t PWM_UpdateControl(float * measResults);

#endif /* CHARGER_PWM_H_ */
