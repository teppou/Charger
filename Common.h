/*
 * Common.h
 *
 * Common header file includes constants and data type declarations needed in multiple
 * submodules of main module Charger. This way submodules don't need to reference each
 * other.
 *
 * Includes:
 * - number representation of measured variables (Menu + PWM)
 * - calibration point definitions               (Menu + Adjustment)
 * - calibration info data type                  (Adjustment + Charger + Menu)
 * - text field data type                        (Menu + LCD)
 *
 *       Part of: Charger project
 *  Created on: 5.9.2015
 *      Author: Teppo Uimonen
 */


#ifndef CHARGER_COMMON_H_
#define CHARGER_COMMON_H_


/****************************************************************************************************
 *                                      CONSTANT DEFINITIONS
 ****************************************************************************************************/


/*
 * Number presentation of measured variables used to reference them in results array
 * and in menu to define which channel should be calibrated and adjusted.
 */
#define PANEL_1_VOLTAGE    0
#define PANEL_1_CURRENT    1

#define PANEL_2_VOLTAGE    2
#define PANEL_2_CURRENT    3

#define PANEL_3_VOLTAGE    4
#define PANEL_3_CURRENT    5

#define PANEL_4_VOLTAGE    6
#define PANEL_4_CURRENT    7

#define BATTERY_VOLTAGE    8
#define BATTERY_CURRENT    9

/*
 * Calibration points definition.
 */
const static float CALIBRATION_POINTS[2][2] = { { 2.0f, 15.0f },    /* Voltage calibration points 1 and 2 (V) */
                                                { 1.0f,  5.0f } };  /* Current calibration points 1 and 2 (A) */


/****************************************************************************************************
 *                                      DATA TYPE DEFINITIONS
 ****************************************************************************************************/


/*
 * Holds info needed to do a calibration for a single measurement channel.
 */
typedef struct
{
    /* Defines which measurement will be calibrated         */
    uint8_t measToCalibrate;

    /* Saves calibration measurement results for two points */
    float   calibResults[2];
} T_CalibrationInfo;


/*
 * Defines a text field that contains it's x and y position plus a char array pointer.
 */
typedef struct
{
   char *  pText;
   uint8_t x;
   uint8_t y;
} T_TextField;

#endif /* CHARGER_COMMON_H_ */
