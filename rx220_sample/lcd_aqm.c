/***********************************************************************/
/* LCD AQM1602/0802 display control via I2C                            */
/***********************************************************************/

#include "ports.h"
#include "cmt.h"    // CMT1_delay_xxx()
#include "i2c.h"
#include "lcd_aqm.h"
#include "cdp_cmd.h"

//----------------------------------------
// Init
//----------------------------------------
// CMT_init() and i2c_init() must have been done in advance.
//----------------------------------------
// LCD ZETTLER DISPLAYS Model No:
//  AQM1602XA-RN-GBW
//  AQM0802XA-RN-GBW
//---
void    LCD_AQM_init(void)
{
    LCD_AQM_contrast_and_power(0); // range=6bit, 0-63

    LCD_AQM_wrcmd(CDP_CMD_CLEAR_DISPLAY);   CMT1_delay_ms(2);       // >1.08ms  // CLEAR_DISPLAY
    LCD_AQM_wrcmd(CDP_CMD_RETURN_HOME);     CMT1_delay_ms(2);       // >1.08ms
    LCD_AQM_wrcmd(CDP_CMD_DISPLAY_ON);      CMT1_delay_10us(3);     // >26.3us  // DISPLAY ON

    LCD_AQM_contrast_and_power(32); // range=6bit, 0-63
                                // 48, NG 
                                // 44, NG
                                // 40, Low contrast
                                // 36, Slightly low contrast
                                // 32, Good
                                // 24, Faint
                                // 16, Not shown
}

//----------------------------------------
// Set Contrast and power.
//  LCD AQM1602XA-RN-GBW
//  LCD AQM0802XA-RN-GBW
//
// Different from OLED-contrast command.
//----------------------------------------
void LCD_AQM_contrast_and_power(unsigned char contrast)     // Valid LSB-6bit
{
    unsigned char v;

    LCD_AQM_wrcmd(0x38);    CMT1_delay_10us(3);     // >26.3us  // Function Set, IS=0

    // Function Set: IS=1
    LCD_AQM_wrcmd(0x39);    CMT1_delay_10us(3);     // >26.3us  // Function Set, IS=1(Instruction table select)
        // LSB5bit: 1, DL _ N, DH, 0, IS
        //          1, 1  _ 1, 0 , 0, 1

        LCD_AQM_wrcmd(0x14);    CMT1_delay_10us(3); // >26.3us  // Bit3:    BS=0, bias: 1/5
                                                                // Bit2-0:  100(b): Frame_freq=183Hz, at 3.0V
        //----------------------------------------
        // Set Contrast Control
        //  C5-0: 6bit
        v = 0x70 + (contrast & 0x0F);
        LCD_AQM_wrcmd(v);   CMT1_delay_10us(3);     // >26.3us
            // 0 1 1 1 _ C3 C2 C1 C0
        v = 0x54 + ((contrast>>4) & 0x03);
        LCD_AQM_wrcmd(v);   CMT1_delay_10us(3);     // >26.3us
            // 0 1 0 1 _ Ion Bon C5 C4      // Ion: 0,    Icon on/off (INVALID)
            //           0   1              // Bon: 1,    Booster circuit on/off
        //----------------------------------------

        LCD_AQM_wrcmd(0x6C);    CMT1_delay_ms(200); // >200ms, for power stable
            // 0 1 1 0 _ Fon Rab2 Rab1 Rab0 // Fon:     1,        follower circuit on/off
            //           1   1    0    0    // Rab2-0:  100(b)    V0 generator amplified ratio

    // Function Set: IS=0
    LCD_AQM_wrcmd(0x38);   CMT1_delay_10us(3);      // >26.3us  // Function Set, IS=0
        // LSB5bit: 1, DL _ N, DH, 0, IS
        //          1, 1  _ 1, 0 , 0, 0
}

//----------------------------------------
// Set Contrast
//  LCD AQM1602XA-RN-GBW
//  LCD AQM0802XA-RN-GBW
//
// Different from OLED-contrast command.
//----------------------------------------
void LCD_AQM_contrast(unsigned char contrast)   // Valid LSB-6bit
{
    unsigned char v;

    LCD_AQM_wrcmd(0x38);    CMT1_delay_10us(3);     // >26.3us  // Function Set, IS=0

    // Function Set: IS=1
    LCD_AQM_wrcmd(0x39);    CMT1_delay_10us(3);     // >26.3us  // Function Set, IS=1(Instruction table select)
        // LSB5bit: 1, DL _ N, DH, 0, IS
        //          1, 1  _ 1, 0 , 0, 1

        //----------------------------------------
        // Set Contrast Control
        //  C5-0: 6bit
        v = 0x70 + (contrast & 0x0F);
        LCD_AQM_wrcmd(v);   CMT1_delay_10us(3);     // >26.3us
            // 0 1 1 1 _ C3 C2 C1 C0
        v = 0x54 + ((contrast>>4) & 0x03);
        LCD_AQM_wrcmd(v);   CMT1_delay_10us(3);     // >26.3us
            // 0 1 0 1 _ Ion Bon C5 C4      // Ion: 0,    Icon on/off (INVALID)
            //           0   1              // Bon: 1,    Booster circuit on/off
        //----------------------------------------

    // Function Set: IS=0
    LCD_AQM_wrcmd(0x38);   CMT1_delay_10us(3);      // >26.3us  // Function Set, IS=0
        // LSB5bit: 1, DL _ N, DH, 0, IS
        //          1, 1  _ 1, 0 , 0, 0
}
//==================================================

//----------------------------------------
// Write single byte command
//----------------------------------------
int LCD_AQM_wrcmd(char cmd)
{
    return i2c_send_byte_2( LCD_AQM_SLAVE_ADRS,
                            0x00,
                                        // Co=0, D/C#=0
                                        // Continue bit         = 0, the last
                                        // Data/Command_x bit   = 0, Command
                            cmd);     
}

//----------------------------------------
// Write data
//----------------------------------------
int LCD_AQM_wrdat(char dat)
{
    return i2c_send_byte_2( LCD_AQM_SLAVE_ADRS,
                            0x40,
                                        // Co=0, D/C#=1
                                        // Continue bit         = 0, the last
                                        // Data/Command_x bit   = 1, Data
                            dat);     
}

//----------------------------------------
// Locate(y, x)
//  y: 0, 1
//  x: 0-15, for AQM1602
//     0-7:, for AQM0802
//----------------------------------------
// LCD AQM1602/0802 address differs from OLED.
//----------------------------------------
void LCD_AQM_locate(int y, int x)
{
    int adrs;

    adrs = 0x40 * y + x;
        //~~~~~ Different from OLED.
    LCD_AQM_wrcmd(0x80+adrs);   // Set DDRAM RAM Address (cmd code=0x80; adrs: B6-0)

    CMT1_delay_10us(3);     // >26.3us
}

//----------------------------------------
// Write string
//----------------------------------------
// LCD_AQM_locate(int y, int x) must be done in advance.
//----------------------------------------
void LCD_AQM_wrstr(char *s)
{
    int i;
    char c;

    for (i=0;i<16;i++) {
        c=*(s++);
        if (c=='\0') return;
        LCD_AQM_wrdat(c);      CMT1_delay_10us(3); // >26.3us
    }
    return;
}
