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
 * Header includes:
 * - definitions used in Menu and Charger modules to define the interaction between them
 * - initialization of menu views and text fields for Charger project
 *
 *    Part of: Charger project
 * Created on: 29.7.2015
 *     Author: Teppo Uimonen
 */


#ifndef CHARGER_MENU_H_
#define CHARGER_MENU_H_

/****************************************************************************************************
 *                                             HEADERS
 ****************************************************************************************************/

#include <stdint.h>

#include "Common.h"


/****************************************************************************************************
 *                                           DEFINITIONS
 ****************************************************************************************************/


/*
 * Actions that are selected from menu system and passed on to the main program.
 * Numbers 0-9 define a specific measure will be calibrated.
 */
#define MENU_SAVE      10
#define MENU_CANCEL    11
#define MENU_NO_ACTION 12
#define MENU_MEASURE_1 13
#define MENU_MEASURE_2 14

#define UPDATABLE_DATA  0


/****************************************************************************************************
 *                                           DATA TYPES
 ****************************************************************************************************/


/*
 * Defines button click states
 */
enum E_ButtonClicks { NO_CLICK, SHORT_CLICK, LONG_CLICK };


/*
 * Different menu states, basically different menu views to help make the code
 * more understandable.
 */
enum E_MenuStates   { PANEL_VIEW         = 0,
                      BATTERY_VIEW       = 1,
                      MENU_VIEW_1        = 2,
                      MENU_VIEW_2        = 3,
                      MENU_VIEW_3        = 4,
                      CALIBRATION_VIEW_1 = 5,
                      CALIBRATION_VIEW_2 = 6,
                      NO_MENU            = 7 };


/*
 * Contains an array of textfields and their amount to define a single menu view.
 */
typedef struct
{
    const T_TextField * const textFields;
    const uint8_t             textFieldCount;
} T_MenuView;


/*
 * Defines a menu system with a pointer to an array of menu views and variables
 * to store the current menu state.
 *
 * As some text fields require to be updated according to measurements and current
 * selection a menuSystem has eight helper char tables of eight bytes length that
 * are modifiable. To hold all the current menu view's text fields nicely in one
 * place there is also a helper table of text fields. This makes managing the text
 * fields easier for other modules.
 */
typedef struct
{
    enum E_MenuStates  menuState;
    uint8_t            currentSelection;

    enum E_MenuStates  previousMenu;

    const T_MenuView * views;

    char               updatableCharTables[8][8];
    T_TextField        currentTextFields[15];
} T_MenuSystem;


/****************************************************************************************************
 *                                           CONSTANTS
 ****************************************************************************************************/


/*
 *                                      MENU INITIALIZATION
 *
 *     In this part the menu views and their text fields are defined. A zero in the char pointer
 *     value means the text field's text value must be defined in Menu.c's Menu_UpdateTextFields
 *     function as it will change it's value according to measurements, menu selection etc.
 *
 */

const static T_TextField PANEL_VIEW_FIELDS[]      = { { "PANEELI",         0,   2     },
                                                      { "JaNNITE",         47,  2     },
                                                      { "VIRTA",           96,  2     },
                                                      { "1",               20, 15     },
                                                      { "2",               20, 28     },
                                                      { "3",               20, 41     },
                                                      { "4",               20, 54     },
                                                      { UPDATABLE_DATA,    45, 15     },   /* Panel 1 Voltage */
                                                      { UPDATABLE_DATA,    85, 15     },   /* Panel 1 Current */
                                                      { UPDATABLE_DATA,    45, 28     },   /* Panel 2 Voltage */
                                                      { UPDATABLE_DATA,    85, 28     },   /* Panel 2 Current */
                                                      { UPDATABLE_DATA,    45, 41     },   /* Panel 3 Voltage */
                                                      { UPDATABLE_DATA,    85, 41     },   /* Panel 3 Current */
                                                      { UPDATABLE_DATA,    45, 54     },   /* Panel 4 Voltage */
                                                      { UPDATABLE_DATA,    85, 54     } }; /* Panel 4 Current */

const static T_TextField BATTERY_VIEW_FIELDS[]    = { { "AKKU",            53,  2     },
                                                      { "JaNNITE",         10, 20     },
                                                      { "VIRTA",           80, 20     },
                                                      { UPDATABLE_DATA,     5, 38     },   /* Battery voltage */
                                                      { UPDATABLE_DATA,    70, 38     } }; /* Battery current */

const static T_TextField MENU_1_FIELDS[]          = { { "VIRITYS 1/3",        15,  2  },
                                                      { "PANEELI 1: JaNNITE", 10, 15  },
                                                      { "PANEELI 1: VIRTA",   10, 28  },
                                                      { "PANEELI 2: JaNNITE", 10, 41  },
                                                      { "PANEELI 2: VIRTA",   10, 54  },
                                                      { UPDATABLE_DATA,        5, 15  },   /* Selection 0 */
                                                      { UPDATABLE_DATA,        5, 28  },   /* Selection 1 */
                                                      { UPDATABLE_DATA,        5, 41  },   /* Selection 2 */
                                                      { UPDATABLE_DATA,        5, 54  } }; /* Selection 3 */

const static T_TextField MENU_2_FIELDS[]          = { { "VIRITYS 2/3",        15,  2 },
                                                      { "PANEELI 3: JaNNITE", 10, 15 },
                                                      { "PANEELI 3: VIRTA",   10, 28 },
                                                      { "PANEELI 4: JaNNITE", 10, 41 },
                                                      { "PANEELI 4: VIRTA",   10, 54 },
                                                      { UPDATABLE_DATA,        5, 15 },    /* Selection 0 */
                                                      { UPDATABLE_DATA,        5, 28 },    /* Selection 1 */
                                                      { UPDATABLE_DATA,        5, 41 },    /* Selection 2 */
                                                      { UPDATABLE_DATA,        5, 54  } }; /* Selection 3 */

const static T_TextField MENU_3_FIELDS[]          = { { "VIRITYS 3/3",        15,  2 },
                                                      { "AKKU: JaNNITE",      10, 15 },
                                                      { "AKKU: VIRTA",        10, 28 },
                                                      { "TALLENNA",           10, 41 },
                                                      { "PERUUTA",            10, 54 },
                                                      { UPDATABLE_DATA,        5, 15 },    /* Selection 0 */
                                                      { UPDATABLE_DATA,        5, 28 },    /* Selection 1 */
                                                      { UPDATABLE_DATA,        5, 41 },    /* Selection 2 */
                                                      { UPDATABLE_DATA,        5, 54 } };  /* Selection 3 */

const static T_TextField CALIBRATION_MENU_FIELDS[] = {{ UPDATABLE_DATA,       40,  2 },    /* PANEELI / AKKU    */
                                                      { UPDATABLE_DATA,       90,  2 },    /* Panel number: 1-4 */
                                                      { "ASETA",              15, 15 },
                                                      { UPDATABLE_DATA,       50, 15 },    /* JÄNNITE / VIRTA   */
                                                      { UPDATABLE_DATA,       95, 15 },    /* 1/2 or 2/2        */
                                                      { UPDATABLE_DATA,       40, 28 },    /* Calibration point */
                                                      { UPDATABLE_DATA,        7, 41 },    /* Selection mark: > */
                                                      { "TAKAISIN",           13, 41 },
                                                      { UPDATABLE_DATA,       60, 41 },    /* Selection: < > <  */
                                                      { "OK",                 84, 41 },
                                                      { "MITTAUS",            19, 54 },
                                                      { UPDATABLE_DATA,       67, 54 } };  /* Measured value    */

/*
 * Defines an array of menuScreen entities. Both calibration states use the same layout so it's used twice.
 */
const static T_MenuView MENU_VIEWS[] = { { PANEL_VIEW_FIELDS,       15 },
                                         { BATTERY_VIEW_FIELDS,      5 },
                                         { MENU_1_FIELDS,            9 },
                                         { MENU_2_FIELDS,            9 },
                                         { MENU_3_FIELDS,            9 },
                                         { CALIBRATION_MENU_FIELDS, 12 },
                                         { CALIBRATION_MENU_FIELDS, 12 } };


/****************************************************************************************************
 *                                         GLOBAL FUNCTIONS
 ****************************************************************************************************/


/* Updates menu view with given information and returns a task for main module perform */
inline uint8_t Menu_UpdateView(T_MenuSystem * pMenu, uint8_t buttonState, float * pMeasResults, T_CalibrationInfo * pCalibInfo);


#endif /* CHARGER_MENU_H_ */
