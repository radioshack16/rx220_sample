//
// SCI
// RX220
//
// 20160409
//
#include    "iodefine.h"

#include    <stdio.h>
#include    <stdarg.h>

#include    "sci.h"

static  char    vsbuff[SCI_BUF_LEN];    /* 文字列展開用バッファ(必要なら増やす) */

//============================================================
// SCI
//============================================================
// Use SCI1
// LQFP64:  pin16:  TXD1 (Port26)
//          pin14:  RXD1 (Port30)
//------------------------------------------------------------

void SCI_init(void) {
    //------------------------------------------------------------
    // Release module stop
    //-----------------------------------------------------------
    //SYSTEM.MSTPCRB.BIT.MSTPB30  = 0; // 0: release stopped SCI1 module to run
    MSTP_SCI1 = 0;
    //------------------------------------------------------------

    //------------------------------------------------------------
    // SCI1: Serial Control Register
    //------------------------------------------------------------
    SCI1.SCR.BIT.CKE    = 0;    // Clock Enable:                        use INTERNAL(*1) baud rate generator
    SCI1.SCR.BIT.TEIE   = 0;    // Transmit End Interrupt Enable:       Disable
    SCI1.SCR.BIT.MPIE   = 0;    // Multi-Processor Interrupt Enable:    Normal
    SCI1.SCR.BIT.RE     = 0;    // Receive Enable:                      Disable
    SCI1.SCR.BIT.TE     = 0;    // Transmit Enale:                      Disable     // Enable later.
    SCI1.SCR.BIT.RIE    = 0;    // Receive Interrupt Enable:            Disable
    SCI1.SCR.BIT.TIE    = 0;    // Transmit Interrupt Enable:           Disable
    //------------------------------------------------------------

    //------------------------------------------------------------
    // SCI1: I2C mode register 1
    //------------------------------------------------------------
    SCI1.SIMR1.BIT.IICM = 0;    // I2C mode select:     Serial Inteface Mode
    //------------------------------------------------------------
    // SCI1: SPI mode registers
    //------------------------------------------------------------
    SCI1.SPMR.BYTE      = 0;    // Prohibit CTS; clock polarity, phase etc.
    //------------------------------------------------------------

    //------------------------------------------------------------
    // SCI1: Seaial Moder Register
    //------------------------------------------------------------
    SCI1.SMR.BIT.CKS    = 0;    // Clock select:        PCLK/1 clock (n=0; BRR)
    SCI1.SMR.BIT.MP     = 0;    // Multi Processor Communication:   Prohibit
    SCI1.SMR.BIT.STOP   = 0;    // Stop bit length:     1 bit
    SCI1.SMR.BIT.PM     = 0;    // Parity mode:         Even parity
    SCI1.SMR.BIT.PE     = 0;    // Parity Enable;       Paritny none
    SCI1.SMR.BIT.CHR    = 0;    // Character length:    8 bit
    SCI1.SMR.BIT.CM     = 0;    // Communication mode:  Asynchronous
    //------------------------------------------------------------

    //------------------------------------------------------------
    // SCI1: Smart Card Mode Register
    //    SCI1.TDR = 'A';
    //------------------------------------------------------------
    SCI1.SCMR.BIT.SMIF  = 0;    // Smart Card Interface mode select:    Serial Communication
    SCI1.SCMR.BIT.SINV  = 0;    // Data Invert bit:     not invert, the same as TDR/RDR
    SCI1.SCMR.BIT.SDIR  = 0;    // Direction:           LSB first
    SCI1.SCMR.BIT.BCP2  = 0;    // Clock pulse bit2:    93clocks (Valid only in Smart Card Mode)
    //------------------------------------------------------------

    //------------------------------------------------------------
    // SCI1: Serial Extended Mode Register
    //------------------------------------------------------------
    SCI1.SEMR.BIT.ACS0  = 0;    // Asynchronous clock source select:    External clock  (Don't care, since (*1) precedes.)
    SCI1.SEMR.BIT.ABCS  = 0;    // Clock pulse count select for (asynchronouns mode) 1bit:  16
    SCI1.SEMR.BIT.NFEN  = 0;    // Digital Noise Filter Enable:         Disable
    SCI1.SEMR.BIT.RXDESEL =0;   // Asynchronous RX Data Edge select for start bit:  Low
    //------------------------------------------------------------

    //------------------------------------------------------------
    // SCI1: Bit Rate Register
    //------------------------------------------------------------
    // 9600bps, PCLK=20MHz, n=0,
    SCI1.BRR = SCI_BRR_9600baud_20MHz;
    //------------------------------------------------------------

    //------------------------------------------------------------
    // SCI1: Serial Control Register, again.
    //------------------------------------------------------------
    SCI1.SCR.BIT.TE     = 1;    // Transmit Enale:  Enable
}

//--------------------------------------------------------------------------------
void SCIput(char c)
//--------------------------------------------------------------------------------
{
    while(SCI1.SSR.BIT.TEND!=1);    // Wait until transmit ends.
    SCI1.TDR = c;
    while(SCI1.SSR.BIT.TEND!=0);    // Wait until transmit starts.
}

static  int     len = 0;
//--------------------------------------------------------------------------------
// SCIprintf
//--------------------------------------------------------------------------------
void SCIprintf(char *fmt, ...)
{
int     i;
va_list arg;

    va_start(arg, fmt);
    *vsbuff = '\0';
    vsprintf(vsbuff, fmt, arg);
    va_end(arg);

    for(i=0;i<SCI_BUF_LEN;i++) {
        if( vsbuff[i] == 0) break;
        if( vsbuff[i] == '\n') {    // if LF,
            SCIput('\r');           // insert CR before LF for MS-DOS.
            len++;
        }
        SCIput(vsbuff[i]);
        len++;
    }
}

int SCIprintf_len(void)
{
    SCIprintf("SCIprintf_len=%d\n", len);
    return  len;
}


