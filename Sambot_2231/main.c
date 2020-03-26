#include <msp430.h> 
#include <intrinsics.h>

volatile unsigned char RXDta;
volatile unsigned int obstacle = 0, trou = 0;
/*
 * main.c
 */
void main( void )
{
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;
    init_USCI();
    init_infra();
    init_ultrason();
    while(1) {
        obstacle = get_distance_ultrason();
        trou = get_distance_infra();
    }
}

// --------------------------- R O U T I N E S   D ' I N T E R R U P T I O N S

/* ************************************************************************* */
/* VECTEUR INTERRUPTION USI                                                  */
/* ************************************************************************* */
#pragma vector=USI_VECTOR
__interrupt void universal_serial_interface(void)
{
    while( !(USICTL1 & USIIFG) );   // waiting char by USI counter flag
    RXDta = USISRL;

    if (RXDta == '0') //if the input buffer is 0x31 (mainly to read the buffer)
    {
        P1OUT |= BIT0; //turn on LED
    }
    USISRL = obstacle;
    USICNT &= ~USI16B;  // re-load counter & ignore USISRH
    USICNT = 0x08;      // 8 bits count, that re-enable USI for next transfert
}
//------------------------------------------------------------------ End ISR
