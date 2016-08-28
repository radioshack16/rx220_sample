/***********************************************************************/
/* I2C                                                                 */
/***********************************************************************/

#include "ports.h"
#include "cmt.h"
#include "i2c.h"

//--------------------------------------------------------------------------------
// Delay function, temporarily used.
//--------------------------------------------------------------------------------
#define _DELAY_COUNT_EXPERIMENTALLY_DECIDED_      7     // AKI-RX220, CPU_CLK=20MHz.
// Measured time for n=3:
//  T = 15.0us
static void i2c_delay_about_5us_x(int n)
{
    volatile   int i, j;
    for (j=0;j<n;j++) {
        for (i=0;i<_DELAY_COUNT_EXPERIMENTALLY_DECIDED_;i++);
    }
}

//--------------------------------------------------------------------------------
// Delay function test bench
//--------------------------------------------------------------------------------
void i2c_dbg_delay_test(void)
{
    while(1) {
        I2C_SCL_X = 0;
        CMT1_delay_100us(2);
        I2C_SCL_X = 1;
        CMT1_delay_100us(2);
    }
    // NEVER COME HERE

    while(1) {
        I2C_SCL_X = 0;
        i2c_delay_about_5us_x(3);
        I2C_SCL_X = 1;
        i2c_delay_about_5us_x(3);
        I2C_SCL_X = 0;
        i2c_delay_about_5us_x(3);
        I2C_SCL_X = 1;
        i2c_delay_about_5us_x(3);
        I2C_SCL_X = 0;
        i2c_delay_about_5us_x(6);
        I2C_SCL_X = 1;
        i2c_delay_about_5us_x(6);
    }
    // NEVER RETURN.
}

//--------------------------------------------------
// Wait propagation delay time of line SDA or SCL.
//--------------------------------------------------
void i2c_wait_tpd(void)
{
    CMT1_delay_10us(2);         // 20us for safty.
                                // Tr/collector tpd(Propagation Delay Time) at room temperature:
                                //  0.052us, when input gets L to H.
                                //  3.4  us, when input gets H to L
    //--------------------
    // OBSOLETE
    // i2c_delay_about_5us_x(3);   // About 15us
    //--------------------
}

//----------------------------------------
void i2c_init(void)
{
    i2c_SCL_set(1);
    i2c_SDA_set(1);
}
//----------------------------------------

int i2c_send_byte_2(char slave_adrs, char ctrl_byte,
                                     char cmddat_byte)
{
    char    d;
    int     status;

    i2c_start_cond();
    // Slave address
    i2c_send_slave_adrs(slave_adrs, I2C_WR);
    d = i2c_check_ack();
    if (d!=0) { status = -1; goto I2C_SEND_BYTE_2_RETURN; }

    // Next data control byte:
    // 0x00: cmd or 0x40: data
    //  B7: Co, Continue bit:                   0: last
    //  B6: D/C#, Data/Command selection bit
    //  other bits: 0
    i2c_send_raw_data(ctrl_byte);
    d = i2c_check_ack();
    if (d!=0) { status = -2; goto I2C_SEND_BYTE_2_RETURN; }

    // OLED cmd or data
    i2c_send_raw_data(cmddat_byte);
    d = i2c_check_ack();
    if (d!=0) { status = -3; goto I2C_SEND_BYTE_2_RETURN; }

    status = 0; // Success
I2C_SEND_BYTE_2_RETURN:
    i2c_stop_cond();

    return status;
}

int i2c_send_byte_3(char slave_adrs, char ctrl_byte,
                                     char cmddat_byte1,
                                     char cmddat_byte2)
{
    char    d;
    int     status;

    i2c_start_cond();
    // Slave address
    i2c_send_slave_adrs(slave_adrs, I2C_WR);
    d = i2c_check_ack();
    if (d!=0) { status = -1; goto I2C_SEND_BYTE_3_RETURN; }

    // Next data control byte:
    // 0x00: cmd or 0x40: data
    //  B7: Co, Continue bit:                   0: last
    //  B6: D/C#, Data/Command selection bit
    //  other bits: 0
    i2c_send_raw_data(ctrl_byte);
    d = i2c_check_ack();
    if (d!=0) { status = -2; goto I2C_SEND_BYTE_3_RETURN; }

    // 1st cmd or data
    i2c_send_raw_data(cmddat_byte1);
    d = i2c_check_ack();
    if (d!=0) { status = -3; goto I2C_SEND_BYTE_3_RETURN; }

    // 2nd: cmd or data
    i2c_send_raw_data(cmddat_byte2);
    d = i2c_check_ack();
    if (d!=0) { status = -4; goto I2C_SEND_BYTE_3_RETURN; }

    status = 0; // Success
I2C_SEND_BYTE_3_RETURN:
    i2c_stop_cond();

    return status;
}

void i2c_SCL_set(char b)
{
    int d;

    d=~b & 0x01;    // Invert for an external inverting transisitor.
    I2C_SCL_X = d;
    i2c_wait_tpd();
}

void i2c_SDA_set(char b)
{
    int d;

    d=~b & 0x01;    // Invert for an external inverting transisitor.
    I2C_SDAOUT_X= d;
    i2c_wait_tpd();
}

char i2c_SDA_get(void)
{
    int d;
    char ret;

    d = I2C_SDAIN_X;
    ret = (~d) & 0x01;
    return ret;
}

//----------------------------------------

// Start condition
void i2c_start_cond(void)
{
    i2c_SDA_set(1); i2c_SCL_set(1);
    i2c_SDA_set(0);
}

// Send slave address
//  slave address:  7bit
//  read/write_x:   1bit
void i2c_send_slave_adrs(char   sa7, char r_wx)
{
    int i, d;

    i2c_SCL_set(0);
    // slave address
    for (i=6;i>=0;i--) {
        d=(sa7>>i) & 0x01;
        i2c_SDA_set(d);
            i2c_SCL_set(1);
            i2c_SCL_set(0);
    }
    // read/write_x bit
    i2c_SDA_set(r_wx & 0x01);
        i2c_SCL_set(1);
        i2c_SCL_set(0);
}

// Send raw data
//  raw data:  8bit
void i2c_send_raw_data(char v)
{
    int i, d;

    i2c_SCL_set(0);
    for (i=7;i>=0;i--) {
        d=(v>>i) & 0x01;
        i2c_SDA_set(d);
            i2c_SCL_set(1);
            i2c_SCL_set(0);
    }
}

// Check Acknowledge
//  return:
//      0: Low,  Ack
//      1: High, NACK
//
char i2c_check_ack(void)
{
    int d;

    i2c_SCL_set(0);
    i2c_SDA_set(1); // release SDA

    i2c_SCL_set(1);
    d = i2c_SDA_get();
    i2c_SCL_set(0);

    return d;
}

// Stop condition
void i2c_stop_cond(void)
{
    i2c_SDA_set(0); i2c_SCL_set(0);
                    i2c_SCL_set(1);
    i2c_SDA_set(1);
}
