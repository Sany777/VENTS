#include "main.h"

volatile _Bool
		conveer = OFF,
		signale = OFF,
		blink = FALSE,
		timer_run = OFF,
		signal_allowed = FALSE,
		voltage_f = TRUE;
uint8_t	setup = READY;
			
int8_t time_current[SIZE_TIME_CUR];

int main (void)
{
		port_ini ();
		timer_init();
		read_m ();
		int8_t numbers[MAX_DIGITS]={None,None,None,None,None,None};
		sei();
															
	while (1)
	{
		send_to_SPI(numbers);
		execute(get_button());											
		set_digits_numbers(numbers);
	}																		
}



int8_t getCharSegment(int8_t n)
{
	switch(n)
	{
		case 1:  return  0b00000110; break;
		case 2:  return  0b01011011;  break;
		case 3:  return  0b01001111;  break;
		case 4:  return  0b01100110;  break;
		case 5:  return  0b01101101;  break;
		case 6:  return  0b01111101;  break;
		case 7:  return  0b00000111;  break;
		case 8:  return  0b01111111;  break;
		case 9:  return  0b01101111;  break;
		case 0:  return  0b00111111;  break;
		default: return  0;           break;
	}
}

void send_to_SPI (int8_t *numbers) 
{
	cli();
	for (int8_t digit = 0,byte = 0; digit<MAX_DIGITS; digit++) 
	{
		if (voltage_f) 
		{
			byte = getCharSegment(numbers[digit]);
			// ---------------------------------- control load
			switch(digit)
			{
				case BLINK_FIRST_POINTS  : if(timer_run == OFF || blink && (time_current[HOUR] || time_current[MIN])) active_Load; break;	
				case BLINK_SECOND_POINTS : if(timer_run == OFF || blink && time_current[HOUR]) active_Load; break;
				case CONVEER			 : if(conveer == ON) active_Load; break;
				case SIGNAL				 : if(signale == ON) active_Load; break;
				default                  : break;
			}
		}
		else 
		{
		// ---------------------------------- show "OFF"
			 if (digit == 3)      
					byte = 0X3F;          
			 else if (digit == 2) 
					byte = 0X71;
			 else if (digit == 1) 
					byte = 0X71;
			 else  
					byte = 0;
		}
		//---------------------------------- send to SPI
		for (uint8_t i=0; i<SIZE_BYTE; i++)
		{
			if (byte&0x80) 
				send_1;
			else 
				send_0;
			byte = (byte<<1);
			send_CLK;
		}
	}
	end_Transmision_Spi;
	sei();
}
	


void set_digits_numbers(int8_t *numbers)
{
	for (int8_t digit=0; digit<MAX_DIGITS; )
	{
		int8_t cur_time_member = digit/2;
		 numbers[digit++] =  cur_time_member == setup && blink ? None : time_current[cur_time_member]%10;
		 numbers[digit++] =  cur_time_member == setup && blink ? None : time_current[cur_time_member]/10;
	}
	
	if (timer_run)
	{
		for (int8_t digit=5; digit && numbers[digit]; digit--)
		{
			numbers[digit] = None;     
		}
	}
}


void EEPROM_WRITE (uint16_t uiAddress, int8_t ucData)
{
	while (EECR&(1<<EEWE));
	EEAR = uiAddress;
	EEDR = ucData;
	EECR |= (1<<EEMWE);
	EECR |= (1<<EEWE);
}


int8_t EEPROM_read(uint16_t uiAddress)
{
	while(EECR & (1<<EEWE));
	EEAR = uiAddress;
	EECR |= (1<<EERE);
	return EEDR;
}

void read_m (void)
{
	int8_t temp;
	for(uint8_t i=0; i<SIZE_TIME_CUR; i++)
	{
		temp = EEPROM_read(i+ADDR_SEC);
		if(temp > MAX_MIN_SEC || temp < 0)temp = 5;
		time_current[i] = temp;
	}
	if (time_current[HOUR] || time_current[MIN]) signal_allowed = TRUE;
	else signal_allowed = FALSE;
}
						


void port_ini (void)
{
		
	//---------------------- program SPI : 0-6 bit - show number, 7bit - control load
	DDRD|=(1<<6);     //DS
	PORTD&=~(1<<6);   // set 0
	DDRB|=(1<<0);     //clk
	PORTB&=~(1<<0);   // 
	DDRD|=(1<<7);     // ST 
	PORTD&=~(1<<7);   // 
	DDRB|=(1<<1);     //MR 
	PORTB|=(1<<1);    // +
	DDRD|=(1<<5);     //OE
	PORTD&=~(1<<5);   // OE enable
	
	//----------------------------- port input

	DDRC&=~(1<<2); //button SET
	DDRC&=~(1<<3); //button  start
	DDRC&=~(1<<4); //button pause
	DDRC&=~(1<<5); //voltage
		
	//--------------------------- pin pull up
	PORTC|=(1<<2);
	PORTC|=(1<<3);
	PORTC|=(1<<4);
	PORTC|=(1<<5);
		
	//-------------------------- clear registers
	for (uint8_t i=0; i<SIZE_BYTE*MAX_DIGITS; i++) 
	{
		send_CLK;
	}
	end_Transmision_Spi;
}


void timer_init (void)
{
#ifdef QUARTZ_32768

	TCCR2 = 0; //tick 1/2 sek
	TCCR2 |=(1<<CS22);
	ASSR|=(1<<AS2);
	TIMSK |=(1<<TOIE2);

#else
	
	TCCR1B |= (1<<WGM12)      // CTC mode
	| (1<<CS12) | (1<<CS10); // /1024
	OCR1AH = (uint16_t)HALF_SEC_4M>>SIZE_BYTE;
	OCR1AL = HALF_SEC_4M;
	TIMSK = (1<<TOIE1)       // Timer 1 enable
	| (1<<OCIE1A);           // interupt compare with OCR1A

#endif
}

#ifdef QUARTZ_32768
	ISR (TIMER2_OVF_vect)
#else 
	ISR (TIMER1_COMPA_vect)
#endif
{
	static uint8_t timing=0;
	if (voltage_f)
	{
		blink = !blink;
		if (timer_run)
		{
			if (time_current[MIN]==0 && time_current[HOUR]==0 && time_current[SEC]==SIGNAL_TO_LOAD_ON && signal_allowed && signale==OFF) signale = ON;
			else if (time_current[MIN]==0 && time_current[HOUR]==0 && time_current[SEC]<6) signale = OFF;
			if (time_current[MIN]==0 && time_current[HOUR]==0 && time_current[SEC]==0)
			{
				if (timing == 0)
				{
					conveer = ON;
				}
				else if(timing == 3)
				{
					conveer = OFF;
				}
				else if (timing > 44)
				{
					read_m();
					timing = 0;
					blink = FALSE;
				}
				timing++;
			}
			else if(blink)
			{
				for(int8_t i=0; i<SIZE_TIME_CUR; i++)
				{
					if(time_current[i] == 0)
					{
						time_current[i] = MAX_MIN_SEC;
					}
					else 
					{
						time_current[i]--;
						break;
					}
				}
			}
		}
	}
	else if(conveer == ON)
	{
		conveer = OFF;
	}
}
		
			
	
			
uint8_t get_button (void) 
{
	static uint16_t active_button = UNPRESS;
	static uint16_t count_volt=0, count_but=0;
	if (voltage_f != voltage_state)
	{
		count_volt++;
		if (count_volt>RESPONSE)
		{
			voltage_f = voltage_state; 
			count_volt = 0;
		}
	}
	else if (count_volt)
	{
		count_volt--;
	} 
	
	if(count_but == 0)active_button = UNPRESS;
	if(active_button == UNPRESS)
	{
		if(buton_set)active_button=PRESS_SETTING;
		else if(buton_start)active_button=PRESS_START;
		else if(buton_stop)active_button=PRESS_STOP;
	}

	if((buton_set && active_button==PRESS_SETTING) || (buton_start && active_button==PRESS_START) || (buton_stop && active_button==PRESS_STOP))
	{
		if(count_but > RESPONSE)
		{
			count_but = 0;
			return active_button;
		}
		count_but++;
	}
	else if(count_but)
	{
		count_but--;
	}
	return UNPRESS;	
}

		
void execute(const uint8_t but) 
{
	if (timer_run) 
	{
		if (but == PRESS_STOP)
		{
			timer_run = OFF;
			if (signale) signale = OFF;
			if (conveer) conveer = OFF;
		}
	}
	else 
	{		
		if (but == PRESS_SETTING)
		{
			setup++;
			if (setup == READ_SETUP)
			{
				read_m();
				setup = EDITING_SEC;
			}
			else if(setup >= WRITE_SETUP)
			{
				cli();
				if (time_current[MIN] || time_current[HOUR]) signal_allowed = TRUE;
				else signal_allowed = FALSE;
				if(time_current[HOUR] == 0 && time_current[MIN] == 0 && time_current[SEC] < ALLOW_MINIMUM_DELAY_TIMER)time_current[SEC] = ALLOW_MINIMUM_DELAY_TIMER;
				for(int8_t i=0; i<SIZE_TIME_CUR; i++)
				{
					EEPROM_WRITE(i, time_current[i]);
				}
				setup = READY;
				sei();
			}
		}
		else
		{
			if (but == PRESS_STOP)
			{
				if(setup != READY)
				{
					time_current[setup] = time_current[setup] == 0 ? MAX_MIN_SEC : time_current[setup]-1;
				}									
			}													
			else if (but == PRESS_START)
			{
				if (setup != READY)
				{
					time_current[setup] = time_current[setup] == MAX_MIN_SEC ? 0 : time_current[setup]+1;
				}
				else
				{
					timer_run = ON;
				}
			}																
	}	
	}
}