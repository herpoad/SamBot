#include <msp430.h>
void init_UART(void)
{
    P1SEL |= (BIT1 | BIT2);                     // P1.1 = RXD, P1.2=TXD
    P1SEL2 |= (BIT1 | BIT2);                    // P1.1 = RXD, P1.2=TXD
    UCA0CTL1 = UCSWRST;                         // SOFTWARE RESET
    UCA0CTL1 |= UCSSEL_3;                       // SMCLK (2 - 3)

    UCA0CTL0 &= ~(UCPEN | UCMSB | UCDORM);
    UCA0CTL0 &= ~(UC7BIT | UCSPB | UCMODE_3 | UCSYNC); // dta:8 stop:1 usci_mode3uartmode
    UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**

    UCA0BR0 = 104;                              // 1MHz, OSC16, 9600 (8Mhz : 52) : 8/115k
    UCA0BR1 = 0;                                // 1MHz, OSC16, 9600
    UCA0MCTL = 10;

    /* Enable USCI_A0 RX interrupt */
    IE2 |= UCA0RXIE;
}

void TXdata( unsigned char c )
{
    while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
    UCA0TXBUF = c;              // TX -> RXed character
}

//void TXint(unsigned int i)
//{
//    while (!(IFG2 & UCA0TXIFG));  // USCI_A0 TX buffer ready?
//    UCA0TXBUF = i;              // TX -> RXed character
//}

//void TXObstacle(unsigned int i)
//{
//    TXdatas(intToString(i));
//    TXdatas("\r\n");
//}
//
//void TXHole(unsigned int i)
//{
//    TXdatas("hole: ");
//    TXdatas(intToString(i));
//    TXdatas("\r\n");
//}
//
//char* intToString(int i)
//{
//    char str[12];
//    sprintf(str, "%d", i);
//    return str;
//}
//
//
void TXdatas(const char *msg)
{
    int i = 0;
    for(i=0 ; msg[i] != 0x00 ; i++)
    {
       TXdata(msg[i]);
    }
}
