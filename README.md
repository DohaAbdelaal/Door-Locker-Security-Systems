# Door-Locker-Security-Systems

Door Locker Security is a system to unlock a door using a password.

1. the system uses 2 ATmega32 Microcontrollers with frequency 8Mhz.

2.The project is designed and implemented based on the layered architecture 
model as follow:


a-ℳc1 → HMI_ECU (Human Machine Interface) with 2x16 LCD and 4x4 keypad.
This MCU is responsible only to take the password entered by the user and to display the required instruction by LCD. 
-LCD and Keypad in the HAL layer.
-GPIO, UART,and Timer in the MCAL layer.

b-ℳc2 → Control_ECU with EEPROM, Buzzer, and Dc-Motor. This MCU is responsible to compare the two passwords and to save it in the EEPROM in case it was right, and to restore it when it is needed.
-buzzer, DC-MOTOR and EEPROM in the HAL layer. 
-GPIO, I2C, UART, and Timer in the MCAL layer.

3. The system asks the user to enter a password and confirm it,if he did not enter the same password the system asks him to start to make the password and confirm it.

4. After setting the password, the user will be allowed to open the door using the password saved in the EEPROM.
The user have 3 chances to enter the right password and the motor will open the door, otherwise if it is wrong for 3 consecutive times the buzzer is activated.

5.The user also has a chance to change the password using the operation mentioned in step3.

-The connection between the 2 MCUs is built through UART.

-The connection between the MCU and EEPROM is built using I2C.
