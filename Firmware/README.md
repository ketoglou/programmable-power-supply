# 5V,1.5A Programmable Power Supply 

## The project developement use the PIC18F25K50 as mcu.

### The project is compiled in XC8 v2.0

#### Code Structure

* config.h : A header file that contains all the necessary configurations for the basic registers.
* I2C.h : A header file which defines the functions and variables that I2C protocol use.
* I2C.c : The implementation of the I2C functions.
* UART1.h : A header file which defines the functions and variables that UART protocol use.
* UART1.c : The implementation of the UART functions.
* main.c : This file contains the main function along with the interrupt routines and some other helping functions.
