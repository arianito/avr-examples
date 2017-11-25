#include <mega16.h>
#include <i2c.h>
#include <alcd.h>
#include <stdio.h>



unsigned char buffer[16];
unsigned char page = 0;
unsigned char key = 0;
unsigned char last_key=0xff;

bit update = 1;

unsigned long millis = 0;
unsigned long last_time = 0;


#define F_CPU 8000000
#define GO(p) page=p; update=1;
#define UP() update = 1;

#define DIS(p) if(page == p && update) {
#define SID() update = 0; }

#define KBD() key = read(); if(key != 0xff && key != last_key) {
#define DBK() last_key = key; } if (key == 0xff) last_key=0xff;

#define KEY_ON 10
#define KEY_EQL 11
#define KEY_ADD 12
#define KEY_SUB 13
#define KEY_MUL 14
#define KEY_DIV 15

#define D_READ   0xD1u
#define D_WRITE  0xD0u

#define D_SEC	0x00u
#define D_DATE	0x04u
#define D_CR	0x07u

typedef struct
{
  unsigned char sec;
  unsigned char min;
  unsigned char hour;
  unsigned char weekDay;
  unsigned char date;
  unsigned char month;
  unsigned char year;
} rtc;

void set(rtc* date, unsigned char index, unsigned char value) {
	switch(index){
		case 0:
			date->hour = value;
			break;
		case 1:
			date->min = value;
			break;
		case 2:
			date->sec = value;
			break;
		case 3:
			date->year = value;
			break;
		case 4:
			date->month = value;
			break;
		case 5:
			date->date = value;
			break;
	}
}

unsigned char get(rtc* date, unsigned char index) {
	switch(index){
		case 0:
			return date->hour;
		case 1:
			return date->min;
		case 2:
			return date->sec;
		case 3:
			return date->year;
		case 4:
			return date->month;
		case 5:
			return date->date;
	}
}
#define HI_NIBBLE(b) (((b) >> 4) & 0x0F)
#define LO_NIBBLE(b) ((b) & 0x0F)

unsigned char bcd_to_byte(unsigned char bcd){
    return (char)((HI_NIBBLE(bcd)*10)+(LO_NIBBLE(bcd)));
}

unsigned char byte_to_bcd(unsigned char byte){
    return (char) ((byte / 10)*16)+(byte % 10);
}


void rtc_init() {
	i2c_start();
	i2c_write(D_WRITE);
	i2c_write(D_CR);
	i2c_write(0x00);
	i2c_stop();
}

rtc rtc_get() {
	rtc a;
	i2c_start();
	i2c_write(D_WRITE);
	i2c_write(D_SEC);
	i2c_stop();

	i2c_start();
	i2c_write(D_READ);

	a.sec = bcd_to_byte(i2c_read(1));
	a.min = bcd_to_byte(i2c_read(1));
	a.hour = bcd_to_byte(i2c_read(1));
	a.weekDay = bcd_to_byte(i2c_read(1));
	a.date = bcd_to_byte(i2c_read(1));
	a.month = bcd_to_byte(i2c_read(1));
	a.year =bcd_to_byte(i2c_read(0));

	i2c_stop();

	return a;
}

void rtc_set(const rtc* a) {
	i2c_start();

	i2c_write(D_WRITE);
	i2c_write(D_SEC);

	i2c_write(byte_to_bcd(a->sec));
	i2c_write(byte_to_bcd(a->min));
	i2c_write(byte_to_bcd(a->hour));
	i2c_write(byte_to_bcd(a->weekDay));
	i2c_write(byte_to_bcd(a->date));
	i2c_write(byte_to_bcd(a->month));
	i2c_write(byte_to_bcd(a->year));

	i2c_stop();
}

unsigned char read() {
	const unsigned char map[] = {10,0,11,12,1,2,3,13,4,5,6,14,7,8,9,15};
	unsigned char r, c;
	PORTD |= 0x0F;
	for(c=0;c<4;c++){
		DDRD = 0x00;
		DDRD |= 0x80 >> c;
		for(r=0;r<4;r++){
			if(!(PIND & (0x08 >> r))) {
				return map[r*4+c];
			}
		}
	}
	return 0xFF;
}


void write_line(unsigned char line, const char* str) {
	lcd_gotoxy(0,line);
	lcd_puts("                ");
	lcd_gotoxy(0,line);
	lcd_puts(str);
}


void timer_init() {
	TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
	TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (0<<CS12) | (0<<CS11) | (1<<CS10);
	TCNT1H=0xE0;
	TCNT1L=0xC0;
	ICR1H=0x00;
	ICR1L=0x00;
	OCR1AH=0x00;
	OCR1AL=0x00;
	OCR1BH=0x00;
	OCR1BL=0x00;
	TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (1<<TOIE1) | (0<<OCIE0) | (0<<TOIE0);
}



interrupt [TIM1_OVF] void timer1_ovf_isr(void) {
	TCNT1H=0xE0C0 >> 8;
	TCNT1L=0xE0C0 & 0xff;

	millis++;
}

void main(void) {
	rtc now = {0,0,0,0,0,0,0};
	unsigned char current = 0;
	bit flag = 0;

	lcd_init(16);
	i2c_init();
	rtc_init();
	timer_init();


	#asm("sei")


	while (1) {
		DIS(0);
		write_line(0,"Aryan Say Hello!");
		write_line(1, "Press [ON]");
		SID();


		DIS(1);
		lcd_gotoxy(0,0);
		sprintf(buffer, "%02d:%02d:%02d        ",now.hour, now.min, now.sec);
		lcd_puts(buffer);
		lcd_gotoxy(0,1);
		lcd_puts("20");
		sprintf(buffer, "%02d/%02d/%02d    ",now.year, now.month, now.date);
		lcd_puts(buffer);
		SID();

		DIS(2);
		// 00:00:00
		lcd_gotoxy(0,0);
		sprintf(buffer, "%02d:%02d:%02d        ",now.hour, now.min, now.sec);
		if(current >= 0 && current < 3 && flag){
			buffer[current*3] = ' ';
			buffer[current*3+1] = ' ';
		}
		lcd_puts(buffer);
		lcd_gotoxy(0,1);
		lcd_puts("20");
		sprintf(buffer, "%02d/%02d/%02d    ",now.year, now.month, now.date);
		if(current >= 3 && current < 6 && flag){
			buffer[(current-3)*3] = ' ';
			buffer[(current-3)*3+1] = ' ';
		}
		lcd_puts(buffer);
		SID();

		switch(page) {
			case 0:
				KBD();
				if(key == KEY_ON){
					GO(1);
				}
				DBK();
				break;
			case 1:
				if(millis - last_time > 1000) {
					now = rtc_get();

					last_time = millis;
					UP();
				}
				KBD();
				if(key == KEY_EQL){
					GO(2);
				}
				DBK();
				break;
			case 2:
				if(millis - last_time > 20) {
					flag = !flag;
					last_time = millis;
					UP();
				}
				KBD();
				if(key == KEY_SUB && current > 0){
					current --;
					UP();
				}
				else if(key == KEY_ADD && current < 5){
					current ++;
					UP();
				}else if(key == KEY_EQL){
					rtc_set(&now);
					GO(1);
				}else if (key >= 0 && key <= 9){
					unsigned char nw = get(&now, current);
					unsigned char temp = 0;
					if(nw > 9) nw =0;
					temp = nw * 10 + key;

					if((current == 0 && temp > 24) || (current == 1 && temp > 60) || (current == 2 && temp > 60) || (current == 3 && temp > 99) || (current == 4 && temp > 12) || (current == 5 && temp > 31))
						temp = 0;

					set(&now, current,  temp);
				}

				DBK();
				break;
		}
	}
}
