#include <msp430.h> 
#include <intrinsics.h>
#include "spi.h"
#include "capteur_infra.h"
#include "capteur_ultrason.h"
#include "board.h"

volatile unsigned char RXDta;
volatile unsigned int distance_obstacle = 0, distance_trou = 0;
unsigned int up = 0;    // sens de variation
unsigned int cmd = 0;   // periode du signal de commande moteur
int test;
/*
 * main.c
 */
void main( void )
{
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;

    init_board();
    init_USCI();
    init_infra();
    init_ultrason();
    __enable_interrupt();
    BCSCTL3 &= ~(LFXT1S0 | LFXT1S1);
    while(1) {
        _delay_cycles(50000);
        distance_obstacle = get_distance_ultrason();
        distance_trou = get_distance_infra();
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

    if (RXDta == 'x') {
        int value = distance_obstacle;
        if(value > 127) {
            value = 127;
        }

        USISRL = ~BIT7 & value;
    } else if (RXDta == 'y') {
        int value = distance_trou;
        if(value > 127) {
            value = 127;
        }
        test = BIT7 | value;
        USISRL = BIT7 | value;
    }

    USICNT &= ~USI16B;  // re-load counter & ignore USISRH
    USICNT = 0x08;      // 8 bits count, that re-enable USI for next transfert
}
