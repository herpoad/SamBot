#include <msp430.h>

void init_USCI(void)
{
        //--------------- Secure mode
        P1SEL = 0x00;        // GPIO
        P1DIR = 0x00;         // IN

        // Reset USI State
        USICTL0 = USISWRST;

        // Sets this micro-controller as a slave for SPI communication
        USICTL0 &= ~USIMST;

        // USIOE = Enable data output
        // USIGE = We don't handle output latch ourselves
        // USILSB = Least significant bit comes out of the shift register first
        // USIPE5 = In slave mode this enabled the input for synchronized clock coming from the master (on pin 1.5)
        // USIPE6 = Enable output data on pin 1.6 (MISO)
        // USIPE7 = Enable input data on pin 1.7 (MOSI)
        USICTL0 |= (USIOE | USIGE | USILSB | USIPE5 | USIPE6 | USIPE7);

        // Enable interrupts on USI counter
        USICTL1 |= USIIE;

        // ~USIIFG = Setting interrupt flag down
        // ~USISTTIFG = Setting start condition flag down (we won't use this as USISTTIE is set to 0)
        // ~USISTP = setting stop condition flag down (we won't use this as USISTTIE is set to 0)
        // ~USIAL = No arbitration lost condition
        // ~USISTTIE = interrupt on start condition disabled
        // ~USII2C = disabling I2C mode as we're using SPI mode only
        // ~USICKPH = transferring data on rising edge of SCLK and reading data on falling edge of SCLK
        USICTL1 &= ~(USIIFG | USISTTIFG | USISTP | USIAL | USISTTIE | USII2C | USICKPH);

        // USIDIVx (clock divider) and USISSELx (clock source select) not used in slave mode because SCLK comes from master
        USICKCTL = 0;

        // ~USICKPL = Clock polarity inactive state is low
        // ~USISWCLK = Input clock is low
        USICKCTL = ~(USICKPL | USISWCLK);

        // ~USISCLREL = the bit should NOT be release because this is the only slave used. The master will ALWAYS speaks with him and only him
        // ~USI16B = using 8-bit shift register
        // ~USIIFGCC = the flag USIIFG will cleared automatically every time USICNTx will update
        // ~USICNT4, USICNT3, ~USICNT2, ~USICNT1, ~USICNT0 to start the counter on 8
        USICNT = 0x08;

        // Initialazing shift register
        USISRL = 0;

        USICTL0 &= ~USISWRST;
        //__bis_SR_register(LPM4_bits | GIE); // general interrupts enable & Low Power Mode
}



