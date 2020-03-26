#include <msp430.h>

void init_USCI(void)
{
    if(CALBC1_1MHZ==0xFF || CALDCO_1MHZ==0xFF)
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

        //--------------- Secure mode
        P1SEL = 0x00;        // GPIO
        P1DIR = 0x00;         // IN

        // led
        P1DIR |=  BIT0;
        P1OUT &= ~BIT0;

        // USI Config. for SPI 3 wires Slave Op.
        // P1SEL Ref. p41,42 SLAS694J used by USIPEx
        USICTL0 |= USISWRST;
        USICTL1 = 0;

        // 3 wire, mode Clk&Ph / 14.2.3 p400
        // SDI-SDO-SCLK - LSB First - Output Enable - Transp. Latch
        USICTL0 |= (USIPE7 | USIPE6 | USIPE5 | USILSB | USIOE | USIGE );
        // Slave Mode SLAU144J 14.2.3.2 p400
        USICTL0 &= ~(USIMST);
        USICTL1 |= USIIE;
        USICTL1 &= ~(USICKPH | USII2C);

        USICKCTL = 0;           // No Clk Src in slave mode
        USICKCTL &= ~(USICKPL | USISWCLK);  // Polarity - Input ClkLow

        USICNT = 0;
        USICNT &= ~(USI16B | USIIFGCC ); // Only lower 8 bits used 14.2.3.3 p 401 slau144j
        USISRL = 0x23;  // hash, just mean ready; USISRL Vs USIR by ~USI16B set to 0
        USICNT = 0x08;

        // Wait for the SPI clock to be idle (low).
        while ((P1IN & BIT5)) ;

        USICTL0 &= ~USISWRST;

        __bis_SR_register(LPM4_bits | GIE); // general interrupts enable & Low Power Mode
}



