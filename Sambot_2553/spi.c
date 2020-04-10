#include <msp430.h>

#define _CS         BIT4            // chip select for SPI Master->Slave ONLY on 4 wires Mode
#define SCK         BIT5            // Serial Clock
#define DATA_OUT    BIT6            // DATA out
#define DATA_IN     BIT7            // DATA in

volatile int waitObstacleResponse;

void init_USCI( void )
{
    // Waste Time, waiting Slave SYNC
    __delay_cycles(250);

    // Reset USI State
    UCB0CTL1 = UCSWRST;

    // UCSSEL_2 = Source clock 2 : SMCLK
    UCB0CTL1 |= UCSSEL_2;

    // UCSYNC = SPI mode (synchronous = same clock)
    // UCMODE_0 = 3 Pin used for SPI
    // UCMST = This is the master
    UCB0CTL0 |= (UCSYNC | UCMODE_0 | UCMST);

    // ~UC7BIT = data are written on 8 bits
    // ~UCMSB = Least significant bit comes out of the shift register first
    // ~UCCKPL = Clock polarity inactive state is low
    // ~UCCKPH = transferring data on rising edge of SCLK and reading data on falling edge of SCLK
    UCB0CTL0 &= ~(UC7BIT | UCMSB | UCCKPL | UCCKPH);

    // clearing flags that will be used to write and read data
    IFG2 &= ~(UCB0TXIFG | UCB0RXIFG);

    UCB0BR0 = 0x0A;     // divide SMCLK by 10
    UCB0BR1 = 0x00;

    // SPI : Secondary functions
    P1SEL  |= ( SCK | DATA_OUT | DATA_IN);
    P1SEL2 |= ( SCK | DATA_OUT | DATA_IN);

    // Release USI State
    UCB0CTL1 &= ~UCSWRST;

    IE2 |= UCB0RXIE;
}

void send_spi(unsigned char carac)
{
    while ((UCB0STAT & UCBUSY));   // attend que USCI_SPI soit dispo.
    while(!(IFG2 & UCB0TXIFG)); // p442
    UCB0TXBUF = carac;              // Put character in transmit buffer
}

void askObstacle(void) {
    send_spi('x');
}
