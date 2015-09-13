/*
 * Charger.c
 *
 *
 * 		The charger module is in control of initializing the system and controlling the program
 * 	flow with a while(1) loop. In the loop ADC measurements and button states are read as
 * 	inputs and submodules are called with specific input parameters to control the state of
 * 	each subsystem. Also control of calibrating a measurement channel is included in Charger
 * 	module though adjustment has it's own module.
 *
 * 	Charger source file implements functionality to:
 * 	 - initialize used pins in Charger device
 *   - configure devices (clock, timers, USCI for LDC use, ADC10)
 *   - read inputs (ADC10 measurements and button state)
 *   - save calibration information for a measurement channel
 *   - control program flow
 *
 *	   Part of: Charger project
 *  Created on: 7.8.2015
 *      Author: Teppo Uimonen
 */


/****************************************************************************************************
 * 											HEADERS
 ****************************************************************************************************/


#include <msp430f2232.h>
#include <stdint.h>

#include "Charger.h"


/****************************************************************************************************
 * 											CONSTANTS
 ****************************************************************************************************/


/* Defines the threshold separating a short and a long click. Unit is the number of program cycles. */
#define SHORT_CLICK_THRESHOLD 20

/* Lookup table of different measurement values in the measurement array. In following order:
 * Panel 1 voltage, panel 1 current, panel 2 voltage, panel 2 current,
 * panel 3 voltage, panel 3 current, panel 4 voltage, panel 4 current,
 * battery voltage, battery current											 	*/
const static uint8_t MEAS_LOOKUP_TABLE[10] 	= { 14, 13, 12, 11, 10, 9, 8, 7, 2, 0 };


/****************************************************************************************************
 * 											FUNCTIONS
 ****************************************************************************************************/


/*
 * Initializes used pins of the charger project.
 */
inline static void Charger_InitializePins(void)
{
  	/* Configure pull-up resistors P1.0 and P4.7	*/
	P1REN |= BIT0;
	P1OUT |= BIT0;

	P4REN |= BIT7;
	P4OUT |= BIT7;

	/* Set specified 10 channels to measure voltage and current of battery and each four panels */
	ADC10AE0 |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5 + BIT6 + BIT7;	/* A0 - A7 		*/
	ADC10AE1 |= BIT4 + BIT5 + BIT6;										/* A12 - A14 	*/

	/* Set TA2 output for PWM 1 (1.3) and TA1 for PWM 2 (1.2) */
	P1DIR = BIT2 + BIT3;
	P1SEL = BIT2 + BIT3;

	/* Set TB1 output for PWM 3 (4.1) and TB2 for PWM 4 (4.2) and LCD power on with port (4.6) */
	P4DIR |= BIT1 + BIT2 + BIT6;
	P4SEL |= BIT1 + BIT2;

	/* Set XIN and XOUT for crystal */
	P2SEL = BIT6 + BIT7;

	/* USB0 SIMO, CLK, TXD and RXD for USCI use */
	P3SEL |= BIT1 + BIT3 + BIT4 + BIT5;

    /* 4.0 is Cs port for LCD protocol, setting it down tells data is sent. */
	P4DIR  |=  BIT0;
    P4OUT  |=  BIT0;

    /* 2.5 is A0 port for LCD protocol, it defines whether data (up) or command (down) is sent.
     * By default it's assumed data is sent and 2.5 is set up. */
    P2DIR |= BIT5;
	P2OUT |= BIT5;
}

/*
 * Initializes devices of the charger project.
 */
inline static void Charger_InitializeDevices(){

    /****************************************************************************************************
     *  								CLOCK SYSTEM CONFIGURATION
     * A 16 MHz external crystal sourced from ACLK is used by timers A and B to set the PWM output for
     * high frequency. ACLK with crystal output is also used by USCI module for LCD screen's data transfer.
     *
     * TODO: MCLK is sourced from DCO which also has 16 MHz. For some reason sourcing MCLK from external
     * crystal caused very unrealiable behaviour for system as it suddenly started stopping completely from
     * working. The actual cause should be examined more in-depth than so far has been done.
     ****************************************************************************************************/

    /* Reset DC0 and set it to 16 MHz */
    DCOCTL = 0;
    DCOCTL = CALDCO_16MHZ;

    BCSCTL1 = CALBC1_16MHZ + XTS;	/* 16 MHz and high-frequency settings				*/
	BCSCTL3 = LFXT1S_2;				/* Set 3 - 16 MHz crystal range						*/

	/* Wait for crystal to stabilize */
	while(IFG1 & OFIFG)
	{
		IFG1 &= ~OFIFG;
		_delay_cycles(100000);
	}

	BCSCTL2 = SELM_0; 				/* Select MCLK to source DCO */


    /****************************************************************************************************
     *  								TIMER CONFIGURATION
     * Timers A and B set PWM outputs for each four panels. Both of them source from ACLK taking
     * 16 MHz crystal clock signal. With CCR0 set to 128 they set PWM frequency to 128 kHz which
     * is fast enough for charging.
     ****************************************************************************************************/

	/* Timer_A sets outputs for PWM 1 and 2 */
	TACTL    = TACLR;   				/* Timer_A clear 									*/
	TACTL   |= TASSEL_1 + MC_1 + ID_0;	/* Select ACLK which is configured for 16 MHz crystal,
										   set continuous mode and divide with one 			*/
	TACCR0   = 128;						/* Set PWM frequency: 16 MHz / 128 = 128kHz			*/

	/* Initialize PWMs with output off and reset/set mode 				*/

	/* PWM 1 is connected to port 1.3 where TACCR2 output is located	*/
	TACCR2   = 0;
	TACCTL2  = OUTMOD_7;

	/* PWM 2 is connected to port 1.2 where TACCR1 output is located	*/
	TACCR1   = 0;
	TACCTL1  = OUTMOD_7;



	/* Timer_B sets outputs for PWM 3 and 4 */
	TBCTL   = TBCLR;   					/* Timer_B clear 									*/
	TBCTL  |= TBSSEL_1 + MC_1 + ID_0;	/*	Select ACLK which is configured for 16 MHz crystal,
	  										set continuous mode and divide with one 		*/
	TBCCR0  = 128;						/*	Set PWM frequency: 16 MHz / 128 = 128kHz 		*/

	/* Initialize CCR:s with 0 output and reset/set mode */

	/* PWM 3 is connected to port 4.1 where TBCCR1 output is located	*/
	 TBCCR1  = 0;
	 TBCCTL1 = OUTMOD_7;

	/* PWM 4 is connected to port 4.2 where TBCCR2 output is located	*/
	TBCCR2  = 0;
	TBCCTL2 = OUTMOD_7;


    /****************************************************************************************************
     *  								USCI CONFIGURATION
     * USCI is configured for sending commands and data to LCD screen. It takes crystal's 16 Mhz
     * signal and divides it to be used as data transfer protocol's clock signal.
     ****************************************************************************************************/

	UCB0CTL1 = UCSWRST;	/* USCI reset ON */

	UCB0CTL0 = UCSYNC + UCMSB + UCCKPL + UCMST;		/* Synchronous mode, MSB first,
	 	 	 	 	 	 	 	 	 	 	 	 	 inactive state high, master mode */

	UCB0CTL1 = UCSSEL_1; /* Select ACLK which is configured for 16 MHz crystal		  */

	/* Signal divided so that LCD screen is able to receive data */
	/* For 16MHz signal: 										 */
	UCB0BR0 = 0x21;
	UCB0BR1 = 0x00;

	UCB0CTL1 &= ~UCSWRST; /* USCI reset OFF */



	/****************************************************************************************************
	 * 									ADC10 CONFIGURATION
	 * Set ADC10 to perform conversions on 15 channels of which 10 are active.
	 * This is because ADC10 starts multiple channel conversion from the highest channel (14)
	 * and goes to the lowest channel one by one no matter if a specific channel is active or not.
	 ****************************************************************************************************/

	ADC10CTL0 &= ~ENC;	/* Disable conversion */

	/* Set highest active channel to 14, repeat sequence-of-channels conversion */
	ADC10CTL1 = INCH_14 + CONSEQ_3;

	/* Sample-and-hold time 16 ADC10LCKs, reference Vcc and Vss, ADC10 ON and multiple sample conversion */
	ADC10CTL0 = ADC10SHT_2 + SREF_0 + ADC10ON + MSC;

	ADC10DTC1 = 15; 				/* Number of channels to convert */

	/* Enable interrupts */
	_BIS_SR(GIE);

	/* LCD is turned on with LCD modules own initialization function */
	LCD_Initialize();
}

/*
 * Performs measurements on each 15 ADC channels and calculates current float values
 * for 10 wanted measurements.
 */
static void Charger_MeasureADC(T_MeasureInformation * pMeasInfo)
{

	/* Perform measurements and save results to corresponding meas value array 		*/
	ADC10CTL0 &= ~ENC;
	while (ADC10CTL1 & BUSY);
	ADC10SA = (unsigned int)pMeasInfo->rawMeas;
    ADC10CTL0 |= ENC + ADC10SC;

    /* Calculate average of each 10 wanted ADC measurements and save them as floats */
    uint8_t i;

    /* Loop through wanted ADC channels */
    for(i = 0; i < 10; i++)
    {
    	/* Calculate the average and convert using calibration coefficient and offset values corresponding to each channel 	 */
   		pMeasInfo->measResults[i] = ((float)pMeasInfo->rawMeas[MEAS_LOOKUP_TABLE[i]] * pMeasInfo->adjustmentCoeff[i]) + pMeasInfo->adjustmentOffset[i];

    	/* If value is close to zero it's possible that offset value decreases it below zero. Set value to 0 in case this happens */
    	if(pMeasInfo->measResults[i] < 0)
    		pMeasInfo->measResults[i] = 0;
    }

}


/*
 * Checks if button is pressed or not and returns if there's been a short click, long click or no action.
 */
static enum E_ButtonClicks Charger_IsButtonClicked()
{
    enum E_ButtonClicks isClicked = NO_CLICK;

	static uint8_t  buttonPressed  = 0;	/* Stores if the button was pressed or not. 0 = false, 1 = true */
    static uint16_t pressedCounter = 0;	/* Stores for how many cycles the button has been pressed 		*/

    /* Check if button is pressed */
    if(!(P3IN & BIT2))
    {
    	/* Check if this is the first time button is pressed or if it is held pressed */
    	if(0 == buttonPressed)
    		buttonPressed++;
    	else
    	{
    		/* If button has been held pressed long enough the click is defined a long click */
    		if(pressedCounter == SHORT_CLICK_THRESHOLD)
    			isClicked = LONG_CLICK;

    		pressedCounter++;
    	}
    }

    /* If button is not pressed but it was previously pressed then it has been released.
     * In case of a quick release the click is defined as a short click     			*/
    else if(1 == buttonPressed)
    {
    	if (pressedCounter < SHORT_CLICK_THRESHOLD)
    		isClicked = SHORT_CLICK;

    	buttonPressed = 0;
    	pressedCounter = 0;
    }

    return isClicked;
}


/*
 * First initializes devices and variables, then controls the overall flow of the program
 * with a while loop where inputs are read and the input results are then handled in
 * specific program areas.
 */
void main(void)
{
	/*											DEVICE CONFIGURATION														*/

	/* Stop watchdog timer */
    WDTCTL = WDTPW + WDTHOLD;

    /* Initialize pins and devices */
    Charger_InitializePins();
    Charger_InitializeDevices();

    /*										INITIALIZATION OF USED VARIABLES												*/

    /* measInfo saves three past measurements of all 15 ADC channels and for used 10 channels it also saves their averages,
     * calibration coefficients and offsets.																				*/
   T_MeasureInformation measInfo = { 0 };

    /* Gets current calibration info by first setting the "factory" values and then checking if new calibration data is found in FLASH. */
   	Adjustment_GetCurrentAdjustment(&measInfo);

	/* Initialize menu system with menuScreens */
    T_MenuSystem 		menu		= { NO_MENU, 0, NO_MENU, MENU_VIEWS, 0, 0 };

    /* Holds calibration values when calibrating an ADC channel */
    T_CalibrationInfo 	calib 		= { 0, { 0, 0 } };

    enum E_ButtonClicks buttonClick = NO_CLICK; /* Button state */

    int8_t  menuAction    = -1; /* Action to perform defined by menu module    */
    int8_t  chargingState = -1; /* Will be used when PWM module is implemented */
    uint8_t LCDinit       =  0; /* LCD init counter							   */
    uint8_t cycleDelay    =  0;

    /* 												MAIN LOOP																*/
    while(1){

    	/* Read inputs and perform submodule tasks with results */
    	Charger_MeasureADC(&measInfo);
    	chargingState = PWM_UpdateControl(measInfo.measResults);

    	buttonClick   = Charger_IsButtonClicked();
    	menuAction    = Menu_UpdateView(&menu, buttonClick, measInfo.measResults, &calib);

    	/* Perform action given by menu module */
    	switch(menuAction)
    	{
    	case MENU_NO_ACTION:
			break;

    	case MENU_MEASURE_1:

    		/* Save given measurement's raw measurement data at the first calibration point */
			calib.calibResults[0] = measInfo.rawMeas[MEAS_LOOKUP_TABLE[calib.measToCalibrate]];
    		break;

    	case MENU_MEASURE_2:

    		/* Save given measurement's raw measurement data at the second calibration point
    		 * and perform adjustment														*/
			calib.calibResults[1] = measInfo.rawMeas[MEAS_LOOKUP_TABLE[calib.measToCalibrate]];
			Adjustment_MakeAdjustment(&measInfo, &calib);
			break;

    	case MENU_SAVE:
    		Adjustment_SaveAdjustmentToFlash(&measInfo);
			break;

    	case MENU_CANCEL:

    		/* In case of cancel reload previous adjustment data from factory defaults and FLASH */
    		Adjustment_GetCurrentAdjustment(&measInfo);
			break;

    	default:

    		/* Numbers 0-9 get here indicating which measurement will be calibrated and adjusted */
			calib.measToCalibrate = menuAction;
			break;
    	}

    	/* Update LCD screen */
    	LCD_UpdateScreen(menu.currentTextFields, menu.views[menu.menuState].textFieldCount);

    	/*
    	 * TODO: Sometimes the LCD screen has shut down unexpectedly and initializing it again
    	 * every now and then seems to prevent it. This behaviour should be given a deeper
    	 * investagion.
    	 */
    	LCDinit++;

 		if(LCDinit > 100)
 		{
 			LCD_Initialize();
 			LCDinit = 0;
 		}

		for (cycleDelay = 0; cycleDelay < 50000; cycleDelay++);
    }
}

