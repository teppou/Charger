/*
 * LCD.h
 *
 *		LCD module contains functionality to use an EA DOGL 128-6 screen. The screen
 * is connected to the charger system with an USCI module and two ports defining EA DOGL
 * specific functions. Also included in the source file is an 8p font made from a font
 * bitmap image by converting it with MATLAB into hexadecimal representation.
 *
 *
 * Header includes:
 * - necessary includes for uint8_t and text field data structures
 * - global functions declarations for turning on LCD screen and updating the screen
 *
 *
 *	   Part of: Charger project
 *  Created on: 11.7.2015
 *      Author: Teppo Uimonen
 */


#ifndef CHARGER_LCD_H_
#define CHARGER_LCD_H_


/****************************************************************************************************
 * 											HEADERS
 ****************************************************************************************************/


#include <stdint.h>

#include "Common.h"


/****************************************************************************************************
 * 										GLOBAL FUNCTIONS
 ****************************************************************************************************/


/*
 * Draws the text field array's text fields to the screen.
 */
void LCD_UpdateScreen(T_TextField * pTextFields, uint8_t textFieldCount);


/*
 * Initializes and turns on the LCD screen.
 */
void LCD_Initialize(void);


#endif /* CHARGER_LCD_H_ */
