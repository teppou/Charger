/*
 * LCD.c
 *
 *		LCD module contains functionality to use an EA DOGL 128-6 screen. The screen
 * is connected to the charger system with an USCI module and two ports defining EA DOGL
 * specific functions. Also included in the source file is an 8p font made from a font
 * bitmap image by converting it with MATLAB into hexadecimal representation.
 *
 *
 * Source includes:
 * - constant parameters needed to initialize to screen and turn it on
 * - 8p font in char array hexadecimal representation
 * - device functions to send commands and pixel data to LCD
 * - a helper function to change to a specific row and column of LCD
 * - global functions for turning on LCD screen and updating the screen with text fields
 *
 *
 *	   Part of: Charger project
 *  Created on: 11.7.2015
 *      Author: Teppo Uimonen
 */


/****************************************************************************************************
 * 											HEADERS
 ****************************************************************************************************/


#include <msp430f2232.h>

#include "LCD.h"


/****************************************************************************************************
 * 										   CONSTANTS
 ****************************************************************************************************/


/* Width of a single char written to the screen */
const uint8_t CHARWIDTH = 5;


/*
 * LCD init and turn on array.
 */
const char LCD_INIT[] =  {   0x40,    /* Display start line 0				*/
                     	 	 0xA1,    /* ADC reverse						*/
							 0xC0,    /* COM0 - COM63						*/
							 0xA6,    /* Display normal						*/
							 0xA2,    /* Set Bias 1/9 (Duty 1/65)			*/
							 0x2F,    /* Booster, Regulator and Follower On */
							 0xF8,    /* Set internal Booster to 4x			*/
							 0x00,
							 0x27,    /* Contrast set						*/
							 0x81,
							 0x0F,
							 0xAC,    /* No indicator						*/
							 0x01,
							 0xAF,    /* Display on							*/	};


/*
 * Font with a height of 8 pixels.
 */
const char FONT_8P[] = {  0x7E, 0x09, 0x09, 0x09, 0x7E, /*  A	*/
					   	  0x7F, 0x49, 0x49, 0x49, 0x36, /*  B	*/
						  0x3E, 0x41, 0x41, 0x41, 0x22, /*  C	*/
						  0x7F, 0x41, 0x41, 0x41, 0x3E, /*  D	*/
						  0x7F, 0x49, 0x49, 0x49, 0x41,	/*  E	*/
						  0x7F,	0x09, 0x09, 0x09, 0x01,	/*  F	*/
						  0x3E, 0x41, 0x49, 0x49, 0x7A,	/*  G	*/
						  0x7F, 0x08, 0x08, 0x08, 0x7F,	/*  H	*/
						  0x00, 0x41, 0x7F, 0x41, 0x00,	/*  I	*/
						  0x20, 0x40, 0x41, 0x3F, 0x01,	/*  J	*/
						  0x7F, 0x08, 0x14, 0x22, 0x41, /*  K	*/
						  0x7F, 0x40, 0x40, 0x40, 0x40,	/*  L	*/
						  0x7F, 0x02, 0x0C, 0x02, 0x7F,	/*  M	*/
						  0x7F,	0x04, 0x08, 0x10, 0x7F,	/*  N	*/
						  0x3E, 0x41, 0x41, 0x41, 0x3E,	/*  O	*/
						  0x7F, 0x09, 0x09, 0x09, 0x06,	/*  P	*/
						  0x3E, 0x41, 0x51, 0x21, 0x5E,	/*  Q	*/
						  0x7F, 0x09, 0x19, 0x29, 0x46,	/*  R	*/
						  0x46, 0x49, 0x49, 0x49, 0x31,	/*  S	*/
						  0x01, 0x01, 0x7F, 0x01, 0x01,	/*  T	*/
						  0x3F, 0x40, 0x40, 0x40, 0x3F,	/*  U	*/
						  0x0F, 0x30, 0x40, 0x30, 0x0F,	/*  V	*/
						  0x3F, 0x40, 0x38, 0x40, 0x3F,	/*  W	*/
						  0x63, 0x14, 0x08, 0x14, 0x63,	/*  X	*/
						  0x07, 0x08, 0x70, 0x08, 0x07,	/*  Y	*/
						  0x61, 0x51, 0x49, 0x45, 0x43,	/*  Z	*/
						  0x20, 0x55, 0x54, 0x55, 0x78,	/*  ä	*/
						  0x38, 0x45, 0x44, 0x45, 0x38, /*  ö	*/
						  0x00, 0x60, 0x1C, 0x03, 0x00,	/*  /	*/
						  0x3E, 0x51, 0x49, 0x45, 0x3E,	/*  0	*/
						  0x00, 0x42, 0x7F, 0x40, 0x00,	/*  1	*/
						  0x42, 0x61, 0x51, 0x49, 0x46,	/*  2	*/
						  0x21, 0x41, 0x45, 0x4B, 0x31,	/*  3	*/
						  0x18, 0x14, 0x12, 0x7F, 0x10,	/*  4	*/
						  0x27, 0x45, 0x45, 0x45, 0x39,	/*  5	*/
						  0x3C, 0x4A, 0x49, 0x49, 0x30,	/*  6	*/
						  0x01, 0x71, 0x09, 0x05, 0x03,	/*  7	*/
						  0x36, 0x49, 0x49, 0x49, 0x36,	/*  8	*/
						  0x06, 0x49, 0x49, 0x29, 0x1E,	/*  9	*/
						  0x00, 0x24, 0x00, 0x00, 0x00, /*  :	*/
						  0x00, 0x22, 0x14, 0x08, 0x00,	/*  >	*/
						  0x00, 0x08, 0x14, 0x22, 0x00	/*  <	*/
};


/****************************************************************************************************
 * 											VARIABLES
 ****************************************************************************************************/


/* Buffer is used for fast transfer of data to LCD */
static			char	msgBuffer[128]	 = { 0 };
static 			uint8_t msgLength		 =   0;
static volatile uint8_t	msgIndex 		 =   0;
static volatile uint8_t	isTransferReady  =   1;


/****************************************************************************************************
 * 										STATIC FUNCTIONS
 ****************************************************************************************************/


/*
 * Sends commands to LCD.
 */
static void LCD_SendCommands(char * data, uint8_t length)
{
	uint8_t i = 0;

	/* Wait for previous transfer to be finished and then set a new one */
	while(0 == isTransferReady);

	isTransferReady = 0;

	/* Fill the buffer with data */
	while(i < length)
		msgBuffer[i] = data[i++];

	msgLength = length;

	/* Set down ports that define data transfer and command mode */
	P4OUT &= ~BIT0;
	P2OUT &= ~BIT5;

	/* Enable interrupts for transfer to work */
	UC0IE |= UCB0TXIE;
}


/*
 * Send data to LCD.
 */
static inline void LCD_SendData(uint8_t length)
{
	uint8_t i;

	/* Wait for previous transfer to be ready and then set a new one */
	while(0 == isTransferReady);

	isTransferReady = 0;

	msgLength = length;

	/* Set down the port that indicates data transfer start */
	P4OUT &= ~BIT0;

	for(i = 0; i < 10; i++);

	/* Enable interrupts for transfer to work */
	UC0IE |= UCB0TXIE;
}


/*
 * Set row and column where to draw.
 */
inline void LCD_SetRowColumn(uint8_t row, uint8_t column)
{
	char setArray[] = {	0xB0 + row,
						0x10,
						0x00 };

	if(column > 15)
		setArray[1] |= column >> 4;

	setArray[2] = 0x0F & column;

	LCD_SendCommands(setArray, 3);
}


/*
 * Initialize LCD and switch it on.
 */
inline void LCD_Initialize(void)
{
	LCD_SendCommands((char*)LCD_INIT, 15);
}


/*
 * Updates the screen with an array of text fields. As parametres it takes the pointer
 * to the first element of text field array and the number of text fields in the array.
 */
void LCD_UpdateScreen(T_TextField * pTextFields, uint8_t textFieldCount)
{
	uint8_t currentTextField;
	uint8_t y_end[15];

	/*
	 *  First the vertical direction ending points of each text field are counted as it
	 *  helps determining whether or not the text field should be written to each LCD page.
	 */
	for(currentTextField = 0; currentTextField < textFieldCount; currentTextField++)
	{
		y_end[currentTextField] = pTextFields->y + 8;
		pTextFields++;
	}

	uint8_t currentRow;
	uint8_t bufferPosition;
	uint8_t byteOfChar = 0;
	uint8_t charPosition = 0;
	uint8_t direction = 0;

	/*
	 * Loop through rows of LCD screen.
	 */
	for(currentRow = 0; currentRow < 8; currentRow++)
	{
		/* Always point to the first text field when starting to write a new row */
		pTextFields -= textFieldCount;

		/* Set the LCD point to the beginning of current row (page) */
		LCD_SetRowColumn(currentRow, 0);

		/* Wait for previous transfer to be completed and then clear the transfer data buffer */
		while(isTransferReady == 0);

		for(bufferPosition = 0; bufferPosition < 128; bufferPosition++)
			msgBuffer[bufferPosition] = 0x00;

		/* Loop through text fields */
		for(currentTextField = 0; currentTextField < textFieldCount; currentTextField++)
		{
			/* Direction tells if current text field belongs to the current row and which direction
			 * it should be translated for the correct position in the row. By default it's set as zero
			 * to tell the text field area doesn't cover current row and should not be drawn there */
			direction = 0;

			/* If top pixels of the text field belong to the current row */
			if((pTextFields->y >= currentRow*8) && (pTextFields->y <= ((currentRow + 1)*8 - 1)))
				direction = 1;

			/* If bottom pixels of the text field belong to the current row */
			else if(((y_end[currentTextField] + 8) >= currentRow*8) && (y_end[currentTextField] <= ((currentRow + 1)*8 - 1)))
				direction = 2;

			if(direction != 0)
			{
				/* Change buffer write position to the beginning of current text field */
				bufferPosition = pTextFields->x;

				/* Point to the first char of text field's char array */
				char * charInText = pTextFields->pText;

				/* While char array has chars in it */
				while(*charInText != '\0')
				{
					/* By default char's position in font table is 255 which is undefined and nothing will be written */
					charPosition = 255;

					/* Set char's position in font table by it's ASCII number */
					if((*charInText >= 65) && (*charInText <= 90))		/* Letter A-Z							*/
						charPosition = ((*charInText - 65)*5);
					else if((*charInText >= 47) && (*charInText <= 58))	/* Char '/', numbers 0-9 and char ':' 	*/
						charPosition = ((*charInText - 19)*5);
					else if(97 == *charInText)							/* Letter a translates into ä 			*/
						charPosition = 130;
					else if(111 == *charInText)							/* Letter o translates into ö			*/
						charPosition = 135;
					else if(62 == *charInText)							/* Char > 								*/
						charPosition = 200;
					else if(60 == *charInText)							/* Char <								*/
						charPosition = 205;
					else if((44 == *charInText) || (46 == *charInText))	/* , and . both translate to ,			*/
					{
						if(1 == direction)
							msgBuffer[bufferPosition] |= (0xC0 << (pTextFields->y - (currentRow*8)));
						else
							msgBuffer[bufferPosition] |= (0xC0 >> (8-(y_end[currentTextField] - (currentRow*8))));
						bufferPosition += 2;
					}
					else if(32 == *charInText)							/* Space								*/
						bufferPosition += CHARWIDTH + 1;

					/* If char has a position in font table then write it byte by byte to the correct x and y position */
					if(charPosition != 255)
					{
						for(byteOfChar = 0; byteOfChar < CHARWIDTH; byteOfChar++)
						{
							if(1 == direction)
								msgBuffer[bufferPosition + byteOfChar] |= (FONT_8P[charPosition + byteOfChar] << (pTextFields->y - (currentRow*8)));
							else
								msgBuffer[bufferPosition + byteOfChar] |= (FONT_8P[charPosition + byteOfChar] >> (8-(y_end[currentTextField] - (currentRow*8))));
						}

						bufferPosition += 6;
					}
					charInText++;
				}
			}
			pTextFields++;
		}

		/* Send 128 bytes of data in data buffer thus filling one row */
		LCD_SendData(128);
	}
}


/*
 * Interruption for USCI TX vector puts a new char from the data buffer to TX buffer
 * and stops data transfer and interrupts for USCI when the data has been sent.
 */
#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{
   	UCB0TXBUF = msgBuffer[msgIndex++];

   	if(msgIndex >= msgLength)
	{
   		UC0IE &= ~UCB0TXIE;

		uint8_t i;

        for(i = 0; i < 100; i++);

		P4OUT |= BIT0; 	/*  Unselect    Device					*/
		P2OUT |= BIT5;	/* 	Set data transfer mode as default	*/

		for(i = 0; i < 100; i++);

		msgIndex = 0;
		msgLength = 0;

		isTransferReady = 1;
	}
}
