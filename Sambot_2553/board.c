#include <msp430.h>
void init_board( void )
{
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;

    if( (CALBC1_1MHZ==0xFF) || (CALDCO_1MHZ==0xFF) )
    {
        __bis_SR_register(LPM4_bits);
    }
    else
    {
        // Factory Set.
        DCOCTL = 0;
        BCSCTL1 = CALBC1_1MHZ;
        DCOCTL = (0 | CALDCO_1MHZ);
    }

    // Initialisation des registres
    P1SEL  = 0x00;        // GPIO
    P1SEL2 = 0x00;        // GPIO
    P2SEL  = 0x00;        // GPIO
    P2SEL2 = 0x00;        // GPIO
    P1DIR = 0x00;         // IN
    P2DIR = 0x00;         // IN
}
