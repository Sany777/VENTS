#ifndef MAIN_H_
#define MAIN_H_

// --------------------- select quartz
#ifdef QUARTZ_32768    
	#define  F_CPU 1000000UL
#else
	#define  F_CPU 4000000UL
#endif


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>


#define ON  1
#define OFF 0

#define TRUE  1
#define FALSE 0
#define None 11

#define SIZE_BYTE 8
#define SIZE_TIME_CUR 3

#define HALF_SEC_4M 1944
#define HALF_SEC_8M HALF_SEC_4M*2


#define QUARTZ HALF_SEC_4M



#define TO_DEC 10

#define MAX_MIN_SEC 59
#define MAX_HOUR 23

#define SIGNAL_TO_LOAD_ON 10        
#define ALLOW_MINIMUM_DELAY_TIMER 10
#define MAX_DIGITS  6


#define DELAY_BUTTON 10000
#define RESPONSE    2000			// delay response button

//------------------------- EEPROM
#define ADDR_SEC  1
#define ADDR_MIN  2
#define ADDR_HOUR 3


enum SPI_digits {
	BLINK_FIRST_POINTS,
	BLINK_SECOND_POINTS,
	BIT_3,
	BIT_4,
	SIGNAL,
	CONVEER,
};

enum Button_press {
	UNPRESS,
	PRESS_STOP,
	PRESS_START,
	PRESS_SETTING
};

enum Mode_menu{
	EDITING_SEC,
	EDITING_MIN,
	EDITING_HOUR,
	WRITE_SETUP,
	READY,
	READ_SETUP,
};

enum Index_in_time_current{
	SEC,
	MIN,
	HOUR
};

#define	 buton_stop		(!(PINC&(1<<2)))
#define  buton_start    (!(PINC&(1<<3)))
#define  buton_set      (!(PINC&(1<<4)))
#define  voltage_state  (!(PINC&(1<<5)))

#define start_Transmision_Spi   (PORTD&=~(1<<5))

#define end_Transmision_Spi		PORTD|=(1<<7);\
								PORTD&=~(1<<7);\

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
uint8_t getKey(void);
void timer_init (void);



#endif /* MAIN_H_ */