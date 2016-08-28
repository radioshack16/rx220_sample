/***********************************************************************/
/* cmt functions
/***********************************************************************/

#include "iodefine.h"
#include "cmt.h"

//--------------------------------------------------
// Assumed:
//  PCLK:    20MHz
//--------------------------------------------------

//--------------------------------------------------
// Setting:
//  CMT0:   not used.
//  CMT1:   used for delay
//  CMT2:   not used.
//  CMT3:   not used.
//--------------------------------------------------
//  CMT1: clock select: PCLK/8
//      cmt1_clk    = 2.5MHz
//      cmt1_period = 400ns
//--------------------------------------------------

void CMT_init(void)
{
    //--------------------------------------------------
    // Relase module from standby
    //--------------------------------------------------
    //MSTP_CMT0   = 0;  // not used
    MSTP_CMT1   = 0;
    //MSTP_CMT2   = 0;  // not used
    //MSTP_CMT3   = 0;  // not used

    //--------------------------------------------------
    // Compare Match Timer Start Register:  Stop all
    //--------------------------------------------------
    //                  0: stop, 1: start
    CMT.CMSTR0.BIT.STR0=0;   // ch0
    CMT.CMSTR0.BIT.STR1=0;   // ch1
    CMT.CMSTR1.BIT.STR2=0;   // ch2    CMSTR<1>, not 0.
    CMT.CMSTR1.BIT.STR3=0;   // ch3

    //--------------------------------------------------
    // Compare Match Counter: Clear all
    //--------------------------------------------------
    CMT0.CMCNT=0;
    CMT1.CMCNT=0;
    CMT2.CMCNT=0;
    CMT3.CMCNT=0;

    //--------------------------------------------------
    // Compare Match Timer Control/Status Register
    //--------------------------------------------------
    // Ch1
    CMT1.CMCR.BIT.CMIE=1;   // Compare Match Interrupt Enable: Enable, just for setting interrup flag
    CMT1.CMCR.BIT.CKS=0;
        // Divider select:
        //  2bit
        //  00: PCLK/8      <==
        //  01: PCLK/32
        //  10: PCLK/128
        //  11: PCLK/512
        // PCLK freq = 20MHz
        // => 20MHz/8=2.5MHz
    CMT1.CMCOR = (2500-1);  // 1ms temp // Take care! "-1"

    // Keep stopped state
}

#define CMT1_CMF    IR(CMT1, CMI1)  // =ICU.IR[IR_CMT1_CMI1].BIT.IR, is an equivalent for a compare-match-flag of CMT1.
//--------
// Memo:
//  IRn:    Interrupt Request
//  IERn:   Interrupt Enable
//  IPRn    Interrupt Priority
//
// Interrupt Vector table Info
// Src  Name Vect-No.   Detect  IER         IPR     DTCER
// CMT1 CMI1 29         edge    IER03.IEN5  IPR005  DTCER029 
//--------

//----------------------------------------
// (Internal use)
// CMT1_delay_core
//
//  (cmt_count x cmt1_period) x n delay
//----------------------------------------
static void CMT1_delay_core(int cmt_count, int n)
{
    int i;

    // Initialize
    //  It will take a few [us], but ignore it.
    CMT.CMSTR0.BIT.STR1=0;          // Stop     // 0: stop, 1: run
    CMT1.CMCOR  = (cmt_count-1);    // "-1" for correct period.  // Compare match Constant Register
    CMT1.CMCNT  = 0;                // Compare Match Counter
    CMT1_CMF    = 0;                // Clear Compare Match Flag, 0: not-match, 1: match
                                    // No need read-modify-write.
    CMT.CMSTR0.BIT.STR1=1;          // Start

    // Wait (base)us x n
    for (i=0;i<n;i++) {
        while(CMT1_CMF!=1);         // Wait for Compare-Match
        CMT1_CMF= 0;                // Clear(write after read) Compare Match Flag
                                    // No need read-modify-write.
    }
    CMT.CMSTR0.BIT.STR1=0;          // Stop
}

//----------------------------------------
//  10us x n delay
//----------------------------------------
void    CMT1_delay_10us(int n)
{
    // 25 = cmt1_clk x 10us
    CMT1_delay_core(25, n);
}

//----------------------------------------
//  40us x n delay
//----------------------------------------
void    CMT1_delay_40us(int n)
{
    // 100 = cmt1_clk x 40us
    CMT1_delay_core(100, n);
}

//----------------------------------------
//  100us x n delay
//----------------------------------------
void    CMT1_delay_100us(int n)
{
    // 250 = cmt1_clk x 100us
    CMT1_delay_core(250, n);
}

//----------------------------------------
//  1ms x n delay
//----------------------------------------
void    CMT1_delay_ms(int n)
{
    // 2500 = cmt1_clk x 1ms
    CMT1_delay_core(2500, n);
}
