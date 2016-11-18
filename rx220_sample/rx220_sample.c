/***********************************************************************/
/*                                                                     */
/*  FILE        :rx220_sample.c                                        */
/*  DATE        :Mon, Jul 20, 2015                                     */
/*  DESCRIPTION :Main Program                                          */
/*  CPU TYPE    :RX210                                                 */
/*                                                                     */
/*  This file is generated by Renesas Project Generator (Ver.4.53),    */
/*  and modified by Kazuyuki Hirooka.                                  */
/*                                                                     */
/***********************************************************************/

//----------------------------------------
// Akizuki RX220 CPU board      // LQFP64
// CPU chip: RF52206BDFM
// ROM 256KB   // FFFC0000h〜FFFFFFFFh
// RAM 16KB    // 00000000h〜00003FFFh
//----------------------------------------

#include "iodefine.h"

#include "ports.h"
#include "util.h"           // tcnt_show(), ms_abs_count_show()
#include "sci.h"            // SCIprintf()
#include "mtu.h"
#include "cmt.h"
#include "i2c.h"
#include "oledsl.h"
#include "lcd_aqm.h"
#include "etc.h"
#include "dft.h"
#include "fft_real_n_recur.h"
#include "stack_var_adrs.h"

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

#define     MTU2_CH3_UNIT_SEC   (256.0/20e6)        // =12.8[us]=(1/(20MHz/256))=PRESCALE/PLCK

#define     PRCR_PRKEY          0xA5                // Protect key

//------------------------------------------------------------
// Global variables
//------------------------------------------------------------
int     g_ms_abs_count;
int     g_ms_count;
int     g_led0; // 0 or 1
int     g_led1; // 0 or 1
//----------------------------------------
// Stack variable address monitor
int     *g_intp_monitor     = (int*)0xFFFF;
int     *g_intp_monitor_min = (int*)0xFFFF;
//----------------------------------------
// ADC input data buffer
short   g_adc_in_a[120*6];
short   g_adc_in_b[120*6];
//----------------------------------------

//======================================================================
// Port
//======================================================================
void PORT_init(void)
{
    //------------------------------------------------------------
    // LEDs and slide-switch on the BASE board
    //------------------------------------------------------------
    // PH0: LED1
    // PH1: LED2
    // PH3: Slide-switch
    PORTH.PDR.BYTE      = 0xFB; // bit: 3 2 1 0     // Port Direction Register
                                //      1 0 1 1     // 0/1 -> Input/Output
    PORTH.PODR.BYTE     = 0x00;
        // Below are the same as power-on reset:
        PORTH.PCR.BYTE      = 0x00; // Pull-up register: NO

    //------------------------------------------------------------
    // I2C ports for OLED character display
    //------------------------------------------------------------
    // PB0: output  SDAO_X
    // PB1: input   SDAI
    // PB3: output  SCL_X       // PB2 does not exist.
    PORTB.PDR.BYTE      = 0xF9; // bit: 3 2 1 0     // Port Direction Register
                                //      1 0 0 1     // 0/1 -> Input/Output
    PORTB.PODR.BYTE     = 0x00;
        // Below are the same as power-on reset:
        PORTB.ODR0.BYTE     = 0x00; // CMOS output(Not open-drain)
        PORTB.ODR1.BYTE     = 0x00; // CMOS output(Not open-drain)
        PORTB.PCR.BYTE      = 0x00; // Pull-up register: NO
        PORTB.DSCR.BYTE     = 0x00; // Drive-capability: NORMAL
}
//------------------------------------------------------------
// Set port direction as input not to affect ADC performance
//------------------------------------------------------------
//  port 4 and E:    The pins are multiplexed with AD input.
//  port 0, 4:       These use analog power supply, and
//                   must not be used as output ports.
void PORT_set_input_for_ADC(void)
{
    PORT4.PDR.BYTE  = 0x00;
    PORTE.PDR.BYTE  = 0x00;
}

//============================================================
// MTU2
//============================================================

//////////////////////////////////////////////////////////////
// MTU2/ch0: stop, clear, start, restart
//////////////////////////////////////////////////////////////
void MTU2_ch0_stop(void)
{
    MTU.TSTR.BIT.CST0 = 0;      // ch0 stop
}

void MTU2_ch0_clear(void)
{
    MTU0.TCNT = 0x0000;         // ch0 clear
}

void MTU2_ch0_start(void)
{
    MTU.TSTR.BIT.CST0 = 1;      // ch0 start
}

void MTU2_ch0_restart(void)
{
    MTU2_ch0_stop();
    MTU2_ch0_clear();
    MTU2_ch0_start();
}

//--------------------------------
// Check MTU2/ ch0 counter half passed?
//--------------------------------
int MTU2_ch0_if_half_passed(void)
{
    return (MTU0.TCNT >= MTU0_N_PERIOD_HALF);
}

//////////////////////////////////////////////////////////////
// MTU2/ch3: stop, clear, start, restart
//////////////////////////////////////////////////////////////
void MTU2_ch3_stop(void)
{
    MTU.TSTR.BIT.CST3 = 0;      // ch3 stop
}

void MTU2_ch3_clear(void)
{
    MTU3.TCNT = 0x0000;         // ch3 clear
}

void MTU2_ch3_start(void)
{
    MTU.TSTR.BIT.CST3 = 1;      // ch3 start
}

void MTU2_ch3_restart(void)
{
    MTU2_ch3_stop();
    MTU2_ch3_clear();
    MTU2_ch3_start();
}

int MTU2_ch3_count(void)
{
    return MTU3.TCNT;
}

double  MTU2_ch3_unit_in_sec(void)
{
    return MTU2_CH3_UNIT_SEC;
}

//============================================================
// MTU2 init
//============================================================
// Memo: MTU name in HW manual:
//  MTU2:   RX621, SH2-Tiny(7125)  // with no 'a' appended.
//  MTU2a:  RX220
//---
// ch0:     for 1ms-interrupt
//  input:  PCLK=20MHz xtal clock
//  output: periodic 1000Hz, T=1ms
//          TIOCOA, 1 on compare match
//---
// ch3:     for elapsed time measurement
//  input:  PCLK=20MHz xtal clock
//          Prescale=1/256 ==> T=12.8us
//              //Prescale=1/1024 ==> T=51.2us
//  output: none
//---
void MTU2_init(void)
{
    //------------------------------------------------------------
    // Release module stop
    //------------------------------------------------------------
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
    ////////////////////////////////////////////////////////////////////////////////
    // ch0
    ////////////////////////////////////////////////////////////////////////////////
    //----------------------------------------
    // Timer Control Register
    //----------------------------------------
    MTU0.TCR.BIT.TPSC = 0;          // Timer count clock / Prescaler select:
                                    //   Internal clock 0:*PCLK/1, 1: PCLK/4, 2: PCLK/16, 3: PCLK/64,
                                    //   External clock 4: TCLKA,  5: TCLKB,  6: TCLKC,   7: TCLKD
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
    MTU0.TGRB = 0xFFFF;             // the same as init value
    MTU0.TGRC = 0xFFFF;             // the same as init value
    MTU0.TGRD = 0xFFFF;             // the same as init value
    MTU0.TGRF = 0xFFFF;             // the same as init value
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
    MTU0.NFCR.BYTE = 0;             // disable for all MTICOC0<A/B/C/D> input
    //----------------------------------------
    // MTU0 Remains stopped here.
    //----------------------------------------

    ////////////////////////////////////////////////////////////////////////////////
    // ch1
    ////////////////////////////////////////////////////////////////////////////////
    // Blank

    ////////////////////////////////////////////////////////////////////////////////
    // ch2
    ////////////////////////////////////////////////////////////////////////////////
    // Blank

    ////////////////////////////////////////////////////////////////////////////////
    // ch3
    ////////////////////////////////////////////////////////////////////////////////
    //----------------------------------------
    // Timer Control Register
    //----------------------------------------
    //
    MTU3.TCR.BIT.TPSC = 4;          // Timer count clock / Prescaler select:
                                    //   Internal clock 0: PCLK/1,   1: PCLK/4, 2: PCLK/16, 3: PCLK/64,
                                    //                 *4: PCLK/256, 5: PCLK/1024,
                                    //   External clock 6: TCLKA,    7: TCLKB
    MTU3.TCR.BIT.CKEG = 0;          // Clock edge by:    0:*rising edge, 1: falling edge, 2, 3: both edges
    MTU3.TCR.BIT.CCLR = 1;          // Counter clear by: 0: free run
    //----------------------------------------
    // Timer Mode Register
    //----------------------------------------
    MTU3.TMDR.BIT.MD  = 0;          // Mode of operation: 0: normal, 2: PWM, 4: phase measure mode1, etc.
    MTU3.TMDR.BIT.BFA = 0;          // Mode for TGRA: 0:*normal, 1: TGRC is a buffer of TGRA
    MTU3.TMDR.BIT.BFB = 0;          // Mode for TGRB: 0:*normal, 1: TGRD is a buffer of TGRB
    //            BFE none.
    //----------------------------------------
    // Timer I/O Control Register
    //----------------------------------------
    // High
    MTU3.TIORH.BIT.IOA = 0;         // 0: Prohibit ouput from TIOC0A
    MTU3.TIORH.BIT.IOB = 0;         // 0: Prohibit ouput from TIOC0B
    // Low
    MTU3.TIORL.BIT.IOC = 0;         // 0: Prohibit ouput from TIOC0C
    MTU3.TIORL.BIT.IOD = 0;         // 0: Prohibit ouput from TIOC0D
    //----------------------------------------
    // Timer Buffer Transfer Mode Register
    //----------------------------------------
    MTU3.TBTM.BYTE = 0;             // 0: Transfer at compare match A/B/E
    //----------------------------------------
    // Timer Counter Register
    //----------------------------------------
    MTU3.TCNT = 0x0000;
    //----------------------------------------
    // Timer General Register
    //----------------------------------------
    MTU3.TGRA = 0xFFFF;             // the same as init value
    MTU3.TGRB = 0xFFFF;             // the same as init value
    MTU3.TGRC = 0xFFFF;             // the same as init value
    MTU3.TGRD = 0xFFFF;             // the same as init value
    //----------------------------------------
    // Timer Interrupt Enable Register
    //----------------------------------------
    MTU3.TIER.BYTE  = 0;            // disable all MTU3-related interrupt
    //   TIER2 none
    //----------------------------------------
    // Timer Noise Filter Control Register
    //----------------------------------------
    MTU3.NFCR.BYTE = 0;             // disable for all MTICOC0<A/B/C/D> input
    //----------------------------------------
    // MTU3 Remains stopped here.
    //----------------------------------------

    ////////////////////////////////////////////////////////////////////////////////
    // ch4
    ////////////////////////////////////////////////////////////////////////////////
    //----------------------------------------
    // Timer AD Conversion Request Controll Register
    //----------------------------------------
    MTU4.TADCR.WORD = 0;        // MTU4, not MTU0.

    ////////////////////////////////////////////////////////////////////////////////
    // ch5
    ////////////////////////////////////////////////////////////////////////////////
    // Blank
}

//============================================================
// ADC
//============================================================
// Data:                12bit
// Ch count:            12
//                      4 of 16ch are not assigned for 64-pin chip.
//---
// AD Data Register:    ADDR0-ADDR15
//---
// Data Alignment:      right aligned in 16bit
// ADC start trigger:   TRG0EN(MTU0.TGRE compare match),
//                      1ms period
// Mode:                single scan
// Interrupt:           generate interrupt request S12ADI0
//                      on the end of scan
//---
// registers not used:
//  ADRD:   AD Self Diagnose Data register
//  ADOCDR: AD Internal Reference Data Register
//---
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
void ADC_init(void) {
    int n;

    MSTP_S12AD = 0;     // Release module stop

    //------------------------------------------------------------
    // AD Control (Scan) Register
    //------------------------------------------------------------
    S12AD.ADCSR.BIT.ADST    = 0;    // AD Start bit: 0: ADC stop, 1: ADC start
                                    // Stop while register setting.
    S12AD.ADCSR.BIT.DBLANS  = 0;    // Don't care in single scan.
    S12AD.ADCSR.BIT.GBADIE  = 0;    // Disable interrupt on the end of group B scan
    S12AD.ADCSR.BIT.DBLE    = 0;    // Not double trigger mode
    S12AD.ADCSR.BIT.EXTRG   = 0;    // Select synchronous trigger by MTU, ELC for ADC start
    S12AD.ADCSR.BIT.TRGE    = 0;    // Disable trigger for ADC start. Enable later.
    S12AD.ADCSR.BIT.ADIE    = 1;    // Enable Interrupt S12ADI0 on the end of scan
    S12AD.ADCSR.BIT.ADCS    = 0;    // Scan mode: 0:*single scan, 1: group scan, 2: continuous scan
    // S12AD.ADCSR.BIT.ADST         // AD Start bit: auto set/reset by the ADC circuit.
    //------------------------------------------------------------

    //------------------------------------------------------------
    // ADANSA: AD Analog channel select A
    //------------------------------------------------------------
    S12AD.ADANSA.WORD  = 0xffff;    //  Enable all 16ch, except:
    S12AD.ADANSA.BIT.ANSA5  = 0;    //  Disable
    S12AD.ADANSA.BIT.ANSA7  = 0;    //  Disable
    S12AD.ADANSA.BIT.ANSA14 = 0;    //  Disable
    S12AD.ADANSA.BIT.ANSA15 = 0;    //  Disable
    //------------------------------------------------------------

    //------------------------------------------------------------
    // ADANSB: AD Analog channel select B
    //------------------------------------------------------------
    S12AD.ADANSB.WORD = 0x0000;     // Not used in single scan.
    //------------------------------------------------------------

    //------------------------------------------------------------
    // ADADS: AD Add-mode channel select
    //------------------------------------------------------------
    S12AD.ADADS.WORD = 0x0000;      // Disable add-mode for all ch.
    //------------------------------------------------------------

    //------------------------------------------------------------
    // ADADC: AD Add count
    //------------------------------------------------------------
    S12AD.ADADC.BIT.ADC = 0;        // No addition.
    //------------------------------------------------------------

    //------------------------------------------------------------
    // ADCER: AD Control Extention Register
    //------------------------------------------------------------
    S12AD.ADCER.BIT.ADRFMT  = 0;    // AD Data Register Format:     right aligned
    S12AD.ADCER.BIT.ACE     = 1;    // Auto Clear Enable:           Enable
    S12AD.ADCER.BIT.DIAGVAL = 2;    // Self diagnose voltage value: (Vref x 1/2) (Not used)
    S12AD.ADCER.BIT.DIAGLD  = 1;    // Self diagnose voltage:       fixed (Not used)
    S12AD.ADCER.BIT.DIAGM   = 0;    // Self diagnose:               Disable
    //------------------------------------------------------------

    //------------------------------------------------------------
    // ADSTRGR: AD Start Trigger select Register
    //------------------------------------------------------------
    S12AD.ADSTRGR.BIT.TRSA  = 4;    // TRG0EN (MTU0/TRGE compare match)
    S12AD.ADSTRGR.BIT.TRSB  = 0;    // Group B trigger:             ADTRG0# (Not used)
    //------------------------------------------------------------

    //------------------------------------------------------------
    // ADEXICR: AD Extended Input Control Register
    //------------------------------------------------------------
    S12AD.ADEXICR.BIT.OCSAD = 0;    // Addtion mode of internal Vref:   No
    S12AD.ADEXICR.BIT.TSS   = 0;    // (Although RX220 does not have TSS bit.)
    S12AD.ADEXICR.BIT.OCS   = 0;    // AD convert internal Vref:        No  // Must be 0 for single scan mode.
    //------------------------------------------------------------

    //------------------------------------------------------------
    // ADSSTR: AD Sampling State Register
    //------------------------------------------------------------
    // Necessary:
    //  12 <= n <= 255
    //  T>0.4us  ;Sampling time T=n*(1/fclk)=n*50ns     // fclk=20MHz
    //------------------------------------------------------------
    // n=255, T=12.75us     // Tentative
    //------------------------------------------------------------
    n=255;
    S12AD.ADSSTR0   = n;    // AN000
    //
    S12AD.ADSSTRL   = n;    // AN008--AN015
    S12AD.ADSSTRT   = n;    // (Although RX220 does not have Temperature sensor.)
    S12AD.ADSSTRO   = n;    // internal vref
    //
    S12AD.ADSSTR1   = n;    // AN001
    S12AD.ADSSTR2   = n;    // AN002
    S12AD.ADSSTR3   = n;    // AN003
    S12AD.ADSSTR4   = n;    // AN004
    S12AD.ADSSTR5   = n;    // AN005
    S12AD.ADSSTR6   = n;    // AN006
    S12AD.ADSSTR7   = n;    // AN007
    //------------------------------------------------------------

    //------------------------------------------------------------
    // ADDISCR: AD Disconnect Detect Control Register
    //------------------------------------------------------------
    S12AD.ADDISCR.BIT.ADNDIS = 0;   // Assist Disconnect Detect: No
    //------------------------------------------------------------

}

void ADC_trigger_enable(int trig_enable) {
    if (trig_enable==1) {
        S12AD.ADCSR.BIT.TRGE  = 1;  // Enable  trigger for ADC start
    } else {
        S12AD.ADCSR.BIT.TRGE  = 0;  // Disable trigger for ADC start
    }
}

//------------------------------------------------------------
// Port Mode Register: reset all ports' mode to be general ports,
// temporarily.
//------------------------------------------------------------
void    PORT_PMR_reset(void)
{
    PORT0.PMR.BYTE      = 0;
    PORT1.PMR.BYTE      = 0;
    PORT2.PMR.BYTE      = 0;
    PORT3.PMR.BYTE      = 0;
    PORT4.PMR.BYTE      = 0;
    PORT5.PMR.BYTE      = 0;
    PORTA.PMR.BYTE      = 0;
    PORTB.PMR.BYTE      = 0;
    PORTC.PMR.BYTE      = 0;
    PORTD.PMR.BYTE      = 0;
    PORTE.PMR.BYTE      = 0;
    PORTH.PMR.BYTE      = 0;
    PORTJ.PMR.BYTE      = 0;
}

//------------------------------------------------------------
// MPC(Multi Pin Function Controller)
//------------------------------------------------------------
//
//  TXD1, RXD1
//  ADC001..015 (12ch in total)
//
void MPC_init(void)
{
    //------------------------------------------------------------
    // Write Protect Registser
    //------------------------------------------------------------
    MPC.PWPR.BIT.B0WI   = 0;    // Enable writing PFSWE bit
    MPC.PWPR.BIT.PFSWE  = 1;    // Enable writing PFS register
    //------------------------------------------------------------

    //============================================================
    // Set pin: SCI/TXD1, RXD1
    //============================================================
    MPC.P26PFS.BIT.PSEL = 0xA;  // Pin select: TXD1
    //--------------------
    MPC.P30PFS.BIT.PSEL = 0xA;  // Pin select: RXD1
    MPC.P30PFS.BIT.ISEL = 0;    // Interrupt select: not used as IRQ0 input.
    //------------------------------------------------------------

    //============================================================
    // Set pin: ADC/AN000..AN013
    //============================================================
    // Port 4   // ASEL: Analog select  1/0: analog/not
    MPC.P40PFS.BIT.ASEL = 1;    //  AN000
    MPC.P41PFS.BIT.ASEL = 1;    //  AN001
    MPC.P42PFS.BIT.ASEL = 1;    //  AN002
    MPC.P43PFS.BIT.ASEL = 1;    //  AN003
    MPC.P44PFS.BIT.ASEL = 1;    //  AN004
    // MPC.P45PFS.BIT.ASEL = 1;    //  AN005 NA
    MPC.P46PFS.BIT.ASEL = 1;    //  AN006
    // MPC.P47PFS.BIT.ASEL = 1;    //  AN007 NA
    //---
    // Port E   // ASEL: Analog select  1/0: analog/not
    MPC.PE0PFS.BIT.ASEL = 1;    //  AN008
    MPC.PE1PFS.BIT.ASEL = 1;    //  AN009
    MPC.PE2PFS.BIT.ASEL = 1;    //  AN010
    MPC.PE3PFS.BIT.ASEL = 1;    //  AN011
    MPC.PE4PFS.BIT.ASEL = 1;    //  AN012
    MPC.PE5PFS.BIT.ASEL = 1;    //  AN013
    // MPC.PE6PFS.BIT.ASEL = 1;    //  AN014 NA
    // MPC.PE7PFS.BIT.ASEL = 1;    //  AN015 NA
    //---
    // Port 1-6
    MPC.P16PFS.BIT.PSEL = 0;    // Pin select: Hi-z,  not used as ADTRG0#
    //------------------------------------------------------------

    //------------------------------------------------------------
    // Write Protect Registser
    //------------------------------------------------------------
    MPC.PWPR.BIT.PFSWE  = 0;    // Prohibit writing PFS register
    MPC.PWPR.BIT.B0WI   = 1;    // Prohibit writing PFSWE bit
    //------------------------------------------------------------
}

//------------------------------------------------------------
// Port Mode Register: set ports' mode to be peripheral
//------------------------------------------------------------
void    PORT_PMR_set(void)
{
    // Set '1', if a pin to be used as some specific peripheral,
    // other than general port.

    // SCI1
    PORT2.PMR.BIT.B6    = 1;    // SCI1/TXD.
    PORT3.PMR.BIT.B0    = 1;    // SCI1/RXD.

    // ADC
    PORT4.PMR.BYTE      = 0x5F; //  0101_1111: AN ch:[x6x4_3210]
    PORTE.PMR.BYTE      = 0x3F; //  0011_1111: AN ch:[xxDC_BA98]
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
    // はじめはLOCO(125kHz)が動いている。
    //
    //----------------------------------------------------------------------
    // Main OSC Force Control Register
    SYSTEM.MOFCR.BIT.MODRV  = 0x00;     // 0: Not ceramic oscillator
    SYSTEM.MOFCR.BIT.MODRV2 = 0x03;     // 11b: 16MHz..20MHz
    SYSTEM.MOFCR.BIT.MOSEL  = 0;        // 0: Xtal, not external input
    //----------------------------------------------------------------------
    // Main OSC Wait Control Register
    SYSTEM.MOSCWTCR.BIT.MSTS = 0x0E;    // 01110b: (1/20MHz)*262144=13.11ms
    //----------------------------------------------------------------------
    // Main OSC Control Register:       RUN Main clock   (20MHz)
    //                          (0/1)=(run/stop)
    SYSTEM.MOSCCR.BIT.MOSTP     = 0;
    //----------------------------------------------------------------------
    // ★SCKCR, SCKCR3は ReadCheck してから進む。
    //----------------------------------------------------------------------
    // Select each divider
    SYSTEM.SCKCR.BIT.PCKD   = 0x00;     // PCLKD(Peripheral clock D; ADC module clock): Ndiv=1
        while(SYSTEM.SCKCR.BIT.PCKD!=0x00);
    SYSTEM.SCKCR.BIT.PCKB   = 0x00;     // PCLKB(Peripheral clock B; modules but ADC):  Ndiv=1
        while(SYSTEM.SCKCR.BIT.PCKB!=0x00);
    SYSTEM.SCKCR.BIT.BCK    = 0x01;     // External Bus clock:  Ndiv=2
        while(SYSTEM.SCKCR.BIT.BCK!=0x01);
    SYSTEM.SCKCR.BIT.ICK    = 0x00;     // ICLK(System clock):  Ndiv=1
        while(SYSTEM.SCKCR.BIT.ICK!=0x00);
    SYSTEM.SCKCR.BIT.FCK  = 0x00;     // FlashIF clock:  Ndiv=1
        while(SYSTEM.SCKCR.BIT.FCK!=0x00);
    //----------------------------------------------------------------------
    // Select Clock source:             SELECT MAIN
    SYSTEM.SCKCR3.BIT.CKSEL = 0x02;     // 0x00: LOCO (after reset)
                                        // 0x01: HOCO (Freq accuracy: +/- 1%)
                                        // 0x02:*MAIN 20MHz Xtal
                                        // 0x03: SUB
        while(SYSTEM.SCKCR3.BIT.CKSEL!=0x02);
    //----------------------------------------------------------------------
    SYSTEM.HOCOCR2.BIT.HCFRQ    = 0;    // HOCO control register2: freq select
                                        // 0/1/2/3 = {32/36.864/40/50} [MHz]
    //----------------------------------------------------------------------
    // OSC Control Register:            STOP all clock sources except main
    //                          (0/1)=(run/stop)
    SYSTEM.SOSCCR.BIT.SOSTP     = 1;    // Sub clock    (32.768kHz)
    SYSTEM.LOCOCR.BIT.LCSTP     = 1;    // LOCO
                                        // Below are the same as reset.
    SYSTEM.ILOCOCR.BIT.ILCSTP   = 1;    // IWDT(Independant Watch Dog Timer) clock
    SYSTEM.HOCOCR.BIT.HCSTP     = 1;    // HOCO         (32--50MHz; +/-1%)
    //----------------------------------------------------------------------
    // HOCO Power Control Register
    SYSTEM.HOCOPCR.BIT.HOCOPCNT = 1;    // HOCO Power off
    //----------------------------------------------------------------------

    ////////////////////////////////////////////////////////////////////////
    PORT_PMR_reset();   // Port Mode Register: reset to use all pins as ports, temporarily.

        PORT_init();
        PORT_set_input_for_ADC();

        MTU2_init();
        MTU2_ch0_restart();

        CMT_init();

        SCI_init();

        ADC_init();

        MPC_init(); // Multi Pin Funciton Controller
    PORT_PMR_set(); // Port Mode Register: set to use specified perpherals' interface.
    ////////////////////////////////////////////////////////////////////////

    //----------------------------------------------------------------------
    // Protect Control Register
    //----------------------------------------------------------------------
    SYSTEM.PRCR.WORD        = (PRCR_PRKEY<<8)+0x00;     // Write-protect all

    //----------------------------------------------------------------------
    i2c_init();
    OLED_init();
    LCD_AQM_init();
}

void    gvar_init(void)
{
    g_ms_abs_count  = 0;
    g_ms_count      = 0;
    g_led0          = 0;
    g_led1          = 0;
}

void    test_func(int n)    // for stack address monitor
{
    int depth;

    depth=n;
    SCIprintf("test_func(): depth=%d, &depth=%0Xh\n", depth, &depth);
    if (depth<8) {
        test_func(depth+1);
    }
    SCIprintf("return, depth=%d\n", depth);
}

void main(void)
{
int     sw_slide;
int     f_out_mon;
int     *intp_monitor_min_save;
int     dft_n;
//int     dft_n_prev;

int         i;
volatile    int j;
int     preset_no;

int     ms_abs_count_prev;

    //============================================================
    // Interrupt disable
    //============================================================
    IEN(CMT1,   CMI1)       = 0;    // Disable CMT1 interrupt
    IEN(MTU0,   TGIA0)      = 0;    // Disable MTU0 interrupt
    IEN(S12AD,  S12ADI0)    = 0;    // Disable S12AD interrupt
    //------------------------------------------------------------

    //============================================================
    // Init
    //============================================================
    gvar_init();
    hwsetup();

    //============================================================
    // Interrupt Controller setting
    //============================================================
    // Interrupt Priority Level
    //  0:      prohibit interrupt
    //  1--15:  larger --> higher priority
    //------------------------------------------------------------
    IPR(MTU0,   TGIA0)      = 2;
    IPR(S12AD,  S12ADI0)    = 3;
    //------------------------------------------------------------
    // Interrupt enable
    //------------------------------------------------------------
    IEN(S12AD,  S12ADI0)    = 1;    // Enable S12AD interrupt
    //------------------------------------------------------------

    ADC_trigger_enable(1);

    //------------------------------------------------------------
    // Program main
    //------------------------------------------------------------
    // OLED test
    OLED_locate(0,  0);     OLED_wrstr("0123456789ABCDEF");
    OLED_locate(1,  0);     OLED_wrstr("Hello,");

    LCD_AQM_locate(0,  0);  LCD_AQM_wrstr("0123456789ABCDEF");
    LCD_AQM_locate(1,  0);  LCD_AQM_wrstr("World.");

    // Variable size
    SCIprintf("sizeof(short) =%d\n", sizeof(short));
    SCIprintf("sizeof(int)   =%d\n", sizeof(int));
    SCIprintf("sizeof(int*)  =%d\n", sizeof(int*));
    SCIprintf("sizeof(long)  =%d\n", sizeof(long));
    SCIprintf("sizeof(float) =%d\n", sizeof(float));
    SCIprintf("sizeof(double)=%d\n", sizeof(double));
    if (sizeof(short)!=2) {
        SCIprintf("WARNING: sizeof(short)!=2.\n");
    }
    if (sizeof(double)!=8) {
        SCIprintf("WARNING: sizeof(double)!=8.\n");
    }
    // test_func(1);
    intp_monitor_min_save = (int*)-1;   // to differ from g_intp_monitor_min.
    //--------------------------------------------------------------------------------
    dft_address_show();
    fft_address_show();
    //--------------------------------------------------------------------------------
    stack_var_adrs_min_update();
    stack_var_adrs_min_show();

    //--------------------------------------------------------------------------------
    // MTU/ch3 TCNT test
    SCIprintf("MTU/ch3 TCNT test--------------------\n");
    for (i=0;i<8;i++) {
        ms_abs_count_prev = g_ms_abs_count;
        while (g_ms_abs_count==ms_abs_count_prev);
        ms_abs_count_prev = g_ms_abs_count;
        tcnt_restart();

        while (g_ms_abs_count==ms_abs_count_prev);
        tcnt_show("test", 1);
    }

    //--------------------------------------------------------------------------------
    // DFT/FFT test
switch(0) {
    default:
        // Skip DFT/FFT test
        break;
    case 1:
        //--------------------------------------------------------------------------------
        // Test of FFT over dft_n, using radix default
        for (dft_n=128;dft_n>=16;dft_n-=4) {
            f_out_mon = SLIDE_SWITCH;
            dft_init(dft_n);
            fft_real_n_init(dft_n);
            //dft_test(0, 1);     // DFT RAW
            dft_test(1, f_out_mon);     // DFT with cos/sin table
            fft_real_radix_mon(0);
            if (fft_real_default_n_available(dft_n)) {
                dft_test(2, f_out_mon);     // FFT
            }
            stack_var_adrs_min_show();
        }
        //--------------------------------------------------------------------------------
        break;
    case 2:
        //--------------------------------------------------------------------------------
        // FFT Tests over radix presets (for deciding radix default)
        //dft_n_prev = 0;
        for (i=0;i<=127;i++) {
            preset_no = i;
            //--------------------------------------------------------------------------------
            fft_real_init_by_preset(preset_no);
            dft_n = fft_real_get_npoint();
            dft_init(dft_n);
            // fft_real_radix_preset_tbl_mon(-1);   // -1: show all

            //if (dft_n==dft_n_prev) continue;
            //dft_test(0, 0);  // DFT RAW
            //dft_test(1, 0);  // DFT with cos/sin table
            fft_real_radix_preset_tbl_mon(preset_no, 0);
            // fft_real_radix_tbl_mon();
            dft_test(2, 0);     // 2:FFT, 1: show result
            //dft_n_prev = dft_n;
            stack_var_adrs_min_show();
        }
        break;
}

    //------------------------------------------------------------
    // Main loop
    //------------------------------------------------------------
    while(1) {
        sw_slide = SLIDE_SWITCH;
        switch(sw_slide) {
            case 0:
                g_led0 = (g_ms_count>=500) ? 1 : 0;
                break;
            default:
                g_led0 = 0;
                break;
        }
        g_led1 = (g_ms_count<500) ? 1 : 0;

        // Stack variable pointer monitor
        if (intp_monitor_min_save != g_intp_monitor_min) {
            intp_monitor_min_save = g_intp_monitor_min;
            SCIprintf("g_intp_monitor_min=%0Xh\n", intp_monitor_min_save);
        }

        // Test PORTB
        // for (i=0;i<100;i++) {
        //     PORTB.PODR.BIT.B0 = 1;
        //     PORTB.PODR.BIT.B3 = 0;
        //     for (j=0;j<1000;j++);
        //     PORTB.PODR.BIT.B0 = 0;
        //     PORTB.PODR.BIT.B3 = 1;
        //     for (j=0;j<2000;j++);
        // }

        // i2c_dbg_delay_test();   // NEVER RETURN.
    }
}

#ifdef __cplusplus
void abort(void)
{

}
#endif
