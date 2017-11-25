#include <mega16.h>
#include <delay.h>
#include <stdio.h>
#include <alcd.h>

typedef unsigned long ulong;

typedef unsigned int uint;

typedef unsigned int uchar;

ulong glob = 0;
ulong last_m = 0;
char buffer[16];
ulong pc[2];
ulong pc_m[2];
bit c = 0;
ulong tick = 0, tick2 = 0;
float freq = 0, freq_s = 0;
float dc = 0, dc_s = 0;


float lerp(float a, float b, float dt){
    return a + (b-a) * dt;
}

interrupt [EXT_INT0] void ext_int0_isr(void)
{
    pc[c] = tick - pc_m[!c];
    pc_m[c] = tick;    
    c = !PIND.2;  
    tick2++;
}

interrupt [TIM1_OVF] void timer1_ovf_isr(void)
{
    TCNT1H=0xE0C0 >> 8;
    TCNT1L=0xE0C0 & 0xff;
    glob++;
}

void main(void)
{

        
    // Timer/Counter 1 initialization
    // Clock source: System Clock
    // Clock value: 8000.000 kHz
    // Mode: Normal top=0xFFFF
    // OC1A output: Disconnected
    // OC1B output: Disconnected
    // Noise Canceler: Off
    // Input Capture on Falling Edge
    // Timer Period: 1 us
    // Timer1 Overflow Interrupt: On
    // Input Capture Interrupt: Off
    // Compare A Match Interrupt: Off
    // Compare B Match Interrupt: Off
    TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
    TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10);
    TCNT1H=0xFF;
    TCNT1L=0xF8;
    ICR1H=0x00;
    ICR1L=0x00;
    OCR1AH=0x00;
    OCR1AL=0x00;
    OCR1BH=0x00;
    OCR1BL=0x00;


    // Timer(s)/Counter(s) Interrupt(s) initialization
    TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (1<<TOIE1) | (0<<OCIE0) | (0<<TOIE0);

    // External Interrupt(s) initialization
    // INT0: On
    // INT0 Mode: Any change
    // INT1: Off
    // INT2: Off
    GICR|=(0<<INT1) | (1<<INT0) | (0<<INT2);
    MCUCR=(0<<ISC11) | (0<<ISC10) | (0<<ISC01) | (1<<ISC00);
    MCUCSR=(0<<ISC2);
    GIFR=(0<<INTF1) | (1<<INTF0) | (0<<INTF2);

    lcd_init(16);
                 
    #asm("sei")

    while (1)
    {
        if(glob - last_m >= 1000){  
                
            dc = ((float)pc[0]/(float)(pc[0]+pc[1]))*100.0f;
            freq = (float)tick2 / 2.0f;
            tick2 = 0;
            last_m = glob;
        }
        if(glob % 100 == 0){     
            freq_s = lerp(freq_s, freq, 0.6f);  
            dc_s = lerp(dc_s, dc, 0.6f);  
            
            lcd_gotoxy(0,0);
            sprintf(buffer, "dc: %f", dc_s); 
            lcd_puts(buffer);                        
            lcd_gotoxy(0,1); 
              
            
            sprintf(buffer, "freq: %f", freq_s);
            lcd_puts(buffer);
            
        }      
        
        tick++;
    }
}
