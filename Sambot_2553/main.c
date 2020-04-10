#include <msp430.h>
#include "board.h"
#include "uart.h"
#include "spi.h"
#include "motor.h"

#define MOTORS_FREQUENCY    33333   // motors period T=30 ms (0.0333 s)
#define MOTORS_DUTYCYCLE    1250    // motors duty cycle 50% soit 0.00125 ms 1250 µs
#define STEP_ANGLE          72      // step for 1 deg  PW[500-3000 µs]/180 deg   72
#define PW_MIN              1000     // 1000/55=18 55*18=990
#define PW_MAX              2000    // 2000/55=36 55*36=1980

#define SCK         BIT5            // Serial Clock
#define DATA_OUT    BIT6            // DATA out
#define DATA_IN     BIT7            // DATA in

// prototypes
void init_Timer(void);
void execute_uart_command(unsigned char);
void balayage(void);

volatile unsigned int up = 0;    // sens de variation
volatile unsigned int cmd = 0;   // periode du signal de commande moteur
volatile unsigned int on = 0;   // Met en route le robot
volatile unsigned int log = 0;  // Pour afficher les valeurs des capteurs
volatile unsigned int servo = 0;
volatile unsigned int count = 0;
volatile unsigned int obstacle_value = '0';
volatile unsigned int hole_value = '0';
volatile char txSensorsValues[4];
volatile int timerCounter = 0;
/**
 * main.c
 */
int main(void)
{
    // Stop watchdog timer to prevent time out reset
    WDTCTL = WDTPW | WDTHOLD;

    // Initialisation
    init_board();

    // Init timer
    init_Timer();

    // Spi
    init_USCI();

    // Init motor
    init_motor();

    // Init UART
    init_UART();
    __enable_interrupt(); // general interrupts enable & Low Power Mode
    send_spi('x');
    while(1) {
        if(on == 1) {
            if(obstacle_value > 28) {
                R_avancer();
            } else {
                R_tourner_gauche();
            }
        } else {
            _delay_cycles(10000);
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
        //P1OUT^=BIT0;
        execute_uart_command(UCA0RXBUF);
    }
    // SPI -> 2231 repond au 2553 si il y a un obstacle ou pas
    else if (IFG2 & UCB0RXIFG)
    {

        while( (UCB0STAT & UCBUSY) && !(UCB0STAT & UCOE) );
        while(!(IFG2 & UCB0RXIFG));

        if(!((UCB0RXBUF&BIT7)==BIT7)) {
            obstacle_value = (UCB0RXBUF&BIT6) + (UCB0RXBUF&BIT5) + (UCB0RXBUF&BIT4) + (UCB0RXBUF&BIT3) + (UCB0RXBUF&BIT2) + (UCB0RXBUF&BIT1) + (UCB0RXBUF&BIT0);
        } else{
            hole_value = (UCB0RXBUF&BIT6) + (UCB0RXBUF&BIT5) + (UCB0RXBUF&BIT4) + (UCB0RXBUF&BIT3) + (UCB0RXBUF&BIT2) + (UCB0RXBUF&BIT1) + (UCB0RXBUF&BIT0);
        }

    }
}


#pragma vector=TIMER0_A1_VECTOR
__interrupt void ma_fnc_timer(void)
{
    //send_spi('x');
    //send_spi('y');
    if((TA0CCTL2&CCIFG)==CCIFG) {
        timerCounter++;
        if(timerCounter == 1) {
            send_spi('x');
        } else {
            send_spi('y');
            timerCounter = 0;
        }

        balayage();
        if(log == 1) {
            count++;
            if(count >= 5) {
                count = 0;
                int tmp_obstacle_value = obstacle_value;
                if(tmp_obstacle_value >= 100) {
                    txSensorsValues[0] = '1';
                    tmp_obstacle_value -= 100;
                } else {
                    txSensorsValues[0] = '0';
                }
                txSensorsValues[1] = tmp_obstacle_value/10 + 48;
                txSensorsValues[2] = tmp_obstacle_value%10 + 48;
                txSensorsValues[3] = '\0';
                TXdatas("US sensor:");
                TXdatas(txSensorsValues);
                TXdatas(" / ");
                tmp_obstacle_value = hole_value;
                if(tmp_obstacle_value >= 100) {
                    txSensorsValues[0] = '1';
                    tmp_obstacle_value -= 100;
                } else {
                    txSensorsValues[0] = '0';
                }
                txSensorsValues[1] = tmp_obstacle_value/10 + 48;
                txSensorsValues[2] = tmp_obstacle_value%10 + 48;
                txSensorsValues[3] = '\0';
                TXdatas("IR sensor:");
                TXdatas(txSensorsValues);
                TXdatas("\r\n");
            }
        }
        TA0CCTL2&=~CCIFG;
    }
}

void balayage(void) {
    if(servo == 1) {
        // clockwise
        if (!up) {
            //P1OUT &= ~BIT6;
            // if hasn't reach end
            if (cmd > (PW_MIN + STEP_ANGLE)) {
                cmd -= STEP_ANGLE;
            }
            else {
                cmd = PW_MIN;
                up = 1;
            }
            //P1OUT ^= BIT0;
        }
        // counterclockwise
        else {
            //P1OUT &= ~BIT0;
            // if hasn't reach end
            if (cmd < (PW_MAX - STEP_ANGLE)) {
                cmd += STEP_ANGLE;
            }
            else {
                cmd = PW_MAX;
                up = 0;
            }
            //P1OUT ^= BIT6;
        }
        TA0CCR1 = cmd;
    }

}

void init_Timer(void)
{
    BCSCTL1=CALBC1_1MHZ;//frequenced’horloge 1MHz
    DCOCTL=CALDCO_1MHZ;
    TA0CTL &= ~MC_0; // arrêt timer
    TA0CCR0 = MOTORS_FREQUENCY; // periode du signal PWM 2KHz
    TA0CTL = (TASSEL_2 | MC_1 | ID_0 | TACLR); // SMCLK timer A + mode Up
    TA0CCTL1 = 0 | OUTMOD_7; // mode comparaison
    TA0CCTL2 = 0 | CCIE;
    TA0CCR2 = 27776;
    //TA1CTL |= TAIE; // faudra enlever ca
    cmd=MOTORS_DUTYCYCLE;
    up=1;
    TA0CCR1=cmd;
}



void execute_uart_command(unsigned char c)
{
    TXdata(c);
    TXdatas("\r\n");
    switch (c)
    {
        // robot off
        case '0':
            P1OUT &= ~BIT0;
            TXdatas("Auto-pilot off\r\n");
            on = 0;
            break;
        // robot on
        case '1':
            P1OUT |= BIT0;
            TXdatas("Auto-pilot on\r\n");
            on = 1;
            break;
        // log off
        case '2':
            TXdatas("Log off\r\n");
            log = 0;
            break;
        // log off
        case '3':
            TXdatas("Log on\r\n");
            log = 1;
            break;
        case 'z':
            TXdatas("Moving forward\r\n");
            R_avancer();
            break;
        case 'q':
            TXdatas("Moving left\r\n");
            R_tourner_gauche();
            break;
        case 'd':
            TXdatas("Moving right\r\n");
            R_tourner_droite();
            break;
        case 's':
            TXdatas("Moving backward\r\n");
            R_reculer();
            break;
        case 'l':
            TXdatas("Servomotor on\r\n");
            servo = 1;
            break;
        case 'm':
            TXdatas("Servomotor off\r\n");
            servo = 0;
            break;
        case 'h':
            TXdatas("Commands list:\r\n'0' - Stop auto-robot\r\n'1' - Start auto-robot\r\n'2' - Log IR/Ultrasound sensors off\r\n'3' - Log IR/Ultrasound sensors on"
                    "\r\n'z' - Move forward\r\n'q' - Move left\r\n's' - Move backward\r\n'd' - Move right\r\n'l' - Servomotor on\r\n'm' - Servomotor off\r\n");
            break;
        default:
            TXdatas("Command unknown\r\nType 'h' for help");
            break;
    }
}

