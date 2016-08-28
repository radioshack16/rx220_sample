/***********************************************************************/
/* OLED Sunlike display SO1602A control via I2C                        */
/***********************************************************************/

#include "ports.h"
#include "cmt.h"    // CMT1_delay_xxx()
#include "i2c.h"
#include "oledsl.h"
#include "cdp_cmd.h"

//----------------------------------------
// Init
//----------------------------------------
// CMT_init() and i2c_init() must have been done in advance.
//----------------------------------------
// Device: SO1602A, OLED display, made by SUNLIKE DISPLAY
//         FOSC assumed to be 540kHz
//
// Ref: (helpful) spec sheet for ACM1602NI-FLW-FBW-M01,
//      LCD display,
//---
void    OLED_init(void)
{
    OLED_wrcmd(CDP_CMD_CLEAR_DISPLAY);  CMT1_delay_ms(2);       // >1.52ms
    OLED_wrcmd(CDP_CMD_RETURN_HOME);    CMT1_delay_ms(2);       // >1.52ms
    OLED_wrcmd(CDP_CMD_DISPLAY_ON);     CMT1_delay_10us(4);     // >37us
                // bit[3-0]: {1, Display, Cursor, Blink}
    OLED_wrcmd(0x06);                   CMT1_delay_10us(4);     // >37us
                // Entry_Mode_Set: 0000_0110:
                // bit2: NA:  1
                // bit1: I/D: 1 // Cursor moving direction: 1: right
                // bit0: S:   0 // Shift entire display

    OLED_contrast(0x7F);    // 0x00--0xFF;  0x00: dark, not so much dark. 0x7F: Power-on-reset
}

//----------------------------------------
// Set Contrast
//  SO1602A
//----------------------------------------
void OLED_contrast(unsigned char contrast)
{
    // SD=0, after power on reset, assumed.

    // Function Set: RE=1
    OLED_wrcmd(0x2A);                       CMT1_delay_10us(4);     // >37us
        // LSB4bit: N, DH, RE, IS
        //          1, 0 , 1 , 0

        // Function Set: SD=1
        OLED_wrcmd(0x79);                       CMT1_delay_10us(4);     // >37us

            // Set Contrast Control (double byte command)
            OLED_wrcmd_db(0x81, (char) contrast);   CMT1_delay_10us(4);     // >37us

        // Function Set: SD=0
        OLED_wrcmd(0x78);                       CMT1_delay_10us(4);     // >37us

    // Function Set: RE=0
    OLED_wrcmd(0x28);                       CMT1_delay_10us(4);     // >37us
        // LSB4bit: N, DH, RE, IS
        //          1, 0 , 0 , 0
}
//==================================================

//----------------------------------------
// Write single byte command
//----------------------------------------
int OLED_wrcmd(char cmd)
{
    return i2c_send_byte_2(OLEDSL_SLAVE_ADRS, 0x00, cmd);
}

//----------------------------------------
// Write double byte command
//----------------------------------------
int OLED_wrcmd_db(char cmd1, char cmd2)
{
    return i2c_send_byte_3(OLEDSL_SLAVE_ADRS, 0x00, cmd1, cmd2);
}

//----------------------------------------
// Write data
//----------------------------------------
int OLED_wrdat(char dat)
{
    return i2c_send_byte_2(OLEDSL_SLAVE_ADRS, 0x40, dat);
}

//----------------------------------------
// Write string
//----------------------------------------
//
// OLED_locate(int y, int x) must be done in advance.
//
void OLED_wrstr(char *s)
{
    int i;
    char c;

    for (i=0;i<16;i++) {
        c=*(s++);
        if (c=='\0') return;
        OLED_wrdat(c);      CMT1_delay_10us(4); // >37us
    }
    return;
}

//----------------------------------------
// Locate(y, x)
//  y: 0, 1
//  x: 0-15
//----------------------------------------
void OLED_locate(int y, int x)
{
    int adrs;

    adrs = 0x20 * y + x;
        // ~~~~ Different from LCD/AQM1602/0802.
    OLED_wrcmd(0x80+adrs);  // Set DDRAM RAM Address (cmd code=0x80; adrs: B6-0)

    CMT1_delay_10us(4);     // >37us
}
