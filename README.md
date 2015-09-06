# rx220_sample
Small sample C program as a HEW project for Akizuki AE-RX220 development board
------
<2nd sample>
- MTU2 generates 1ms signal and interrupt
- ADC converts 12 ch of analog input in scan mode 
- ADC generates an interrupt on the end of scan
- Interupt handler send a char of hex for MSB 4bit of the AD value(Ch. 0)
  every 2ms

- LED1/2 on/off with 1s period
- Slide switch controls LED2
-----
Blog: http://solar-club.jp/member/radioshack16/
-----
