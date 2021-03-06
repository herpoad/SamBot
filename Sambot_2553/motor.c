#include <msp430.h>


void R_avancer()
{
    P2DIR |= BIT5;
    P2DIR &=~ BIT1;
    TA1CCR0 = 2000;
    P2SEL |= BIT1|BIT5;

    volatile unsigned int i;

    TA1CCR1 = 332;
    TA1CCR2 = 300;

    for(i=20000;i>0;i--);

    TA1CCR1 = 10;
    TA1CCR2 = 10;
}

void R_tourner_gauche()
{
    TA1CCR0 = 2000;

    P2DIR |= BIT1|BIT5;
    P2SEL |= BIT1|BIT5;

    volatile unsigned int j;

    TA1CCR1 = 443;
    TA1CCR2 = 400;

    for(j=20000;j>0;j--);

    TA1CCR1 = 10;
    TA1CCR2 = 10;
}

void R_tourner_droite()
{

    P2DIR &=~ (BIT1|BIT5);
    P2SEL |= BIT1|BIT5;

    TA1CCR0 = 2000;

    volatile unsigned int k;

    TA1CCR1 = 443;
    TA1CCR2 = 400;


    for(k=20000;k>0;k--);

    TA1CCR1 = 10;
    TA1CCR2 = 10;

}

void R_demi_tour()
{
    R_tourner_gauche();
    R_tourner_gauche();

}

void R_reculer(void) {
    TA1CCR0 = 2000;
    P2DIR |= BIT1;
    P2DIR &=~ BIT5;
    P2SEL |= BIT1|BIT5;

    volatile unsigned int i;

    TA1CCR1 = 332;
    TA1CCR2 = 300;

    for(i=20000;i>0;i--);

    TA1CCR1 = 10;
    TA1CCR2 = 10;
}

void init_motor(void){

    BCSCTL1= CALBC1_1MHZ; //frequence d�horloge 1kHz
    DCOCTL= CALDCO_1MHZ; // "
    P2DIR &=~ (BIT0|BIT3); // entr�es
    P2DIR |= (BIT2|BIT4); //sorties
    P2SEL |= (BIT2|BIT4); // selection fonction TA1.1
    P2SEL2 &= ~(BIT2|BIT4); // selection fonction TA1.1
    TA1CTL = TASSEL_2 | MC_1; // source SMCLK pour TimerA (no 2), mode comptage Up
    TA1CCTL1 |= OUTMOD_7; // activation mode de sortie n�7
    TA1CCTL2 |= OUTMOD_7;
    P2IE |= BIT0|BIT3;
    P2IES |= BIT0|BIT3;
    P2IFG &=~ (BIT0|BIT3);

    __enable_interrupt();

}


#pragma vector = PORT2_VECTOR

__interrupt void Port_2(){
    if(P2IFG & BIT0){
        P2IFG &=~ BIT0;
    }
    else if(P2IFG & BIT3){
        P2IFG &=~ BIT3;
    }
}

