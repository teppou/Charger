Overall view to Charger Project
==============
 	
Charger is a freetime hobby project where four solar panels gather solar energy which is then led to a battery using PWM (pulse-width modulation) technique with MPPT (maximum power point tracking) optimization to charge the battery in a very efficient way. The hardware is designed by Tapio Uimonen and the software implementation (everything in Git) is by Teppo Uimonen.

The microcontroller currently attached to the device is an ultra-low power model MSP430F2232 made by TI. It takes ADC measurements of the current and the voltage of all four panels and also from the battery being charged. Raw measurement results are converted with coefficient and offset values into usable voltage and current results. These values are used to determine the state of charging and to adjust each panel's PWM output to control the charging. The frequency of each PWM output is 128 kHz and this is controlled with Timer_A module for panels one and two and with Timer_B module for panels three and four. Timers source their clock signal from a 16 MHz crystal oscillator also connected to the device and then set the length of CCR0 to 128 thus setting the frequency: 16 MHz / 128 = 128 kHz. CCR1 and CCR2 registers of both timers toggle the PWM signals up according to the charging state.

There's also a 128x64 pixel LCD screen connected through USCI module and pins 4.0 and 2.5. This is used to show the user the measurement values of different quantities through panel and battery measurement views. This view can be switched by clicking shortly a single button next to the screen. Also there is implemented a menu system for two-point-calibration of measurement channels. Calibration measurements are then calculated into conversion coefficient and offset values to adjust the final result of each channel's measurement. Menu system is toggled by pressing the button a bit longer. This longer click is also used to confirm actions in menu state while a short click changes the current selection. Calibration and adjustment results are either saved to FLASH memory or canceled depending on user's choice when exiting the menu mode. When an adjustment is stored to FLASH the charger module reads it during program's initialization phase. The 8p font for menus is made from a font bitmap image by first reading it to MATLAB and then converting it into a char array hexadecimal representation. 
 
The software is divided into relatively small modules. Charger is the main module controlling the overall flow of the program by first initializing the device and then communicating with submodules in main function's while loop. Submodules are: Adjustment, Menu, LCD and PWM. The submodules share a few common datatypes defined in Common.h but never interact with each other directly, instead Charger module calls their global functions with specific parametres.

For testing an oscilloscope is used to detect how signals are being transmitted and the device is powered by an external power source.

The code is written with Code Composer Studio 6.1 and programmed to the device using Olimex MSP430 Programmer 1.3.

CURRENT STATE OF THE PROJECT
--------------
							
HARDWARE: the circuit board is designed and equipped to be used with solar panel number four. At the time this is the only panel with necessary equipment to lead charge from the panel to the battery. Other than this, all four panel's PWM state can be controlled and observed with an oscilloscope, also panel number one is equipped enough for it's voltage value to be read. Also the battery is connected and it's voltage can be read. LCD screen is connected and working.

Due to a fault in circuit board design a single panel cannot simultaneously both tribute usage voltage to the microcontroller and lead charge from the PWM output to the battery. This is critical and the software part is halted until the problem is solved.

SOFTWARE: the software is divided into different parts that work separately. At the moment the software can display the voltage and current measurement values of four panels and the battery in measurement views. Also the menu system for calibrating and adjusting measurement channels is implemented and working. All in all, modules Charger, Adjustment, LCD and Menu are in a good state. Charger module will be developed more when the hardware fault is fixed.	
TODO LIST FOR SOFTWARE
--------------
	
Critical:
- Adding a warning view indicating battery voltage outside limits. Minor additions to Menu and Charger modules.

- Some charging states are thought to have a maximum performing time. For this an internal software timer will be implemented. This would be of use also for defining more accurately the difference between a short and a long button click etc.

- Software works when the device is turned on but a careful testing procedure must be designed and performed before "releasing" it into real use. But this is not anything to hurry with.

- I haven't found a good memory detection tool for embedded C in Windows environment so compiling the code in Linux and checking the code with Valgrind should be done even though the program is working perfectly. But who knows, maybe there's a memory management error causing the two behaviours described below.
		
Should be looked:
- LCD screen has sometimes shut itself down suddenly. This is prevented by turning it on at regular intervals in Charger module but the cause should be investigated.

- For some reason the whole system halts quickly if 16 MHz crystal is sourced to MCLK. The system works well when MCLK is sourced from DC but the real reason should be found.
		
To make the code more elegant:
- Current menu system serves it's purpose but is quite hard-coded and static. If more functionality will be added to the system a more dynamic menu approach should be considered to get rid of the switch approach. Function pointers could be of use here. Possibly also allocating memory dynamically when switching through views: but the current approach is really good because all needed memory is allocated in the initializing phase of the program.

- The PWM module is only in the first steps of development, but instead of doing the calculations with float values an aprroach where all the control is done with unsigned int values should be thought for better performance.

- In most cases when programming with devices there is a need for a structure representing a single device. In the current approach there are no structs (and of course not classes) for panels or battery because their values are easily maintained in measure information structure. But for better readibility, overall logic and dynamics there could be structures for these devices if more functionality will be added to the program. 
		
A microcontroller with more memory will be installed at some point:
- At the moment only the most previous measurement results are read which results into voltage and current values changing 0.03 units (A or V) when raw measurement values change a single step. To make the result step size smaller, to 0.01 units, current values should be calculated as an average of past 3-5 measurements and holding these values requires more RAM. 
 
- Depending on the amount of RAM all the information of 128x64 pixels LCD could be located in one buffer. This would make updating both the buffer and the screen faster and also the code simpler and more elegant. Especially the menu system approach could be though again as there would not be need to hold so many char arrays all the time because of changing data. Then again this of no importance at the moment as everything works well.
 		
Possible system additions:
- Charging different types of batteries with different charging methods.
 		
- A USB interface to connect the device with computers and tablets. This would be used for logging data of charging and solar energy harvesting with a homemade external software.
 
 
 
 
