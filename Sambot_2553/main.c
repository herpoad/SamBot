#include <msp430.h> 
#include "board.h"
#include "uart.h"
#include "spi.h"
#include "motor.h"

#define MOTORS_FREQUENCY    33333   // motors period T=30 ms (0.0333 s)
#define MOTORS_DUTYCYCLE    1250    // motors duty cycle 50% soit 0.00125 ms 1250 µs
#define STEP_ANGLE          72      // step for 1 deg  PW[500-3000 µs]/180 deg
#define PW_MIN              504     // 500/72=7 7*72=504
#define PW_MAX              2448    // 3000/72=41 41*72=295

#define SCK         BIT5            // Serial Clock
#define DATA_OUT    BIT6            // DATA out
#define DATA_IN     BIT7            // DATA in

// prototypes
void init_Timer(void);
void init_Timer2(void);
void execute_uart_command(unsigned char);

unsigned int up = 0;    // sens de variation
unsigned int cmd = 0;   // periode du signal de commande moteur
unsigned int on = 0;   // Met en route le robot
unsigned int log = 0;  // Pour afficher les valeurs des capteurs
unsigned char obstacle = 0; // '1' si ostacle détécté, '0' sinon
/**
 * main.c
 */
int main(void)
{
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;

    // Initialisation
    init_board();
    // Balayge
    init_Timer();
    // Le timer qui va envoyé une impulsion a une certaine fréquence au servomoteur
    init_Timer2();
    // Spi
    init_USCI();
    //Uart
    init_UART();
   
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    __bis_SR_register(GIE); // interrupts enabled

    while(1) {
        R_avancer(on);
        send_spi('0');
        if(log == 1) {
            TXdata(obstacle);
        }
    }
}

// SPI & UART
#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR()
{
    // UART -> 2553 recoit une commande de l'ordinateur
    if (IFG2 & UCA0RXIFG)
    {
        execute_uart_command(UCA0RXBUF);
    }
    // SPI -> 2231 repond au 2553 si il y a un obstacle ou pas
    else if (IFG2 & UCB0RXIFG)
    {
        while( (UCB0STAT & UCBUSY) && !(UCB0STAT & UCOE) );
        while(!(IFG2 & UCB0RXIFG));
        obstacle = UCB0RXBUF;
        if(obstacle == '1') {
            R_tourner_droite();
            P1OUT |= BIT0;
        } else {
            P1OUT &= ~BIT0;
        }
    }
}

// Balayage du capteur
#pragma vector=TIMER0_A1_VECTOR
__interrupt void ma_fnc_timer(void)
{
    P1OUT ^= BIT0;
    // clockwise
    if (!up) {
        P1OUT &= ~BIT6;
        // if hasn't reach end
        if (cmd > (PW_MIN + STEP_ANGLE)) {
            cmd -= STEP_ANGLE;
        }
        else {
            cmd = PW_MIN;
            up = 1;
        }
        P1OUT ^= BIT0;
    }
    // counterclockwise
    else {
        P1OUT &= ~BIT0;
        // if hasn't reach end
        if (cmd < (PW_MAX - STEP_ANGLE)) {
            cmd += STEP_ANGLE;
        }
        else {
            cmd = PW_MAX;
            up = 0;
        }
        P1OUT ^= BIT6;
    }
    TA1CCR1 = cmd;

    TA0CTL&=~TAIFG;
}

void init_Timer(void)
{
    TA1CTL &= ~MC_0; // arrêt timer
    TA1CCR0 = MOTORS_FREQUENCY; // periode du signal PWM 2KHz
    TA1CTL = (TASSEL_2 | MC_1 | ID_0 | TACLR); // SMCLK timer A + mode Up
    TA1CCTL1 = 0 | OUTMOD_7; // mode comparaison
}

void init_Timer2(void)
{
    BCSCTL1=CALBC1_1MHZ;//frequenced’horloge 1MHz
    DCOCTL=CALDCO_1MHZ;
    P1DIR|=BIT0;//bit0port1ensortie
    TA0CTL=0|(TASSEL_2|ID_0);//sourceSMCLK,pas de predivision ID_0
    TA0CTL|=MC_1;//comptage en modeup
    TA0CTL|=TAIE;//autorisation interruption
    TA0CCR0=27776;
}

void execute_uart_command(unsigned char c)
{
    TXdata(c);
    TXdatas("\r\n");
    switch (c)
    {
        // robot off
        case '0':
            on = 0;
            break;
        // robot on
        case '1':
            on = 1;
            break;
        // log off
        case '2':
            log = 0;
            break;
        // log off
        case '3':
            log = 1;
            break;
        default:
            TXdatas("Commande inconnue\r\n");
            break;
    }
}


