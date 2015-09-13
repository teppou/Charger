/*
 * Charger.h
 *
 *
 * 		The charger module is in control of initializing the system and controlling the program
 * 	flow with a while(1) loop. In the loop ADC measurements and button states are read as
 * 	inputs and submodules are called with specific input parameters to control the state of
 * 	each subsystem. Also control of calibrating a measurement channel is included in Charger
 * 	module though adjustment has it's own module.
 *
 * 		Header file includes necessary submodule headers and Common.h for common data types.
 *
 *	   Part of: Charger project
 *  Created on: 7.8.2015
 *      Author: Teppo Uimonen
 */


#ifndef CHARGER_CHARGER_H_
#define CHARGER_CHARGER_H_


/****************************************************************************************************
 * 											HEADERS
 ****************************************************************************************************/

#include "Adjustment.h"
#include "Common.h"
#include "PWM.h"
#include "LCD.h"
#include "Menu.h"

#endif /* CHARGER_CHARGER_H_ */
