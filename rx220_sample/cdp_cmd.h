#ifndef __CDP_CMD__ 
#define __CDP_CMD__    1

// Character Display Panel Commands

// (The command code originated with an ancient chip, HD44780, from HITACHI.)

// Standard commands
#define CDP_CMD_CLEAR_DISPLAY    0x01
#define CDP_CMD_RETURN_HOME      0x02
#define CDP_CMD_DISPLAY_ON       0x0C    // {1, Display, Cursor, Blink}

#endif
