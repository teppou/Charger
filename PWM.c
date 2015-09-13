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
 *	   Part of: Charger project
 *  Created on: 31.8.2015
 *      Author: Teppo Uimonen
 */


/****************************************************************************************************
 * 											HEADERS
 ****************************************************************************************************/


#include <msp430f2232.h>
#include <intrinsics.h>

#include "PWM.h"
#include "Common.h"


/****************************************************************************************************
 * 										GLOBAL FUNCTIONS
 ****************************************************************************************************/

/*
 * Updates PWM outputs for all panels according to measurement values of panels and battery.
 */
inline int8_t PWM_UpdateControl(float * measResults)
{
	static int8_t chargingState = WRONG_BATTERY_VOLTAGE;
		   int8_t controlValue  = 0;

	if((measResults[BATTERY_VOLTAGE] < 9.5f) || (measResults[BATTERY_VOLTAGE] >= 14.5f))
		chargingState = WRONG_BATTERY_VOLTAGE;

	else if(WRONG_BATTERY_VOLTAGE == chargingState)
	{
		chargingState = START_UP;
	}

	switch (chargingState)
	{
	case WRONG_BATTERY_VOLTAGE:

		/* If battery voltage is outside limits charging is off */
		TACCR1 = 0;
		TACCR2 = 0;
		TBCCR1 = 0;
		TBCCR2 = 0;
		break;

	case START_UP:

		if (measResults[PANEL_4_VOLTAGE] > (measResults[BATTERY_VOLTAGE] + 1.5f))
			controlValue = 128 * (measResults[BATTERY_VOLTAGE] / (measResults[PANEL_4_VOLTAGE] - 1.0f)) * 1.05f;
		else
			controlValue = 0;

		if(controlValue > 125)
			controlValue = 125;

		TBCCR2 = controlValue;

		break;
	}

	return chargingState;
}
