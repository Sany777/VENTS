#ifndef MAIN_H_
#define MAIN_H_

#ifdef QUARTZ_32768    
	#define  F_CPU 1000000UL     // inverted fuses: CSEL3=1 CSEL0=1 CKOPT=0
#else
	#define  F_CPU 4000000UL    // inverted fuses: CSEL3=1, CSEL2=1 CKOPT=0
#endif


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>

//--------------------------- BASIC
#define ON  1
#define OFF 0

#define TRUE	 1
#define FALSE	 0
#define NONE	 11
#define MAX_MIN_SEC 59
#define MAX_HOUR 23
#define SIZE_BYTE 8
#define MAX_DIGITS  6

//--------------------------- CONFIG
#define TIME_SIGNAL_ON    10   
#define SIGNAL_TIME       2
#define TIME_SIGNAL_OFF (TIME_SIGNAL_ON-SIGNAL_TIME)
#define MINIMUM_TIME 10


//--------------------------- TIMING
#define PRESCALLER_F_CPU   1024
#define TIMING_HALF_SEC   ((F_CPU/(PRESCALLER_F_CPU*2))-1)
#define BUTTON_DELAY      F_CPU/1700				


//------------------------- EEPROM
#define ADDR_SEC  1
#define ADDR_MIN  2
#define ADDR_HOUR 3


enum SPI_digits {
	BLINK_FIRST_POINTS,
	BLINK_SECOND_POINTS,
	CONVEER_1,
	CONVEER_2,
	SIGNAL_1,
	SIGNAL_2,
};

enum Button_press {
	UNPRESS,
	PRESS_STOP,
	PRESS_START,
	PRESS_SETTING
};

enum Mode{
	READY,
	READ_SETUP,
	EDITING_SEC,
	EDITING_MIN,
	EDITING_HOUR,
	WRITE_SETUP
};

#define	 buton_stop		(!(PINC&(1<<2)))
#define  buton_start    (!(PINC&(1<<3)))
#define  buton_set      (!(PINC&(1<<4)))
#define  voltage_state  (!(PINC&(1<<5)))

#define start_Transmision_Spi   (PORTD&=~(1<<5))

#define end_Transmision_Spi		PORTD|=(1<<7);\
								PORTD&=~(1<<7);

#define send_CLK				PORTB|=(1<<0);\
								PORTB&=~(1<<0)
								
#define send_1					(PORTD|=(1<<6))
#define send_0					(PORTD&=~(1<<6))

#define active_Load				(byte|=(1<<7))


void read_m (void);
void send_to_SPI (int8_t *numbers) ;
void set_digits_numbers(int8_t *numbers);
void execute(const uint8_t but);
void port_ini (void);
void timer_init (void);
uint8_t get_button (void);
void timer_init (void);



#endif /* MAIN_H_ */