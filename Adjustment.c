/*
 * Adjustment.c
 *
 * Adjustment module's function is to maintain data that is needed to convert raw ADC measurement
 * values into usable current and voltage values. For conversion there is a coefficient and an
 * offset value for each measured channel. To do the conversion a specific channel's raw ADC
 * value is first multiplied with it's specific coefficient value and adding it's specific offset
 * value to the result.
 *
 * Adjustment module contains functionality to:
 * - initialize adjustment with raw "factory" values
 * - adjust measurement channels with given calibration data
 * - save current adjustment data to FLASH memory
 * - read existing adjustment data from FLASH memory
 *
 * This file includes:
 * - functionality declared above
 * - FLASH addresses to store adjustment data to
 * - factory values to initialize adjustment with
 * - functionality to read floats from FLASH and to write them to FLASH
 *
 *    Part of: Charger project
 * Created on: 25.8.2015
 *     Author: Teppo Uimonen
 */


/****************************************************************************************************
 *                                         HEADERS
 ****************************************************************************************************/


#include <msp430f2232.h>
#include <stdint.h>

#include "Adjustment.h"


/****************************************************************************************************
 *                                        CONSTANTS
 ****************************************************************************************************/


/* FLASH addresses to store adjustment data */
#define CONVERSION_COEFFICIENT_ADDRESS 0x01000
#define CONVERSION_OFFSET_ADDRESS      0x01040

/*
 * The adjustment values are byte by byte so they can be read from an adjusted
 * device's memory easily to the computer and to the programming environment.
 */
#define PANEL_1_VOLTAGE_COEFF  { 0x3F, 0xE9, 0x13, 0x3D }
#define PANEL_1_VOLTAGE_OFFSET { 0x00, 0xEF, 0x6E, 0xBD }

#define PANEL_1_CURRENT_COEFF  { 0x00, 0x00, 0x00, 0x00 }
#define PANEL_1_CURRENT_OFFSET { 0x00, 0x00, 0x00, 0x00 }

#define PANEL_2_VOLTAGE_COEFF  { 0x3F, 0xE9, 0x13, 0x3D }
#define PANEL_2_VOLTAGE_OFFSET { 0x00, 0xEF, 0x6E, 0xBD }

#define PANEL_2_CURRENT_COEFF  { 0x00, 0x00, 0x00, 0x00 }
#define PANEL_2_CURRENT_OFFSET { 0x00, 0x00, 0x00, 0x00 }

#define PANEL_3_VOLTAGE_COEFF  { 0x3F, 0xE9, 0x13, 0x3D }
#define PANEL_3_VOLTAGE_OFFSET { 0x00, 0xEF, 0x6E, 0xBD }

#define PANEL_3_CURRENT_COEFF  { 0x00, 0x00, 0x00, 0x00 }
#define PANEL_3_CURRENT_OFFSET { 0x00, 0x00, 0x00, 0x00 }

#define PANEL_4_VOLTAGE_COEFF  { 0x3F, 0xE9, 0x13, 0x3D }
#define PANEL_4_VOLTAGE_OFFSET { 0x00, 0xEF, 0x6E, 0xBD }

#define PANEL_4_CURRENT_COEFF  { 0x00, 0x00, 0x00, 0x00 }
#define PANEL_4_CURRENT_OFFSET { 0x00, 0x00, 0x00, 0x00 }

#define BATTERY_VOLTAGE_COEFF  { 0x3F, 0xE9, 0xB2, 0x3C }
#define BATTERY_VOLTAGE_OFFSET { 0x00, 0x00, 0x00, 0x00 }

#define BATTERY_CURRENT_COEFF  { 0x00, 0x00, 0x00, 0x00 }
#define BATTERY_CURRENT_OFFSET { 0x00, 0x00, 0x00, 0x00 }

/* The "factory" default tables for adjustment data            */
const char ADJUSTMENT_COEFFICIENTS[10][4] = { PANEL_1_VOLTAGE_COEFF, PANEL_1_CURRENT_COEFF,
                                              PANEL_2_VOLTAGE_COEFF, PANEL_2_CURRENT_COEFF,
                                              PANEL_3_VOLTAGE_COEFF, PANEL_3_CURRENT_COEFF,
                                              PANEL_4_VOLTAGE_COEFF, PANEL_4_CURRENT_COEFF,
                                              BATTERY_VOLTAGE_COEFF, BATTERY_CURRENT_COEFF };

const char ADJUSTMENT_OFFSETS[10][4]      = { PANEL_1_VOLTAGE_OFFSET, PANEL_1_CURRENT_OFFSET,
                                              PANEL_2_VOLTAGE_OFFSET, PANEL_2_CURRENT_OFFSET,
                                              PANEL_3_VOLTAGE_OFFSET, PANEL_3_CURRENT_OFFSET,
                                              PANEL_4_VOLTAGE_OFFSET, PANEL_4_CURRENT_OFFSET,
                                              BATTERY_VOLTAGE_OFFSET, BATTERY_CURRENT_OFFSET };


/****************************************************************************************************
 *                                      STATIC FUNCTIONS
 ****************************************************************************************************/


/*
 * Writes a specified number of floats to a specific FLASH address.
 */
static void Adjustment_WriteToFlash(float * pFloats, char * pFlash, uint8_t floatCount)
{
    /* Disable interrupts while writing to FLASH */
    _BIC_SR(GIE);

    /* Convert the float pointer into a char pointer so the data can be written char by char to FLASH */
    char * pCharFloats = (char *)pFloats;

    FCTL2 = FWKEY + FSSEL_0 + (FN5 + FN4); /* Use ACLK and divide with 6         */
    FCTL1 = FWKEY + ERASE;                 /* Set erase                          */
    FCTL3 = FWKEY;                         /* Clear lock                         */
    *pCharFloats = 0x00;                   /* Dummy write to erase FLASH segment */

    while(FCTL3 & BUSY);

    FCTL1 = FWKEY + WRT;                   /* Change to write mode               */

    uint8_t i = 0;

    /* Write byte by byte to FLASH  */
    for(i = 0; i < (floatCount * 4); i++)
    {
        *pFlash++ = *pCharFloats++;
        while((FCTL3 & BUSY));
    }

    FCTL1 = FWKEY;
    FCTL3 = FWKEY + LOCK;                    /* Set lock */

    /* Enable interrupts after writing to FLASH */
    _BIS_SR(GIE);
}


/*
 * Reads specified number of floats from a specific FLASH address to a float array.
 */
static void Adjustment_ReadFloatsFromFlash(float * pFloats, char * pFlash, uint8_t floatCount)
{
    uint8_t i = 0;
    uint8_t j = 0;

    float  result  = 0;
    char * pResult = (char *)&result;

    /* Loop through number of floats and on each float go through it's four bytes
     * to assign the value to result which is then assigned to the float array. */
    for(i = 0; i < floatCount; i++)
    {
        /* If flash address contains other than 0xFF in the first byte it's
         * considered a valid value.                                      */
        if(*pFlash != 0xFF)
        {
            for(j = 0; j < 4; j++)
                pResult[j] = *pFlash++;

            pFloats[i] = result;
            pResult    = (char *)&result;
        }
        else
            pFlash += 4;
    }
}


/*
 * Reads existing adjustment information from FLASH memory.
 */
static inline void Adjustment_ReadAdjustmentFromFlash(T_MeasureInformation * pMeasInfo)
{
    Adjustment_ReadFloatsFromFlash(&pMeasInfo->adjustmentCoeff[0],  (char *)CONVERSION_COEFFICIENT_ADDRESS, 10);
    Adjustment_ReadFloatsFromFlash(&pMeasInfo->adjustmentOffset[0], (char *)CONVERSION_OFFSET_ADDRESS,      10);
}


/****************************************************************************************************
 *                                          GLOBAL FUNCTIONS
 ****************************************************************************************************/


/*
 * Writes current adjustment information to FLASH memory.
 */
inline void Adjustment_SaveAdjustmentToFlash(T_MeasureInformation * pMeasInfo)
{
    Adjustment_WriteToFlash(&pMeasInfo->adjustmentCoeff[0],  (char *)CONVERSION_COEFFICIENT_ADDRESS, 10);
    Adjustment_WriteToFlash(&pMeasInfo->adjustmentOffset[0], (char *)CONVERSION_OFFSET_ADDRESS,      10);
}


/*
 * Performs adjustment calculations and sets the results into use.
 */
inline void Adjustment_MakeAdjustment(T_MeasureInformation * pMeasInfo, T_CalibrationInfo * pCalibInfo)
{
    /* To determine if voltage or current is being calibrated */
    uint8_t type = 1;

    if(0 == (pCalibInfo->measToCalibrate % 2))
        type = 0;

    /* Calculate coefficient by taking the difference between calibration points and dividing it by the difference of measured points */
    pMeasInfo->adjustmentCoeff[pCalibInfo->measToCalibrate] = (CALIBRATION_POINTS[type][1] - CALIBRATION_POINTS[type][0])/((float)(pCalibInfo->calibResults[1] - pCalibInfo->calibResults[0]));

    /* Calculate offset by finding where the line would intersect with zero voltage/current */
    pMeasInfo->adjustmentOffset[pCalibInfo->measToCalibrate] = CALIBRATION_POINTS[type][0] - (pMeasInfo->adjustmentCoeff[pCalibInfo->measToCalibrate] * ((float)pCalibInfo->calibResults[0]));
}


/*
 * Retrieves the current calibration by first reading the original "factory" values
 * and then updating them by reading values from FLASH.
 */
void Adjustment_GetCurrentAdjustment(T_MeasureInformation * pMeasInfo)
{
    char * currentByte;
    char * readByte;

    currentByte = (char *)pMeasInfo->adjustmentCoeff;
    readByte    = (char *)ADJUSTMENT_COEFFICIENTS;

    uint8_t i = 0;

    for(i = 0; i < 40; i++)
        *currentByte++ = *readByte++;

    currentByte = (char *)pMeasInfo->adjustmentOffset;
    readByte    = (char *)ADJUSTMENT_OFFSETS;

    for(i = 0; i < 40; i++)
        *currentByte++ = *readByte++;

    Adjustment_ReadAdjustmentFromFlash(pMeasInfo);
}
