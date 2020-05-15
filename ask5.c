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

#define P_0 0x01
#define P_1 0x02
#define P_2 0x04
#define P_3 0x08
#define P_4 0x10
#define P_5 0x20
#define P_6 0x40
#define P_7 0x80
#define P_8 0x100
#define P_9 0x200
#define P_10 0x400
#define P_11 0x800
   
void FIQ_handler(void);

PIO* pioa = NULL;
AIC* aic = NULL;
TC* tc = NULL;

int gen,tmp;
int Tout = 8192;

int Left_Scor = 0x00;
int Right_Scor = 0x00;
int Left_But_Down = 0;
int Right_But_Down = 0;
int Left_But_Active = 0;
int Right_But_Active = 0;
int Last_pos;
int Ball_pos;

int flag = 0;
int speed_count = -1;
int led_state=0;

int dbgInfo = 0;

int main( int argc, const char* argv[]){
    STARTUP; //ARXIKOPOIHSH SYSTHMATOS
    tc->Channel_0.RC = Tout;  //PERIODOS 0.5s, 5Hz	
    tc->Channel_0.CMR = 2084; //SLOW CLOCK, WAVEFORM, DISABLE CLK ON RC COMPARE
    tc->Channel_0.IDR = 0xFF; //APENERGOPOIHSH OLWN TWN DIAKOPWN
    tc->Channel_0.IER = 0x10; //ENERGOPOIHSH MONO TOU RC COMPARE
    aic->FFER = (1<<TC0_ID); //OI DIAKOPES 2,17 EINAI TYPOU FIQ
    aic->IECR = (1<<TC0_ID); //ENERGOPOIHSH DIAKOPWN: TC0
    pioa->PER = 0x2FF; //GRAMMES 0-7: GENIKOU SKOPOU
    pioa->OER = (P_0 | P_1 | P_2 | P_3 | P_4 | P_5 | P_6 | P_7 | P_8 | P_9); //GRAMMES 0-9: LEITOURGIA EKSODOU
    pioa->PUER = (P_10 | P_11);  //GRAMMH 10,11: ENERGOPOIHSH PULL-UP ANTISTASHS
    pioa->ODR = (P_10 | P_11);  //GRAMMH 10,11: LEITOURGIA EISODOU
    pioa->IER = (P_10 | P_11);  //GRAMMH 10,11: ENERGOPOIHSH DIAKOPWN
    gen = pioa->ISR;  //PIOA: EKKATHARISH APO TYXON DIAKOPES ME ANAGNWSH						
    gen = tc->Channel_0.SR; //TC0: EKKATHARISH APO TYXON DIAKOPES ME ANAGNWSH				
    aic->ICCR = (1<<TC0_ID);  //AIC:EKKATHARISH APO TYXON DIAKOPES ME ANAGNWSH
     
    tc->Channel_0.CCR=0x05;
    
    while((tmp=getchar())!='e') {
		if (tmp == '?') printf("%d\t%d\n", Left_Scor, Right_Scor);
    }
                                
    aic->IDCR = (1<<TC0_ID);
    tc->Channel_0.CCR = 0x02;
    CLEANUP;
    return 0;
}    

void FIQ_handler(void){
      unsigned int data_in = 0;
	 unsigned int fiq = 0;
	 unsigned int data_out;
	 fiq = aic->IPR; //ENTOPISMOS PERIFEREIAKOU POU PROKALESE TH DIAKOPH
   
	 if( fiq & (1<<TC0_ID) ){
		    data_in = pioa->PDSR;
         	data_out = tc->Channel_0.SR;
        	aic->ICCR = (1<<TC0_ID);
        	data_out = pioa->ODSR;
        	
         	pioa->CODR = 0xFF; // 8 bits
        
		if(speed_count == -1){
				if(!(data_in & P_10)){
					Ball_pos = 0x00;
					Last_pos = 0x01;
					if(!Right_But_Down){
		 				Right_But_Active = 1;
		 				Right_But_Down = 6;
		 			}
		 			speed_count = 0;
		 			flag=0;
					
					dbgInfo = 1;
				} 
				else if(!(data_in & P_11)){
					Ball_pos = 0x00;
					Last_pos = 0x40;
                    	if(!Left_But_Down){
                    		Left_But_Active = 1;
                    		Left_But_Down = 6;
                    	}
                    	speed_count = 0;
                    	flag=0;
						
					dbgInfo = 2;
				}	
		} 	
		else{
			if(!(data_in & P_11)) 
			    if(!Left_But_Down){
                    		Left_But_Active = 1;
                    		Left_But_Down = 6;
                   }
			if(!(data_in & P_10)) 
			    if(!Right_But_Down){
		 				Right_But_Active = 1;
		 				Right_But_Down = 6;
		 	    }
		}

		/*CHECK PLAYER KEYS*/
		if(Left_But_Down >= 4) Left_But_Down--;
		else if((Left_But_Down < 4) && (Left_But_Down!=0)){
			Left_But_Active = 0;
			Left_But_Down--;
		}
		if(Right_But_Down >= 4) Right_But_Down--;
		else if((Right_But_Down < 4) && (Right_But_Down!=0)){
			Right_But_Active = 0;
			Right_But_Down--;
		}	
		
		
		if(Ball_pos == Last_pos){
			if(!Right_But_Active && Last_pos == 0x40) {
				speed_count = -1;
				
				if(flag) led_state = 0xFF;  //mexri na patithei to koumpi gia na einai ball_pos!=last_pos ta anavei ola
				else if(!flag){
					flag = 1;
					if(!Left_Scor) Left_Scor = 0x01;
					else Left_Scor |= (Left_Scor<<1);
					led_state = Left_Scor | Right_Scor;
				}
			}
			else if(!Left_But_Active && Last_pos == 0x01) {
				speed_count = -1;
				
				if(flag) led_state = 0xFF;
				else if(!flag){
					flag = 1;
					if(!Right_Scor) Right_Scor = 0x40;
					else Right_Scor |= (Right_Scor>>1);
					led_state = Left_Scor | Right_Scor;
				}
			}
			if(Left_Scor==7 || Right_Scor==112) {
				Left_Scor = 0x00;
  				Right_Scor = 0x00;
			}
			else if(Left_But_Active && Last_pos == 0x01) {
				Last_pos = Last_pos ^ 0x41;
				speed_count++;
			}	
			else if(Right_But_Active && Last_pos == 0x40) {
				Last_pos = Last_pos ^ 0x41;
				speed_count++;
			}
	    }
	    if((Ball_pos != Last_pos) && (speed_count != -1)) {
	   	  if(Ball_pos){
			if( (Last_pos & 0x01) == 0x01) Ball_pos >>= 1;
		     else Ball_pos <<= 1;
		     led_state = Ball_pos;
		  }
		  else{
			if( (Last_pos & 0x01) == 0x01) Ball_pos = 0x40;
			else Ball_pos = 0x01;
			led_state = Ball_pos;
		  }
	    }

	   if(speed_count == 2){
	   		 Tout /= 2;
	   		 speed_count = 0;
	   }
	   tc->Channel_0.RC = Tout;
	   pioa->SODR = led_state; 
	   tc->Channel_0.CCR = 0x05;
	}

 
}                          
        
        
