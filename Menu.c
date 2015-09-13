/*
 * Menu.h
 *
 * Menu module controls the information displayed on the screen and defines multiple views
 * that can be shown to the user. During program execution cycle the first thing that
 * happens in menu module is to define what happens when a button is clicked. This leads
 * either to changing current selection on the screen or changing the current view, which
 * again can lead to the main Charger module performing a specific task.
 *
 * Text on the screen is defined with text fields that contain the char array address and
 * the x and y coordinates of a single text field. A single menu view is defined with an
 * array of text fields and the amount of text fields in it.
 *
 * When current menu view is changed to another the menu system changes it's current text
 * fields according to new view's fields. Because some text fields are updatable, such as
 * the ones showing newest measurement results and the ones indicating selection state,
 * given char array pointers (the ones pointing to 0) are put to point menu system
 * structure's pre-allocated char arrays that can be updated according to specific menu
 * view.
 *
 * Source includes functionality to:
 * - switch from one menu view to another
 * - determine tasks to perform in menu and in the main module when a button is clicked
 * - write floats and char arrays to char tables (separate helper functions)
 * - initialize calibration view according to measurement to be calibrated
 * - update a specific view's text fields to match with newest measurements and selections
 *
 *    Part of: Charger project
 * Created on: 29.7.2015
 *     Author: Teppo Uimonen
 */


/****************************************************************************************************
 *                                             HEADERS
 ****************************************************************************************************/


#include "Menu.h"


/****************************************************************************************************
 *                                         STATIC FUNCTIONS
 ****************************************************************************************************/


/*
 * Updates current textfields similar to newly chosen view's text fields.
 */
static void Menu_ChangeView(T_MenuSystem * pMenu)
{
    uint8_t i;
    uint8_t j = 0;

    for(i = 0; i < pMenu->views[pMenu->menuState].textFieldCount; i++)
    {
        pMenu->currentTextFields[i].pText = pMenu->views[pMenu->menuState].textFields[i].pText;
        pMenu->currentTextFields[i].x = pMenu->views[pMenu->menuState].textFields[i].x;
        pMenu->currentTextFields[i].y = pMenu->views[pMenu->menuState].textFields[i].y;

        /* In case textfield's char array is undefined it's supposed to have a modifiable valuable.
         * It will be appointed an address of menuSystems's char table array that are modified in
         * Menu_UpdateTextFields according to current menu view.                                    */
        if(UPDATABLE_DATA == pMenu->currentTextFields[i].pText)
            pMenu->currentTextFields[i].pText = pMenu->updatableCharTables[j++];
    }
}


/*
 * Performs primary action according to current menu state. Basically this is the "select" action.
 */
static inline uint8_t Menu_PrimaryAction(T_MenuSystem * pMenu)
{
    switch (pMenu->menuState)
    {

    case PANEL_VIEW:

        /* Panel and battery views switching with primary action */

        pMenu->menuState = BATTERY_VIEW;
        Menu_ChangeView(pMenu);

        break;

    case BATTERY_VIEW:

        /* Panel and battery views switching with primary action */

        pMenu->menuState = PANEL_VIEW;
        Menu_ChangeView(pMenu);

        break;

    case MENU_VIEW_1:
    case MENU_VIEW_2:

        /* Menu screen is changed when selection goes "over the screen". */

        pMenu->currentSelection++;

        if (pMenu->currentSelection > 3)
        {
            pMenu->menuState++;
            pMenu->currentSelection = 0;
            Menu_ChangeView(pMenu);
        }

        break;

    case MENU_VIEW_3:

        /* Menu view is changed when the selection goes "over the screen". */

        pMenu->currentSelection++;

        if (pMenu->currentSelection > 3)
        {
            pMenu->menuState = MENU_VIEW_1;
            pMenu->currentSelection = 0;
            Menu_ChangeView(pMenu);
        }

        break;

    case CALIBRATION_VIEW_1:
    case CALIBRATION_VIEW_2:

        /* Calibration views only change the selection */

        pMenu->currentSelection++;

        if (pMenu->currentSelection > 1)
            pMenu->currentSelection = 0;

        break;

    }

    /* There is never a task for the main program after primary action so always return no action */
    return MENU_NO_ACTION;
}


/*
 * Performs secondary action according to current menu state. This is the "perform a task" action.
 */
static inline uint8_t Menu_SecondaryAction(T_MenuSystem * pMenu)
{
    /* By default return no task for the main program */
    uint8_t result = MENU_NO_ACTION;

    pMenu->previousMenu = pMenu->menuState;

    switch(pMenu->menuState)
    {

    case PANEL_VIEW:
    case BATTERY_VIEW:

        /* Panel and battery views change into the menu view */

        pMenu->menuState = MENU_VIEW_1;
        pMenu->currentSelection = 0;

        break;

    case MENU_VIEW_1:
    case MENU_VIEW_2:
    case MENU_VIEW_3:

        /* Current menu view and selection are calculated into a number that defines
         * a task for the main program. Menus 1-2 selections 0-3 and menu 3 selections
         * 0-1 become 0-9 which defines a calibration and adjustment to be made. 10 is
         * selection 2 in menu 3 and means save while selection 4 in menu is 11 and
         * means cancel.                                                             */

        result = ((pMenu->menuState - 2) * 4) + pMenu->currentSelection;
        pMenu->currentSelection = 0;

        switch(result)
        {
            case 10:
            case 11:
                pMenu->menuState = PANEL_VIEW;
                break;

            default:
                pMenu->menuState = CALIBRATION_VIEW_1;
                break;
        }

        break;

    case CALIBRATION_VIEW_1:

        /* Selection 0 in calibration views is cancel/back while 1 defines a
         * calibration value should be measured. */

        if(0 == pMenu->currentSelection)
            pMenu->menuState = MENU_VIEW_1;
        else
        {
            pMenu->menuState = CALIBRATION_VIEW_2;
            result = MENU_MEASURE_1;
        }

        break;

    case CALIBRATION_VIEW_2:

        /* Selection 0 in calibration views is cancel/back while 1 defines a
         * calibration value should be measured. */

        if(0 == pMenu->currentSelection)
            pMenu->menuState = CALIBRATION_VIEW_1;
        else
        {
            pMenu->menuState = MENU_VIEW_1;
            result = MENU_MEASURE_2;
        }

        break;
    }

    /* Update menu view as it always changes after secondary action */
    Menu_ChangeView(pMenu);

    return result;
}


/*
 * Handles what function to be taken with current state of the button.
 */
static uint8_t Menu_HandleButtonState(T_MenuSystem * pMenu, uint8_t buttonState)
{
    switch(buttonState)
    {
    case SHORT_CLICK:
        return Menu_PrimaryAction(pMenu);

    case LONG_CLICK:
        return Menu_SecondaryAction(pMenu);

    case NO_CLICK:
    default:
        return MENU_NO_ACTION;
    }
}


/*
 * Writes char array's value to a table of chars. Used for writing constants
 * to menuSystem's updatable char arrays.
 */
static void Menu_CharArrayToTable(char * array, char * table)
{
    char i = 0;
    while(*array != '\0')
        table[i++] = *array++;
    table[i] = '\0';
}


/*
 * Writes a float value as two decimal number into an EIGHT BYTES CHAR ARRAY.
 * This function is only used for writing to T_MenuSystems's updatable char arrays
 * which are defined as eight chars long.
 */
static void Menu_FloatToCharArray(char * charArray, float * value)
{
    /* Convert value into unsigned int after multiplying it with 100 for decimals */
    unsigned int newValue = (unsigned int)((*value) * 100);

    int8_t i = 0;

    /* Set decimals into their places */
    charArray[5] = (newValue % 10) + '0';
    newValue /= 10;
    charArray[4] = (newValue % 10) + '0';
    newValue /= 10;

    charArray[3] = ',';

    i = 2;

    if(0 == newValue)
    {
        charArray[i] = '0';
        i--;
    }
    else
    {
        /* Continue dividing with 10 and putting numbers into char array until value reaches 0 */
        while(newValue > 0)
        {
            charArray[i] = (newValue % 10) + '0';
            newValue /= 10;
            i--;
        }
    }

    /* Put empty spaces to the remaining chars */
    while(i >= 0)
        charArray[i--] = ' ';
}


/*
 * Updates the calibration view's text fields according to calibration state.
 */
static inline void Menu_SetCalibrationView(T_MenuSystem * pMenu, T_CalibrationInfo * pCalibInfo)
{
    /* Change the text fields that tell whether it's a battery or a certain panel that's being calibrated */
    Menu_CharArrayToTable("       ", pMenu->updatableCharTables[1]);

    if(pCalibInfo->measToCalibrate > 7)
        Menu_CharArrayToTable("  AKKU ", pMenu->updatableCharTables[0]);
    else
    {
        Menu_CharArrayToTable("PANEELI", pMenu->updatableCharTables[0]);
        pMenu->updatableCharTables[1][0] = ((pCalibInfo->measToCalibrate + 2)/2) + '0';
    }

    /* Change the text fields that tell of the calibration point and the calibration state */
    char *  quantity         = "VIRTA";
    char *  state            = "1/2    ";
    char    unit             = 'A';
    uint8_t calibrationState = 0;
    uint8_t calibUnit        = 1;

    if(CALIBRATION_VIEW_2 == pMenu->menuState)
    {
        calibrationState = 1;
        state            = "2/2    ";
    }

    if(0 == (pCalibInfo->measToCalibrate % 2))
    {
        quantity  = "JaNNITE";
        unit      = 'V';
        calibUnit = 0;
    }

    Menu_CharArrayToTable(quantity, pMenu->updatableCharTables[2]);
    Menu_CharArrayToTable(state, pMenu->updatableCharTables[3]);
    Menu_FloatToCharArray(pMenu->updatableCharTables[4], &CALIBRATION_POINTS[calibUnit][calibrationState]);
    pMenu->updatableCharTables[5][6] = unit;
    pMenu->updatableCharTables[7][6] = unit;
}


/*
 * Updates the contents of current menu view's textfields to match with newest measurements and selection. *
 *
 * TODO: Content that changes when menus views changes such as measurement units are also updated here. To
 * avoid changing text pieces that don't need to be changed again these parts could be moved to Menu_ChangeView.
 */
void Menu_UpdateTextFields(T_MenuSystem * pMenu, float * pMeasResults, T_CalibrationInfo * pCalibInfo)
{
    if(NO_MENU == pMenu->menuState)
    {
        pMenu->menuState = PANEL_VIEW;
        Menu_ChangeView(pMenu);
    }

    /* If calibration was chosen call the function to update fields according to calibration state */
    if(((5 == pMenu->menuState) || (6 == pMenu->menuState)) && (pMenu->menuState != pMenu->previousMenu))
        Menu_SetCalibrationView(pMenu, pCalibInfo);

    uint8_t i = 0;

    switch(pMenu->menuState)
    {
    case PANEL_VIEW:

        /* In panel view update all eight panel measurements */

        for(i = 0; i < 8; i++)
        {
            Menu_FloatToCharArray(pMenu->updatableCharTables[i], &pMeasResults[i]);
            if(0 == (i % 2))
                pMenu->updatableCharTables[i][6] = 'V';
            else
                pMenu->updatableCharTables[i][6] = 'A';
        }

         break;

    case BATTERY_VIEW:

        /* In battery view update battery's current and voltage */

        Menu_FloatToCharArray(pMenu->updatableCharTables[0], &pMeasResults[BATTERY_VOLTAGE]);
        Menu_FloatToCharArray(pMenu->updatableCharTables[1], &pMeasResults[BATTERY_CURRENT]);
        pMenu->updatableCharTables[0][6] = 'V';
        pMenu->updatableCharTables[1][6] = 'A';

         break;

    case MENU_VIEW_1:
    case MENU_VIEW_2:
    case MENU_VIEW_3:

        /* In menu view update the selection mark place to match with the current selection state */

        for(i = 0; i < 4; i++)
            Menu_CharArrayToTable("       ", pMenu->updatableCharTables[i]);

        pMenu->updatableCharTables[pMenu->currentSelection][0] = '>';

         break;

    case CALIBRATION_VIEW_1:
    case CALIBRATION_VIEW_2:

        /* In calibration views update the measurement of the calibrated quantity and
         * the selection mark to match with the current selection state */

        if(0 == pMenu->currentSelection)
        {
            Menu_CharArrayToTable(">      ", pMenu->updatableCharTables[5]);
            Menu_CharArrayToTable("<      ", pMenu->updatableCharTables[6]);
        }
        else
        {
            Menu_CharArrayToTable("       ", pMenu->updatableCharTables[5]);
            Menu_CharArrayToTable("   >  <", pMenu->updatableCharTables[6]);
        }

        Menu_FloatToCharArray(pMenu->updatableCharTables[7], &pMeasResults[pCalibInfo->measToCalibrate]);

         break;

    }
}


/****************************************************************************************************
 *                                         GLOBAL FUNCTIONS
 ****************************************************************************************************/


/*
 *  Updates menu view with given information and returns a task for main module to perform
 */
inline uint8_t Menu_UpdateView(T_MenuSystem * pMenu, uint8_t buttonState, float * pMeasResults, T_CalibrationInfo * pCalibInfo)
{
    uint8_t menuAction = Menu_HandleButtonState(pMenu, buttonState);

    Menu_UpdateTextFields(pMenu, pMeasResults, pCalibInfo);

    return menuAction;
}

