// Εργ Μικρο Ασκ 3

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <header.h>

#define PIOA_ID 2
#define TC0_ID 17
#define PIN9   9 
#define PIN8   8
#define BUT_IDLE 0
#define BUT_PRESSED 1
#define BUT_RELEASED 2
#define LED_IDLE 3
#define LED_FLASHING 4  
#define IDLE 0
#define HOLD 1

void FIQ_handler(void); 

PIO* pioa = NULL;
AIC* aic  = NULL;
TC* tc    = NULL;

unsigned int BCD_value = 0; 
unsigned int counter2 = 0;
unsigned int press_counter = 0;    
unsigned int button_state = BUT_IDLE;
unsigned int led_state = LED_IDLE;
unsigned int state = IDLE;  

int flag = 0;

int main ( int argc, const char* argv [] )
{   
	unsigned int gen;
	
	STARTUP;         
	
	// timer sta 2 Hz
	tc->Channel_0.RC  = 2084;   
	tc->Channel_0.CMR = 2084; 
	tc->Channel_0.IDR = 0xFF; 
	tc->Channel_0.IER = 0x10; 	
    
	aic->FFER = ( 1<<PIOA_ID ) | (1 << TC0_ID);   
	aic->IECR = ( 1<<PIOA_ID ) | (1 << TC0_ID);  
	
	pioa->PER  = 0x3FF;
	pioa->OER  = 0x1FF;
	pioa->CODR = 0x1FF;
	pioa->ODR  = (1<<PIN9);
    	pioa->PUER = (1<<PIN9);	
	  
	pioa->IER = (1<<PIN9);	// =0x400 interupt apo to diakopth
	gen = pioa->ISR;	//ka8arizei ton ISR
	gen = tc->Channel_0.SR;	// kararisma me anagnwsh
	aic->ICCR = (1<<PIOA_ID) | (1<<TC0_ID);   
		
	tc->Channel_0.CCR = 0x5;	// starts timer 
	
	while( getchar() != 'e'){
     
		// for debugging only
	     	printf("%d\t%d:%d\t%d\n",
	     		BCD_value,
	     		BCD_value >> 4,
	     		BCD_value & 0xF, flag);
	} 
	
	aic->IDCR = (1<<PIOA_ID) | (1<<TC0_ID); 
	tc->Channel_0.CCR = 0x02;               
	CLEANUP;
	return 0; 
}

void FIQ_handler(void)
{		 
	unsigned int fiq = 0;
	unsigned int data_in = 0;
	unsigned int data_out = 0;   
	
	data_in = pioa->PDSR;

	//interupts pou den exoun e3upireti8ei akoma
	fiq = aic->IPR;

	if (fiq & (1<<PIOA_ID)) 
	{
		//clearing the interrupt from pioa and aic
		data_out  = pioa->ISR;  
		aic->ICCR = (1<<PIOA_ID);  
		
		flag=1;
		if(!(data_in & (1<<PIN9)))
		{
			flag=2;
			//an prokaleitai interupt apo ton diakopth
			if(button_state==BUT_IDLE
				// h katastash tou ginetai patimeno
				button_state = BUT_PRESSED;
		}
		//the interrupt has been invoked releasing the button
		else 
		{
			flag=3;
			if (button_state == BUT_PRESSED)
			{
				button_state = BUT_IDLE;
				if (press_counter<2)
				{
					// an patietai < 1s kane hold else kane idle
					if (state==IDLE) state = HOLD;
					else tate = IDLE;
				}
				press_counter = 0;
			}
		}
	}
	
	if(fiq & (1<<TC0_ID) )
	{
		//ka8arisma ta interrupt apo Channel_0 kai  aic 
		data_out  = tc->Channel_0.SR; 
		aic->ICCR = (1<<TC0_ID);

		if (!counter2) counter2 = 1;
		// epeidi trexoume sta 2 Hz o parakatw
		// kwdikas ekteleite mia fora ana duo kuklous
		else 
		{   
			BCD_value++;

			// an BCD_value == 90 (sto dekadiko 60)
			// kanoume reset
			if(BCD_value == 90) BCD_value = 0;
			
			//elegxos thn anaparastash BCD
			if((BCD_value & (1<<3)) && (BCD_value & (1<<1)))
				BCD_value = BCD_value + 6;

			counter2 = 0;
			if(state == IDLE)
		    	{	
				pioa->CODR = 0x1FF;
				// grafontas to  BCD_value at pioa outputs
				pioa->SODR = BCD_value;
		    	}
		}

		if (button_state == BUT_PRESSED)
		{
			press_counter++;
			if (press_counter == 3) 
			{
				//epanafora  twn counters
				BCD_value = 0;
				counter2 = 0;
				//ka8arisma  pioa outputs	
				pioa->CODR = 0x1FF;
			}
		}

		if (state == HOLD)
		{
			pioa->CODR =  (counter2<<PIN8);     
			pioa->SODR =  ((~counter2)<<PIN8);
		}
		tc->Channel_0.CCR = 0x05;		    
	}
}
	
