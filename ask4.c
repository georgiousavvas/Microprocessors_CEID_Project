#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <header.h>
#define PIOA_ID				2
#define TC0_ID				17
#define OFF				0
#define RED 				1
#define YEL				2
#define GRE				3
#define BLN				4
#define START_BUT_IDLE			0
#define START_BUT_PRESSED		1
#define STOP_BUT_IDLE			0
#define STOP_BUT_PRESSED		1
#define F3_IDLE				0
#define F3_FLASHING			1

void FIQ_handler(void);
PIO *	pioa	= NULL;
AIC *	aic	= NULL;
TC  *	tc	= NULL;
unsigned int start_button_state;
unsigned int stop_button_state;
unsigned int time_seconds;
unsigned int f1;
unsigned int f2;
unsigned int f3;
unsigned int flag_k5 = 0;
unsigned int f3_state = F3_IDLE;

int main( int argc, const char* argv[] ) {
	unsigned int gen, tmp;
    	//arxikopoihsh systhmatos
 	STARTUP;
	start_button_state	= START_BUT_IDLE;
	stop_button_state	= STOP_BUT_IDLE;
	//K0(arxikh) tou pinaka "katastash"
	f1 = RED;
	f2 = GRE;
	f3 = OFF;
	time_seconds = 0;
	//diamorfwsh rologiou.
	tc->Channel_0.RC	= 2048;	
	tc->Channel_0.CMR	= 2084;
	tc->Channel_0.IDR	= 0xFF;	
	tc->Channel_0.IER	= 0x10;
	//diamorfwsh diakopwn
	aic->FFER	= (1<<PIOA_ID) | (1<<TC0_ID);
	aic->ICCR	= (1<<PIOA_ID) | (1<<TC0_ID);
	aic->IECR	= (1<<PIOA_ID) | (1<<TC0_ID);
    	//genikou skopou
	pioa->PER	= 0xFF;
	//eksodos
	pioa->OER	= 0x3F;
    	//eisodos
	pioa->ODR	= 0xC0; 
	//antistash pull-up
	pioa->PUER	= 0xC0; 
	//eka8arish diakopwn
	gen = tc->Channel_0.SR; 
	gen = pioa->ISR;	
	pioa->IER	= 0xC0;
		tc->Channel_0.CCR = 0x05;	
 	while (1) {
		tmp = getchar();
		if (tmp == 'e')
			break;	}
	//stop twn diakopwn
	aic->IDCR	= (1<<PIOA_ID) | (1<<TC0_ID);
	tc->Channel_0.CCR = 0x02;
	CLEANUP;	
	return 0;}
	
void FIQ_handler(void) {
	unsigned int data_in = 0;
	unsigned int fiq = 0;
	unsigned int data_out;
	fiq = aic->IPR;
	if ( fiq & (1<<TC0_ID) ) {
		data_out	= tc->Channel_0.SR;
		aic->ICCR	= (1<<TC0_ID); 
		time_seconds++;	
		//apo thn K1 katastash sthn K2
		if ((f1 == RED) && (f2 == GRE) && (f3 == BLN) && (time_seconds == 40)) {
			f2 = YEL;									  
			time_seconds = 0; }
		//apo thn K2 sthn K3
		if ((f1 == RED) && (f2 == YEL) && (f3 == BLN) && (time_seconds == 12)) {
			f2 = RED;													 
			time_seconds = 0; }
		//apo thn K3 sthn K4
		if ((f1 == RED) && (f2 == RED) && (f3 == BLN) && (time_seconds == 8)) {
			f1 = GRE;
			flag_k5 = 1;							 
			time_seconds = 0; }
		//apo thn K4 sthn K5
		if ((f1 == GRE) && (f2 == RED) && (f3 == BLN) && (time_seconds == 40)) {
			flag_k5 = 0;													 
			time_seconds = 0; }
		//apo thn K6 sthn K0
		if ((f1 == RED) && (f2 == RED) && (f3 == OFF) && (time_seconds == 20)) {
			f2 = GRE;										 
			time_seconds = 0; }
		if (f3 == BLN) {
			if(f3_state == F3_IDLE){
				pioa->CODR		= 0x20;
				pioa->SODR		= 0x20;
				f3_state = F3_FLASHING; }
			else {
				pioa->CODR		= 0x20;
				f3_state = F3_IDLE; }
		}
		tc->Channel_0.CCR = 0x05;
	}
	if( fiq & (1<<PIOA_ID) ) {				
		data_in = pioa->ISR;				
		aic->ICCR = (1<<PIOA_ID);			
		data_in = pioa->PDSR;
        	//start button
		if(( data_in & 0x80 )== 0) {  
			if(start_button_state == START_BUT_IDLE){ 
				start_button_state = START_BUT_PRESSED;
				if((f1 == RED) && (f2 == GRE) && (f3 == OFF)){
                    			//apo thn K0 sthn K1   
					f3 = BLN;								   
					time_seconds = 0;	
				}
			}
		}
		else if (start_button_state == START_BUT_PRESSED)
			start_button_state = START_BUT_IDLE;
		}
        	//stop button
		if (( data_in & 0x40 )== 0) { 
			if(stop_button_state == STOP_BUT_IDLE){ 
				stop_button_state = STOP_BUT_PRESSED;
				if(flag_k5==0){
                    			//apo thn K5 sthn K6           
					f1 = RED;								  
					f3 = OFF;
					time_seconds = 0;	
				}
			}
		}
		else if (stop_button_state == STOP_BUT_PRESSED)
			stop_button_state = STOP_BUT_IDLE;			
		}
	}
	//deutero fanari Ö2
	if(f2 == GRE){
   		pioa->CODR		= 0x07;
		pioa->SODR		= 0x01;
	} else if(f2 == YEL){
   		pioa->CODR		= 0x07;
		pioa->SODR		= 0x02;
	} else if(f2 == RED){
   		pioa->CODR		= 0x07;
		pioa->SODR		= 0x04;
	} else{ }
	//prwto fanari Ö1
	if(f1 == GRE){
   		pioa->CODR		= 0x18;
		pioa->SODR		= 0x08;
	} else if(f1 == RED){
		pioa->CODR		= 0x18;
		pioa->SODR		= 0x10;
	} else { }
	//trito fanari Ö3
	if (f3 == OFF) pioa->CODR	= 0x20;
}
