#ifndef __LCD_AQM__ 
#define __LCD_AQM__    1

// LCD ZETTLER DISPLAYS Model No:
//  AQM1602XA-RN-GBW
//  AQM0802XA-RN-GBW

#define LCD_AQM_SLAVE_ADRS      0x3E

//---
// Functions
//---
void LCD_AQM_init(void);   
void LCD_AQM_contrast_and_power(unsigned char contrast);
void LCD_AQM_contrast(unsigned char contrast);

int  LCD_AQM_wrcmd(char cmd);
int  LCD_AQM_wrdat(char dat);

void LCD_AQM_wrstr(char *s);
void LCD_AQM_locate(int y, int x);
#endif
