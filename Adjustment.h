/*
 * Adjustment.h
 *
 * Adjustment module's function is to maintain data that is needed to convert raw ADC measurement
 * values into usable current and voltage values. For conversion there is a coefficient and an
 * offset value for each measured channel. To do the conversion a specific channel's raw ADC
 * value is first multiplied with it's specific coefficient value and adding it's specific offset
 * value to the result.
 *
 * Adjustment module contains functionality to:
 * - initialize adjustment with raw "factory" values included in Adjustment.c
 * - adjust measurement channels with given calibration data
 * - save current adjustment data to FLASH memory
 * - read existing adjustment data from FLASH memory
 *
 * Adjustment header file includes global function declarations and inclusion of header Common.h
 * where are declared data structures needed for these functions. Also included is definition of
 * data type that defines measure information needed in Adjustment and Charger modules.
 *
 *    Part of: Charger project
 * Created on: 25.8.2015
 *     Author: Teppo Uimonen
 */


#ifndef CHARGER_ADJUSTMENT_H_
#define CHARGER_ADJUSTMENT_H_


/****************************************************************************************************
 *                                           HEADERS
 ****************************************************************************************************/


#include "Common.h"


/****************************************************************************************************
 *                                     DATA TYPE DEFINITIONS
 ****************************************************************************************************/


/*
 * Holds measurement information needed to save the ADC measurements and convert
 * them into current and voltage float values.
 */
typedef struct
{
    /* Raw ADC measurement values */
    unsigned int rawMeas[15];

    /* Measurement results after conversion to voltage and current units for 10 needed values */
    float        measResults[10];

    /* Adjustment coefficient values */
    float        adjustmentCoeff[10];

    /* Adjustment offset values */
    float        adjustmentOffset[10];
} T_MeasureInformation;


/****************************************************************************************************
 *                                        GLOBAL FUNCTIONS
 ****************************************************************************************************/

/* Writes current adjustment information to FLASH memory. */
inline void Adjustment_SaveAdjustmentToFlash(T_MeasureInformation * pMeasInfo);

/* Performs adjustment calculations with given calibration data */
inline void Adjustment_MakeAdjustment(T_MeasureInformation * pMeasInfo, T_CalibrationInfo * pCalibInfo);

/* Retrieves available adjustment configuration */
void Adjustment_GetCurrentAdjustment(T_MeasureInformation * pMeasInfo);

#endif /* CHARGER_ADJUSTMENT_H_ */
