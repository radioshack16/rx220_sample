#ifndef __PORT_DEF__
#define __PORT_DEF__ 1

#include "iodefine.h"

//----------------------------------------
// LEDs and switch connected on Port-H
//----------------------------------------

// Slide switch
#define SLIDE_SWITCH    (PORTH.PIDR.BIT.B2)     // INPUT

//----------------------------------------
// I2C lines connected on Port-B
//----------------------------------------
#define I2C_SDAOUT_X    (PORTB.PODR.BIT.B0)     // ouput
#define I2C_SDAIN_X     (PORTB.PIDR.BIT.B1)     // INPUT, P"I"DR
#define I2C_SCL_X       (PORTB.PODR.BIT.B3)     // ouput

#define I2C_SDAIN       ((~I2C_SDAIN_X) & 1)

//----------------------------------------
// Monitor pin for debug
//----------------------------------------
//#define MONOUT1         (PORTX.PODR.BIT.Bn)

#endif
