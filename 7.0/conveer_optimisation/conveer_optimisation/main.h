#ifndef MAIN_H_
#define MAIN_H_


#define  F_CPU 1000000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/eeprom.h>


#define ON  1
#define OFF 0

#define TRUE  1
#define FALSE 0
#define None 11
#define CICLE 50

//
#define TO_DEC 10
#define SIGNAL_TO_LOAD_ON 10        
#define ALLOW_MINIMUM_DELAY_TIMER 10
#define DIGITS_MAX  6
#define RESPONSE    10				// delay response button

//------------------------- EEPROM
#define ADDR_SEC  1
#define ADDR_MIN  2
#define ADDR_HOUR 3


enum SPI_digits {
	BLINK_FIRST,
	BLINK_SECOND,
	RESERV_SPI,
	SIGNAL_SPI,
	CONVEER_SPI,
	BLINK_BUTTON_SPI
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


volatile _Bool  
				conveer = OFF, 
				signale = OFF, 
				blink = FALSE, 
				timer_run = OFF, 
				signal_allowed = FALSE, 
				voltage_f = TRUE;
					
int8_t sek   = 0, 
		min   = 0, 
		hour  = 0, 
		setup = READY;


void read_m (void);
void SPI (int8_t *numbers) ;
void set_digits_numbers(int8_t *numbers);
void execute(const int8_t but);
void port_ini (void);
void timer_init (void);
int8_t get_button (void);
int8_t getKey(void);


#endif /* MAIN_H_ */