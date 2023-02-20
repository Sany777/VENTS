#include "main.h"


int main (void)
{
		port_ini ();
		timer_init ();
		read_m ();
		uint8_t numbers[DIGITS_MAX]={None,None,None,None,None,None};
		sei();

																		
	while (1)
	{	
		control(get_button());											
		set_digits_numbers(numbers);
		SPI(numbers);
	}																		
}



void SPI (uint8_t *numbers) 
{
	cli ();
	for (uint8_t digit = 0,byte = 0; digit<DIGITS_MAX; digit++) 
	{
		if (voltage_f) 
		{
			switch(numbers[digit])
			{
				case 1:  byte = 0b000000110; break;
				case 2:  byte = 0b01011011;  break;
				case 3:  byte = 0b01001111;  break;
				case 4:  byte = 0b01100110;  break;
				case 5:  byte = 0b01101101;  break;
				case 6:  byte = 0b01111101;  break;
				case 7:  byte = 0b00000111;  break;
				case 8:  byte = 0b01111111;  break;
				case 9:  byte = 0b01101111;  break;
				case 0:  byte = 0b00111111;  break;
				default: byte = 0;           break;
			}
			// ---------------------------------- direction
			switch(digit)
			{
				case BLINK_FIRST       : if(blink && (min || hour)) byte|=(1<<7); break;	
				case BLINK_SECOND      : if(blink && hour)          byte|=(1<<7); break;
				case CONVEER_SPI       : if(conveer == ON)          byte|=(1<<7); break;
				case SIGNAL_SPI        : if(signale == ON)          byte|=(1<<7); break;
				default                : break;
			}
		}
		else 
		{
		// ---------------------------------- show "OFF"
			 if (digit == 3)      byte = 0X3F;          
			 else if (digit == 2) byte = 0X71;
			 else if (digit == 1) byte = 0X71;
			 else  byte = 0;
		}
		//---------------------------------- send to SPI
			for (uint8_t c=0; c<8; c++)					  
			{
				if (byte&0x80)
				{
					PORTD|=(1<<6);
				}
				else
				{
					PORTD&=~(1<<6);
				}
				byte = (byte<<1);
				PORTB|=(1<<0);
				PORTB&=~(1<<0);
			}
	}
	PORTD|=(1<<7); 
	PORTD&=~(1<<7);
	sei();
}
	


void set_digits_numbers(uint8_t *numbers)
{
	numbers[0]= setup == EDITING_SEC  && blink ? None : sek%10;
	numbers[1]= setup == EDITING_SEC  && blink ? None : sek/10;
	numbers[2]= setup == EDITING_MIN  && blink ? None : min%10;
	numbers[3]= setup == EDITING_MIN  && blink ? None : min/10;
	numbers[4]= setup == EDITING_HOUR && blink ? None : hour%10;
	numbers[5]= setup == EDITING_HOUR && blink ? None : hour/10;
	
	if (timer_run)
	{
		for (uint8_t digit=5; digit && numbers[digit]; digit--)
		{
			numbers[digit] = None;     
		}
	}
	
	
}




void EEPROM_WRITE (uint16_t uiAddress, uint8_t ucData)
{
	while (EECR&(1<<EEWE));
	EEAR = uiAddress;
	EEDR = ucData;
	EECR |= (1<<EEMWE);
	EECR |= (1<<EEWE);
}


uint8_t EEPROM_read(uint16_t uiAddress)
{
	while(EECR & (1<<EEWE));
	EEAR = uiAddress;
	EECR |= (1<<EERE);
	return EEDR;
}

void read_m (void)
{
	sek  = EEPROM_read(ADDR_SEC)%TO_DEC;
	min  = EEPROM_read(ADDR_MIN)%TO_DEC;
	hour = EEPROM_read(ADDR_HOUR)%TO_DEC;
	if (min || hour) signal_allowed = TRUE;
	else signal_allowed = FALSE;
}


void timer_init (void)
{
	TCCR2 = 0; //tick 1/2 sek
	TCCR2 |=(1<<CS22);
	ASSR|=(1<<AS2);
	TIMSK |=(1<<TOIE2);
}



void port_ini (void)
{
	//---------------------- program SPI : 0-6 bit - show number, 7bit - control load
	DDRD|=(1<<6);      //DS
	PORTD&=~(1<<6);   //
	DDRB|=(1<<0);     //clk
	PORTB&=~(1<<0);  //
	DDRD|=(1<<7);     // ST 
	PORTD&=~(1<<7);  //
	DDRB|=(1<<1);    //MR 
	PORTB|=(1<<1);  // +
	DDRD|=(1<<5);    //OE
	PORTD&=~(1<<5); // -

	//-------------------------- clear registers
	for (int x=0; x<50; x++) 
	{
		PORTB|=(1<<0);
		PORTB&=~(1<<0);
	}
	PORTD |= (1<<7); 
	PORTD &= ~(1<<7);
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
}



ISR (TIMER2_OVF_vect)
{
	static uint8_t timing=0;
	if (voltage_f)
	{
		blink = !blink;
		if (timer_run)
		{
			if (min==0 && hour==0 && sek == SIGNAL_TO_LOAD_ON && signal_allowed && signale == OFF) signale = ON;
			else if (min==0 && hour==0 && sek<6) signale = OFF;
			if (min == 0 && hour == 0 && sek == 0)
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
					blink = TRUE;
				}
				timing++;
			}
			else if(blink) 
			{
				sek--;
				if (sek<0)
				{
					min--;
					sek=59;
					if (min<0)
					{
						hour--;
						min=59;
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
	static uint8_t active_button = UNPRESS;
	static uint16_t count_volt=0, count=0;
	if ( voltage_f != voltage_state )
	{
		if (count_volt<RESPONSE)
		{
			count_volt++;
		}
		else 
		{
			voltage_f = voltage_state; 
			count_volt = 0;
		}
	}
	else if (count_volt > 0)
	{
		count_volt--;
	} 	
	
	if(count == 0)active_button = UNPRESS;
	if(active_button == UNPRESS)
	{
		if(buton_set)active_button=PRESS_SETTING;
		else if(buton_start)active_button=PRESS_START;
		else if(buton_stop)active_button=PRESS_STOP;
	}

	if((buton_set && active_button==PRESS_SETTING) || (buton_start && active_button==PRESS_START) || (buton_stop && active_button==PRESS_STOP))
	{
		if(count > RESPONSE)
		{
			count = 0;
			return active_button;
		}
		count++;
	}
	else if(count)
	{
		count--;
	}
	return UNPRESS;	
}
			
									
void control(const uint8_t but) 
{
	if (timer_run) 
	{			
		if (but == PRESS_STOP)timer_run = OFF;
		if (signale) signale = OFF;
		if (conveer) conveer = OFF;
	}
	else 
	{
		if (but == PRESS_STOP) 
		{
			if(setup == EDITING_SEC) 
			{
				sek--;
				if (sek<0) sek=59;
			}											
			else if(setup == EDITING_MIN)
			{
				min--;
				if (min<0) min=59;
			}										
			else if(setup == EDITING_HOUR)
			{
				hour--;
				if (hour<0)hour=23;
			}
		}													
		else if (but == PRESS_START)
		{
			if (setup == READY)
			{
				timer_run = ON;
				blink = ON;
			}														
			else if (setup == EDITING_SEC)
			{
				sek++;
				if (sek>59)sek = 0;
			}													
			else if (setup==EDITING_MIN)
			{
				min++;
				if (min>59)min = 0;
			}														
			else if(setup==EDITING_HOUR) 
			{
				hour++;
				if (hour>23)hour = 0;
			}
		}															
		else if (but == PRESS_SETTING)
		{
			setup++;
			if (setup == READ_SETUP)
			{
				read_m();
				setup = EDITING_SEC;
			}
			else if(setup == WRITE_SETUP)
			{
				cli();
				if (min || hour) signal_allowed = TRUE;
				else signal_allowed = FALSE;
				if(hour == 0 && min == 0 && sek < ALLOW_MINIMUM_DELAY_TIMER)sek = ALLOW_MINIMUM_DELAY_TIMER;
				EEPROM_WRITE(ADDR_SEC, sek);
				EEPROM_WRITE(ADDR_MIN, min);
				EEPROM_WRITE(ADDR_HOUR, hour);
				setup = READY;
				sei();
			}
		}
	}																		
}

