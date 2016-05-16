# rx220_sample
Small sample C program as a HEW project for Akizuki AE-RX220 development board
------
<3rd sample>
- DFT/FFT functions added.
  On power up, DFT and/or FFT is tested and processing time is shown via serial port.
  FFT function spec:
    input:          real only
    output:         0 to Nuquist frequency
    precision:      double(8 or 4byte)
    (Renesas tool HEW can handle double type as either 8 or 4 byte.)
    N supoortd:     N=4*(2^p)(3^q)(5^r)(7^s),
                    where 4<= N <=128 and
                    p, q, r and s is an integer >= 0.
    desimation type: decimation-in-time
    radix:          mixed radix
        intermediate stage: 8, 7, 5, 4, 3, 2
        terminal stage:     16, 8, 5, 4, 3, 2
    implementation: recursively implemented:
        a core function calls one of radix-specific functions
        depending on its input parameter M, and the called
        radix-specific function calls the core function
        to perform sub-M FFT.
------
Other functions: (the same as <2nd sample>)
- MTU2 generates 1ms signal and interrupt
- ADC converts 12 ch of analog input in scan mode
- ADC generates an interrupt on the end of scan
- Interupt handler send a char of hex for MSB 4bit of the AD value(Ch. 0)
  every 2ms
- LED1/2 on/off with 1s period
- Slide switch controls LED2

-----
Blog : http://solar-club.jp/member/radioshack16/
-----
