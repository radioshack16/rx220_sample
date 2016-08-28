#ifndef __OLED_SUNLIKE__ 
#define __OLED_SUNLIKE__    1

#define OLEDSL_SLAVE_ADRS       0x3C    // Slave address of the OLED SUNLIKE DISPLAY Model No: SO1602A.

//---
// Functions
void OLED_init(void);   
void OLED_contrast(unsigned char contrast);
//
int  OLED_wrcmd(char cmd);
int  OLED_wrcmd_db(char cmd1, char cmd2);
int  OLED_wrdat(char dat);
void OLED_wrstr(char *s);
void OLED_locate(int y, int x);

#endif
