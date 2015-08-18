/***********************************************************************/
/*                                                                     */
/*  FILE        :rx220_sample.c                                        */
/*  DATE        :Mon, Jul 20, 2015                                     */
/*  DESCRIPTION :Main Program                                          */
/*  CPU TYPE    :RX210                                                 */
/*                                                                     */
/*  This file is generated by Renesas Project Generator (Ver.4.53).    */
/*  NOTE:THIS IS A TYPICAL EXAMPLE.                                    */
/*                                                                     */
/***********************************************************************/

#include "iodefine.h"

//#include "typedefine.h"
#ifdef __cplusplus
//#include <ios>                        // Remove the comment when you use ios
//_SINT ios_base::Init::init_cnt;       // Remove the comment when you use ios
#endif

void main(void);
#ifdef __cplusplus
extern "C" {
void abort(void);
}
#endif

#define     MTU0_N_PERIOD       20000               // Period = N*(1/20MHz);    20000 -->1ms
#define     MTU0_N_PERIOD_HALF  (MTU0_N_PERIOD>>1)  // *(1/2)

#define     PRCR_PRKEY          0xA5                // Protect key

//------------------------------------------------------------
// Global variables
//------------------------------------------------------------
int     g_mtu2_count;
int     g_mtu2_count_prev;
int     g_ms_count;
int     g_led0; // 0 or 1
int     g_led1; // 0 or 1


//============================================================
// MTU2a
//============================================================
//--------------------------------
// Stop MTU2a/ ch0 counter
//--------------------------------
void MTU2a_ch0_stop(void)
{
    MTU.TSTR.BIT.CST0 = 0;      // ch0 stop
}

void MTU2a_ch0_clear(void)
{
    MTU0.TCNT = 0x0000;         // ch0 clear
}

//--------------------------------
// Clear MTU2a/ ch0 counter
//--------------------------------
void MTU2a_ch0_start(void)
{
    MTU.TSTR.BIT.CST0 = 1;      // ch0 start
}

//--------------------------------
// Restart MTU2a/ ch0 counter
//--------------------------------
void MTU2a_ch0_restart(void)
{
    MTU2a_ch0_stop();
    MTU2a_ch0_clear();
    MTU2a_ch0_start();
}

//--------------------------------
// Check MTU2a/ ch0 counter half passed?
//--------------------------------
int MTU2a_ch0_if_half_passed(void)
{
    return (MTU0.TCNT >= MTU0_N_PERIOD_HALF);
}


//============================================================
// MTU2a init
//============================================================
// Memo: MTU name:
//  MTU2a:  RX220
//  MTU2:   SH2-Tiny(7125)  // with no 'a' appended.
//---
// ch0:
//  input:  20MHz xtal clock
//  output: periodic 1000Hz, T=1ms
//          TIOCOA, 1 on compare match
//---
void MTU2a_init(void)
{
    //------------------------------------------------------------
    // Release module stop
    //------------------------------------------------------------
    //SYSTEM.MSTPCRA.BIT.MSTPA9  = 0; // 0: release stopped MTU module to run
    MSTP_MTU = 0;

    //------------------------------------------------------------
    // Stop all channels' counter   // MTU2/ch0-4: count start  0: stop, 1: start
    //------------------------------------------------------------
    MTU.TSTR.BIT.CST0 = 0;          // ch0
    MTU.TSTR.BIT.CST1 = 0;          // ch1
    MTU.TSTR.BIT.CST2 = 0;          // ch2
    MTU.TSTR.BIT.CST3 = 0;          // ch3
    MTU.TSTR.BIT.CST4 = 0;          // ch4
    MTU5.TSTR.BYTE    = 0;          // ch5
    //------------------------------------------------------------
    // No synchronization among channels
    //------------------------------------------------------------
    MTU.TSYR.BIT.SYNC0 = 0;
    MTU.TSYR.BIT.SYNC1 = 0;
    MTU.TSYR.BIT.SYNC2 = 0;

    MTU.TSYR.BIT.SYNC3 = 0;
    MTU.TSYR.BIT.SYNC4 = 0;
    //------------------------------------------------------------
    // ch0
    //------------------------------------------------------------
    //----------------------------------------
    // Timer Control Register
    //----------------------------------------
    MTU0.TCR.BIT.TPSC = 0;          // Timer count clock / Prescaler select:
                                    //   Internal clock 0:*PCLK/1, 1: PCLK/4, 2: PCLK/16, 3: PCLK/64,
                                    //   External clock 4: TCLKA,  5: TCLKB,  6: TCLKB,   7: TCLKB
    MTU0.TCR.BIT.CKEG = 0;          // Clock edge by:    0:*rising edge, 1: falling edge, 2, 3: both edges
    MTU0.TCR.BIT.CCLR = 1;          // Counter clear by: 1: TGRA compare match
    //----------------------------------------
    // Timer Mode Register
    //----------------------------------------
    MTU0.TMDR.BIT.MD  = 0;          // Mode of operation: 0: normal, 2: PWM, 4: phase measure mode1, etc.
    MTU0.TMDR.BIT.BFA = 0;          // Mode for TGRA: 0:*normal, 1: TGRC is a buffer of TGRA
    MTU0.TMDR.BIT.BFB = 0;          // Mode for TGRB: 0:*normal, 1: TGRD is a buffer of TGRB
    MTU0.TMDR.BIT.BFE = 0;          // Mode for TGRE: 0:*normal, 1: TRGF is a buffer of TGRE
    //----------------------------------------
    // Timer I/O Control Register
    //----------------------------------------
    // High
    MTU0.TIORH.BIT.IOA = 2;         // 2: Output from TIOC0A: 0 at initial, 1 on compare match
    MTU0.TIORH.BIT.IOB = 0;         // 0: Prohibit ouput from TIOC0B
    // Low
    MTU0.TIORL.BIT.IOC = 0;         // 0: Prohibit ouput from TIOC0C
    MTU0.TIORL.BIT.IOD = 0;         // 0: Prohibit ouput from TIOC0D
    //----------------------------------------
    // Timer Buffer Transfer Mode Register
    //----------------------------------------
    MTU0.TBTM.BYTE = 0;             // 0: Transfer at compare match A/B/E
    //----------------------------------------
    // Timer AD Conversion Request Controll Register
    //----------------------------------------
    MTU4.TADCR.WORD = 0;
    //----------------------------------------
    // Timer Counter Register
    //----------------------------------------
    MTU0.TCNT = 0x0000;
    //----------------------------------------
    // Timer General Register
    //----------------------------------------
    // $$$ SET PERIOD
    MTU0.TGRA = (MTU0_N_PERIOD-1);  // TGRA for compare match
                                    // -1 NECESSARY, since count {0, 1, 2, ..., (N-1)}
    MTU0.TGRE = MTU0.TGRA;          // the same timing as TGRA  // used for ADC start trigger
    MTU0.TGRF = 0;                  // for safty
    //----------------------------------------
    // Timer Interrupt Enable Register
    //----------------------------------------
    MTU0.TIER.BYTE  = 0;            // disable all MTU0-related interrupt
    MTU0.TIER2.BYTE = 0;            //
                                    // except:
    // $$$ SET INTERRUPT
     MTU0.TIER.BIT.TGIEA = 1;       // Interrupt Enabel by compare-match-A: 0: disable, 1: enable
    //----------------------------------------
    // Timer Noise Filter Control Register
    //----------------------------------------
    MTU0.NFCR.BYTE = 0;             // disable by all MTICOC0<A/B/C/D> input
    //----------------------------------------
    // MTU0 Remains stopped here.
    //----------------------------------------
}

//============================================================
// SCI
//============================================================
// Use SCI1
// LQFP64:  pin16:  TXD1 (Port26)
//          pin14:  RXD1 (Port30)
//------------------------------------------------------------
void sci_init(void) {
    //------------------------------------------------------------
    // Release module stop
    //------------------------------------------------------------
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

    //////////////////////////////////////////////////////////////////////////////////
    // Set pin function: TXD1, RXD1
    //////////////////////////////////////////////////////////////////////////////////
    //------------------------------------------------------------
    // Port Mode Register: 1st
    //------------------------------------------------------------
    PORT2.PMR.BIT.B6    = 0;    // General port as first.
    PORT3.PMR.BIT.B0    = 0;    // General port as first.
    //------------------------------------------------------------

    //------------------------------------------------------------
    // MPC(Multi Pin Function Controller)
    //------------------------------------------------------------
    //--------------------
    MPC.PWPR.BIT.B0WI   = 0;    // Enable writing PFSWE bit
    MPC.PWPR.BIT.PFSWE  = 1;    // Enable writing PFS register
    //--------------------

    //--------------------
    MPC.P26PFS.BIT.PSEL = 0xA;  // Pin select: TXD1
    //--------------------
    MPC.P30PFS.BIT.PSEL = 0xA;  // Pin select: RXD1
    MPC.P30PFS.BIT.ISEL = 0;    // Interrupt select: Don't use as IRQ0 input.
    //--------------------

    //--------------------
    MPC.PWPR.BIT.PFSWE  = 0;    // Prohibit writing PFS register
    MPC.PWPR.BIT.B0WI   = 1;    // Prohibit writing PFSWE bit
    //--------------------

    //------------------------------------------------------------
    // Port Mode Register: 2nd
    //------------------------------------------------------------
    PORT2.PMR.BIT.B6    = 1;    // SCI1/TXD.
    PORT3.PMR.BIT.B0    = 1;    // SCI1/RXD.
    //------------------------------------------------------------
    //////////////////////////////////////////////////////////////////////////////////

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
    SCI1.BRR = 64;  // =65.1-1
    //------------------------------------------------------------

    //------------------------------------------------------------
    // SCI1: Serial Control Register, again.
    //------------------------------------------------------------
    SCI1.SCR.BIT.TE     = 1;    // Transmit Enale:  Enable
}

//============================================================
// ADC
//============================================================
// Data:                12bit
// Alignment:           right aligned in 16bit
// ADC start trigger:   TRG0EN(MTU0.TGRE compare match),
//                      1ms period
// Ch count:            12
// Mode:                single scan
// Interrupt:           generate interrupt requestS12ADI0 
//                      on the end of scan
//
// LQFP64:
//  AN00:   pin60:  (PortXX)
//  AN01:   pin58:  (PortXX)
//  AN02:   pin57:  (PortXX)
//  AN03:   pin56:  (PortXX)
//  AN04:   pin55:  (PortXX)
//  AN05:   NA
//  AN06:   pin53:  (PortXX)
//  AN07:   NA
//  AN08:   pin51:  (PortXX)
//  AN09:   pin50:  (PortXX)
//  AN10:   pin49:  (PortXX)
//  AN11:   pin48:  (PortXX)
//  AN12:   pin47:  (PortXX)
//  AN13:   pin46:  (PortXX)
//  AN14:   NA
//  AN15:   NA
//------------------------------------------------------------
void adc_init(void) {
    MSTP_S12AD = 0;     // Release module stop

    //------------------------------------------------------------
    // AD Control (Scan) Register
    //------------------------------------------------------------
   	S12AD.ADCSR.BIT.ADST    = 0;    // AD Start bit: 0: ADC stop, 1: ADC start          HOGE 
    //
	S12AD.ADCSR.BIT.DBLANS  = 0;    // Don't care in single scan.
	S12AD.ADCSR.BIT.GBADIE  = 0;    // Disable interrupt on the end of group B scan
	S12AD.ADCSR.BIT.DBLE    = 0;    // Not double trigger mode
	S12AD.ADCSR.BIT.EXTRG   = 0;    // Select synchronous trigger by MTU, ELC for ADC start
	S12AD.ADCSR.BIT.TRGE    = 0;    // Disable trigger for ADC start                    HOGE
	S12AD.ADCSR.BIT.ADIE    = 1;    // Enable Interrupt S12ADI0 on the end of scan
	S12AD.ADCSR.BIT.ADCS    = 0;    // Scan mode: 0:*single scan, 1: group scan, 2: continuous scan
   	// S12AD.ADCSR.BIT.ADST           // Don't care AD Start bit in signle scan, later.
    //------------------------------------------------------------
 
    //------------------------------------------------------------
    // ADANSA: AD Analog channel select A
    //------------------------------------------------------------
    S12AD.ADANSA.WORD = 0xffffh;    //  Enable all 16ch, except:
    S12AD.ADANSA.BIT.ANSA5  = 0;    //  Disable
    S12AD.ADANSA.BIT.ANSA7  = 0;    //  Disable
    S12AD.ADANSA.BIT.ANSA14 = 0;    //  Disable
    S12AD.ADANSA.BIT.ANSA15 = 0;    //  Disable
    //------------------------------------------------------------

    //------------------------------------------------------------
    // ADANSB: AD Analog channel select B
    //------------------------------------------------------------
    S12AD.ADANSB.WORD = 0x0000h;    // Not used in single scan.

    //------------------------------------------------------------
    // ADADS: AD Add-mode channel select
    //------------------------------------------------------------
    S12AD.ADADS.WORD = 0x0000h;     // Disable add-mode for all ch.

    //------------------------------------------------------------
    // ADADC: AD Add count
    //------------------------------------------------------------
    S12AD.ADADC.BI.ADC = 0;         // No addition.

    //------------------------------------------------------------
    // ADCER: AD Control Extention Register
    //------------------------------------------------------------
    S12AD.ADCER.ADRFMT = 0;   // right aligned
    HOGE


    







    // SET ETC HOGE



    // ADRD     HOGE 

    // ADDR0-ADDR15:    // AD Data Register
    // ADOCDR:          // AD Internal Reference Data Register



    //////////////////////////////////////////////////////////////////////////////////
    // Set pin function:
    //////////////////////////////////////////////////////////////////////////////////
    // HOGE
    // - ADC related port 
    // - port 4, E to be quiet
    //------------------------------------------------------------
    // MPC(Multi Pin Function Controller)
    //------------------------------------------------------------
        :
    //////////////////////////////////////////////////////////////////////////////////



}

void adc_trigger_enable(int sel) {
    if (sel==0) {
	    ADCSR.BIT.TRGE  = 0;    // Disable trigger for ADC start
    } else {
	    ADCSR.BIT.TRGE  = 1;    // Enable  trigger for ADC start
    }
}

//============================================================
// Hardware setup
//============================================================
void hwsetup(void)
{
    //----------------------------------------------------------------------
    // Protect Control Register
    //----------------------------------------------------------------------
    SYSTEM.PRCR.WORD        = (PRCR_PRKEY<<8)+0x0B;     // Write-enable all 

    //----------------------------------------------------------------------
    // Operating Power Consumption Control Register
    //----------------------------------------------------------------------
    SYSTEM.OPCCR.BIT.OPCM   = 0x02;     // 中速動作モード1A(max power)

    //======================================================================
    // Clock source
    //======================================================================
    // ICLK:  MAIN-XTAL 20MHz
    // PCLKD: Same as the above
    //----------------------------------------------------------------------
    // HOCO(32MHz)を使いたいが難。精度が+/-1%。PCLKDもそれに依存してしまう。
    //----------------------------------------------------------------------
    // ★レジスタによっては ReadCheck してから次に進むべしとのこと。あとで修正。HOGE
    //  発振run/stop, etc.

    //----------------------------------------------------------------------
    SYSTEM.SCKCR.BIT.PCKD   = 0x00;     // PCLKD(Peripheral clock D; ADC module clock): Ndiv=1
    SYSTEM.SCKCR.BIT.PCKB   = 0x00;     // PCLKB(Peripheral clock B; modules but ADC):  Ndiv=1
    //SYSTEM.SCKCR.BIT.BCK  = 0x00;     // External Bus clock:  Ndiv=1
    SYSTEM.SCKCR.BIT.ICK    = 0x00;     // ICLK(System clock):  Ndiv=1
    //SYSTEM.SCKCR.BIT.FCK  = 0x00;     // FlashIF clock
    //----------------------------------------------------------------------
    // Select Clock source      ★Run前にこれが先でよいのか？
    SYSTEM.SCKCR3.WORD      = 0x0200;   // 0x0100: HOCO (Freq accuracy: +/- 1%)
                                        // 0x0200:*MAIN 20MHz Xtal 
                                        // 0x0300: SUB
    //----------------------------------------------------------------------
    SYSTEM.HOCOCR2.BYTE     = 0;        // HOCO control register2: freq select
                                        // 0/1/2/3 = {32/36.864/40/50} [MHz]
    //----------------------------------------------------------------------
    // OSC Control Register: (0/1)=(run/stop)
    SYSTEM.MOSCCR.BIT.MOSTP     = 0;    // Main clock   (20MHz)
    SYSTEM.SOSCCR.BIT.SOSTP     = 1;    // Sub clock    (32.768kHz)
    SYSTEM.LOCOCR.BIT.LCSTP     = 1;    // LOCO
    SYSTEM.ILOCOCR.BIT.ILCSTP   = 1;    // IWDT(Independant Watch Dog Timer) clock
    SYSTEM.HOCOCR.BIT.HCSTP     = 1;    // HOCO         (32--50MHz; +/-1%)
    //----------------------------------------------------------------------

    //======================================================================
    // Port
    //======================================================================
	PORTH.PDR.BYTE      =0xfb;  // bit: 3 2 1 0     // Port Direction Register
                                //      1 0 1 1     // 0: input, 1: output 
	PORTH.PODR.BYTE     =0x00;
    //----------------------------------------------------------------------

    //======================================================================
    // MTU2a
    //======================================================================
    MTU2a_init();
    MTU2a_ch0_restart();

    //======================================================================
    // SCI
    //======================================================================
    sci_init();

    //======================================================================
    // ADC
    //======================================================================
    adc_init();

    //----------------------------------------------------------------------
    // Protect Control Register
    //----------------------------------------------------------------------
    SYSTEM.PRCR.WORD        = (PRCR_PRKEY<<8)+0x00;     // Write-protect all 
}

void    gvar_init(void) {
    g_mtu2_count = 0;
    g_ms_count   = 0;
    g_led0       = 0;
    g_led1       = 0;
}

void main(void)
{
    int     sw_slide;

    //------------------------------------------------------------
    // Init
    //------------------------------------------------------------
    gvar_init();
    hwsetup();

    //------------------------------------------------------------
    // Interrupt Controller setting
    //------------------------------------------------------------
    IPR(MTU0, TGIA0) = 2;   // Interrupt Priority Level
    IEN(MTU0, TGIA0) = 1;   // Interrupt Enable

    adc_trigger_enable(1);

    //------------------------------------------------------------
    // Program main
    //------------------------------------------------------------
	while(1) {
        sw_slide = PORTH.PIDR.BIT.B2;
        switch(sw_slide) {
            case 0:
                g_led0 = (g_ms_count>=500) ? 1 : 0;
                break;
            default:
                g_led0 = 0;
                break;
        }
        g_led1 = (g_ms_count<500) ? 1 : 0;
	}
}

#ifdef __cplusplus
void abort(void)
{

}
#endif
