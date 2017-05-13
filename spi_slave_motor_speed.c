/*
 * File:   spi_slave_motor_speed.c
 * Author: Church
 *
 * Created on March 18, 2016, 12:56 PM
 */
#include <xc.h>
#include<stdio.h>
#include<stdlib.h>
#include<htc.h>

/** C O N F I G U R A T I O N   B I T S ******************************/
#pragma config FOSC = INTIO67
//#pragma config WDTEN = OFF, LVP = OFF, MCLRE = OFF
#pragma config WDTEN = OFF, LVP = OFF

/** I N C L U D E S **************************************************/
#include "p18f45k20.h"

/*#define MotorA = PORTDbits.RD0      // motors set to pins that will hold PWM
#define MotorB = PORTDbits.RD1
#define MotorC = PORTDbits.RD2
#define MotorD = PORTDbits.RD3
#define directionA = PORTEbits.RE0  // 0 -> Forward, 1 -> Reverse
#define directionB = PORTEbits.RE1
#define directionC = PORTEbits.RE2
#define directionD = PORTEbits.RE3 
 */

//only receives information

unsigned char MotorA_state;     //left wheel
unsigned char MotorB_state;     //right wheel
//unsigned char directionA;       
//unsigned char directionB;       
unsigned char data_in;
unsigned char data_out;

void init_interrupt(void);//
void Timer0_init(void);//
void CCP2_PWM_init(void);//
void forward(void);
void backward(void);
void turn_left(void);
void turn_right(void);
void spi_slave_config(void);
void transmit(void);
void recieve(void);
void init(void);

//remember to start the timers
void main(void) {
    MotorA_state = 0;
    MotorB_state = 0;
    data_in = 0;
    data_out = 0;
    init();
    
    while(1){
        recieve();
        transmit();        
    }
    return;
}
void init(void){
    init_interrupt();//
    Timer0_init();//
    CCP2_PWM_init();//
    spi_slave_config();
}
void init_interrupt(void){
	INTCON2bits.INTEDGO = 0;    //interrupt on falling edge
	INTCONbits.INTOIF = 0;      //clear flags
	INTCONbits.INT0IE = 1;      //enables INT0 interrupts
	
	RCONbits.IPEN = 1;          //enable priority levels
	INTCONbits.GIEL = 1;        //interrupts being used
	INTCONbits.GIEH = 1;
    
	INTCONbits.TMR0IE = 1;      //enables timer0 interrupts
	INTCONbits.TMR0IF = 0;      //clears flag for TMR0
	INTCON2bits.TMR0IP = 0;     //set to low priority for TMR0
    
    PIE1bits.TMR2IE = 1;        //TMR2 match enable
    PIR1bits.TMR2IF = 0;        //clear TMR2 flag
}
void Timer0_init(void){     //MotorA
    T0CONbits.TMR0ON = 0;       //timer off
    T0CONbits.T08BIT = 1;       //set for 8-bits (0 to 255)
    T0CONbits.T0CS = 0;         //set for internal clock
    T0CONbits.T0SE = 0;         //counts a falling pulse edge 
    T0CONbits.PSA = 1;          //not using a prescaler, ignore the lower ones
    T0CONbits.T0PS2 = 1;        //next bits control the prescaling 
    T0CONbits.T0PS1 = 0;        //check datasheet for more information
    T0CONbits.T0PS0 = 0;        //set 1:32 right now

}
void CCP2_PWM_init(void){   //MotorB
    //Timer 2 configuration
    T2CONbits.T2OUTPS3 = 0;     //postscale the output
    T2CONbits.T2OUTPS2 = 0;
    T2CONbits.T2OUTPS1 = 0;
    T2CONbits.T2OUTPS0 = 0;
    T2CONbits.TMR2ON = 0;       //timer off
    T2CONbits.T2CKPS1 = 0;      //prescale 
    T2CONbits.T2CKPS0 = 0;      //set for 1
    //CCP configuration
    TRISCbits.RC1 = 1;          //set RC1 for PWM for CCP
    
    CCP2CONbits.CCP2M3 = 1;     //set the mode
    CCP2CONbits.CCP2M2 = 1;     //set the mode
    CCP2CONbits.CCP2M1 = 1;     //Don't Care for PWM
    CCP2CONbits.CCP2M0 = 1;     //Don't Care for PWM
    
    PR2 = 63;                   //biasing register for desired frequency
    
    CCPR2L = 128;               //bias for 50% duty cycle
    
    T2CONbits.TMR2ON = 1;       //start the timer
}
void interrupt O_O (void){
    if(INTCONbits.TMR0IF){
        MotorA_state = ~MotorA_state;
        INTCONbits.TMR0IF = 0;
    }
    if(PIR1bits.TMR2IF){
        MotorB_state = ~MotorB_state;
        PIR1bits.TMR2IF = 0;
    }
}
//implementation? How long should they run for?
void forward(void){
    PORTEbits.RE0 = 0;
    PORTEbits.RE1 = 0;
}
void backward(void){
    PORTEbits.RE0 = 1;
    PORTEbits.RE1 = 1;
}
void turn_left(void){
    PORTEbits.RE0 = 0;
    PORTEbits.RE1 = 1;
}
void turn_right(void){
    PORTEbits.RE0 = 1;
    PORTEbits.RE1 = 0;
}


void spi_slave_config(void){
    SSPCON1bits.CKP = 0;    //idle clock low
    SSPCON1bits.SSPEN = 1;
    SSPCON1bits.SSPM3 = 0;  //slave mode with slave select
    SSPCON1bits.SSPM2 = 1;
    SSPCON1bits.SSPM1 = 0;
    SSPCON1bits.SSPM0 = 0;
    
    TRISCbits.RC4 = 1;  //sets SDI as input
    TRISCbits.RC5 = 0;  //sets SDO as output
    TRISCbits.RC3 = 1;  //sets clock input on SCK pin
}
void transmit(void){
    SSPBUF = 42;
    while(!SSPSTATbits.BF);
}
void recieve(void){
    data_in = SSPBUF;
    while(!SSPSTATbits.BF);
    if(data_in = 128){
        forward();
    }
    else if(data_in = 130){
        backward();
    }
    else if(data_in = 132){
        turn_left();
    }
    else if(data_in = 134){
        turn_right();
    }
}