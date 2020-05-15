#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <header.h>

#define BUT_LEFT 0x800
#define BUT_RIGHT 0x400
#define BUT_PRESSED 1
#define BUT_RELEASED 2
#define PLAYING 0
#define FINISHED 1

unsigned int ball_pos=0;
unsigned int last_pos=0;
unsigned int player_left_score=0;
unsigned int player_right_score=0;
unsigned int Tout = 8192;
unsigned int left_button = BUT_RELEASED;
unsigned int right_button = BUT_RELEASED;
unsigned int left_counter;
unsigned int right_counter;
unsigned int game_state = FINISHED;
int Speed_count=0;
unsigned int SHOWDATA = 0;

AIC* aic = NULL;
TC* tc = NULL;
PIO* Pioa = NULL;

void init(int);
void moveBall(void);
void showScore(void);
void advanceBall(void);
int buttonPressed(void);

void FIQ_handler(void)
{
    unsigned int fiq = 0;
    unsigned int data_in = 0;
    unsigned int data_out;
    int button = -1;
    
    fiq = aic->IPR;  //twra h fiq perimenei interrupt apo pioa h tc
    
    if(fiq & (1<<2)) //koumpi
    {
           data_in = Pioa->ISR;
           aic->ICCR = (1<<2); //clear interrupts
           data_in = Pioa->PDSR;
           
           SHOWDATA = data_in;
           
           if( data_in & BUT_RIGHT) //an koumpi deksia
           {
               if(right_button == BUT_PRESSED)
               {
                               right_button = BUT_RELEASED;
                               button = -1;
               }
               else if(right_button == BUT_RELEASED)
               {
                    right_button = BUT_PRESSED;
                    button = 1;
               }
           }
           else if( data_in & BUT_LEFT) //an koumpi aristera
           {
               if(left_button == BUT_PRESSED)
               {
                               left_button = BUT_RELEASED;
                               button = -1;
               }
               else if(left_button == BUT_RELEASED)
               {
                    left_button = BUT_PRESSED;                  
                    button = 0;
               }
           }
           
           if( (game_state == FINISHED) && (button != -1) )
           {
                game_state = PLAYING;
                init(button);
           }
    }
    
    
    if(fiq & (1<<17)) //roloi
    {
           data_out = tc->Channel_0.SR;
           aic->ICCR = (1<<17); //clear interrupts
           data_out = Pioa->ODSR;
           
           moveBall();
           
           if( (player_left_score == 3) || (player_right_score == 3) )
           {
               Pioa->SODR = 0xff; // 8 bit
               game_state = FINISHED;
               tc->Channel_0.CCR = 0x02; //stop timer
           }
           else
           {
            tc->Channel_0.RC = Tout; //mpainei h taxythta
            tc->Channel_0.CCR = 0x05; //timer enable
           }
    }

}

void init (int button) //button 0 = aristera button 1 = deksia
{
     Tout = 8192;
     player_left_score = 0;
     player_right_score = 0;
     left_counter = 6;
     right_counter = 6;
     
     if(button == 0)
     {
            ball_pos = 0x40;
            last_pos = 0x01;
     }
     else if(button == 1)
     {
          ball_pos = 0x01;
          last_pos = 0x40;
     }
     
     tc->Channel_0.RC = Tout; //arxikopoieitai h taxythta
     tc->Channel_0.CCR = 0x05; //timer enable
}

void moveBall (void) //elenxei an eftase h oxi h mpala sto telos
{
     if(ball_pos == last_pos) //exei ftasei h mpala sto telos
     {
      if(buttonPressed()) //yes
      {
       last_pos ^= 0x41;
       Speed_count++;
       
       if(Speed_count == 10)
       {
                      if(Tout > 4096)
                      {
                              Tout -= 0x100;
                      }
       }
       
       advanceBall();
      }
      else //no
      {
           if( (last_pos & 0x01) == 0x01) //if deksia
               player_left_score++;
           else
               player_right_score++;
           
           showScore();
           
           Tout |= 0x8000;
           ball_pos = last_pos ^ 0x41; //h mpala ksekinaei apo afton pou phre ponto
      }
     }
     else //an h mpala prepeina proxorhsei
     {
         advanceBall();
     }
}

void showScore(void)
{     
     Pioa->CODR = 0xff; //mhdenismos 8 bit
     
     switch(player_right_score) //emfanish score deksia
     {
      case 1:
           Pioa->SODR = 0x01;
           break;
      case 2:
           Pioa->SODR = 0x03;
           break;
      case 3:
           Pioa->SODR = 0x07;
           break;
     }
     
     switch(player_left_score) //emfanish score aristera
     {
      case 1:
           Pioa->SODR = 0x40;
           break;
      case 2:
           Pioa->SODR = 0x60;
           break;
      case 3:
           Pioa->SODR = 0x70;
           break;
     }
}

void advanceBall(void)
{
     Pioa->CODR = ball_pos;
     
     if( (last_pos & 0x01) == 0x01) //if deksia
     {
         ball_pos >>= 1;
     }
     else
     {
         ball_pos <<= 1;
     }
     
     Pioa->SODR = ball_pos;
}

int buttonPressed(void)
{
     if(left_button == BUT_PRESSED)
     {
      return 1;
     }
     else if(right_button == BUT_PRESSED)
     {
      return 1;
     }
     
     return 0;
}

/*
int buttonPressed(void)
{
     if(left_button == BUT_PRESSED)
     {
                    if(left_counter > 4)
                        return 1;
                    else
                        return 0;
     }
     else if(right_button == BUT_PRESSED)
     {
         if(right_counter > 4)
                          return 1;
         else
             return 0;
     }
     
     return 0;
}
*/
