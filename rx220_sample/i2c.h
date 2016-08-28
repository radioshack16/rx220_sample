#ifndef __I2C__
#define __I2C__ 1

//----------------------------------------
// I2C
//----------------------------------------
#define I2C_RD      1
#define I2C_WR      0

void i2c_dbg_delay_test(void);
void i2c_wait_tpd(void);

void i2c_init(void);
int  i2c_send_byte_2(char slave_adrs, char ctrl_byte, char cmddat_byte);
int  i2c_send_byte_3(char slave_adrs, char ctrl_byte, char cmddat_byte1, char cmddat_byte2);

void i2c_SCL_set(char b);
void i2c_SDA_set(char b);
char i2c_SDA_get(void);
//
void i2c_start_cond(void);
void i2c_send_slave_adrs(char sa7, char r_wx);
void i2c_send_raw_data(char v);
char i2c_check_ack(void);
void i2c_stop_cond(void);

#endif
