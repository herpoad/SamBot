#include <msp430.h>

#define ECHO BIT3

#define TRIGGER BIT4

int miliseconds;
long sensor;


void init_ultrason(void){
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;                     // submainclock 1mhz
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

    CCTL0 = CCIE;                             // CCR0 interrupt enabled
    CCR0 = 1000;                  // 1ms at 1mhz
    TACTL = TASSEL_2 + MC_1;                  // SMCLK, upmode

    P1IFG  = 0x00;                //clear all interrupt flags
    __enable_interrupt();
}


int get_distance_ultrason()
{
    int distance;
    P1DIR |= TRIGGER;          // trigger pin as output
    P1OUT |= TRIGGER;          // generate pulse
    __delay_cycles(10);             // for 10us
    P1OUT &= ~TRIGGER;                 // stop pulse
    P1DIR &= ~ECHO;         // make pin P1.3 input (ECHO)
    P1IFG = 0x00;                   // clear flag just in case anything happened before
    P1IE |= ECHO;           // enable interupt on ECHO pin
    P1IES &= ~ECHO;         // rising edge on ECHO pin
    __delay_cycles(30000);          // delay for 30ms (after this time echo times out if there is no object detected)
    distance = sensor/58;           // converting ECHO lenght into cm
    return distance;
}

int is_obstacle(unsigned int distance) {
    if(distance > 8) {
        return 0;
    } else {
        return 1;
    }
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    if(P1IFG&ECHO)  //is there interrupt pending?
        {
          if(!(P1IES&ECHO)) // is this the rising edge?
          {
            TACTL|=TACLR;   // clears timer A
            miliseconds = 0;
            P1IES |= ECHO;  //falling edge
          }
          else
          {
            sensor = (long)miliseconds*1000 + (long)TAR;    //calculating ECHO lenght

          }
    P1IFG &= ~ECHO;             //clear flag
    }
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
  miliseconds++;
}


