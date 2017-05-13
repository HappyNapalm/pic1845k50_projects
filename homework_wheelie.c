/*
 * File:   homework_wheelie.c
 * Author: Ian Church Weber
 *
 * Created on March 13, 2016, 1:36 PM
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

unsigned int ROTATION_LOW;      //increase on a count of 128 THETA_TICKS
                                //keep track of 100K rotations
unsigned int ROTATION_HIGH;     //counts the other 50K that low can't
unsigned char THETA_TICKS;      //counts the impulses from 0 to 128
                                //bias the count to get a flag.
                                //counts on a high input.
                                //measure how long it is high.
unsigned char MOTOR_RUNNING;    //flag for if the motor is running
                                //turns off if no motion if high for 500ms
unsigned char STILL_RUNNING;    //trigger for length. Calculated that 19 ~ 500ms
                                //with a 1MHz clock
/*

 So, you'll need to use three timers. 
 * 1 will watch and increment THETA_TICKS
 * 1 will find the time HIGH
 * 1 will operate the PWM for adjustments. 
 * 
 * The flags on THETA_TICKS will increment the Rotation count
 * You will be using a lot of interrupts
 */

void Timer0_init(void);         //used to trigger fall
void Timer1_init(void);         //use as the THETA_TICKS
void CCP2_PWM_init(void);       //used as output
void Timer3_init(void);         //count time between rise and fall
void init_interrupt(void);
void init_general(void);
void init_ADC(void);

void interrupt excitment(void);


void main(void) {
    init_general();
    while(1){
        if(THETA_TICKS >= 128){ //converts movement into full rotation
            if(ROTATION_LOW >= 50000){
                ROTATION_HIGH++;
            }
            else{
                ROTATION_LOW++;
            }
            THETA_TICKS = 0;
        }
        if(STILL_RUNNING >= 19){//19 is the calculated limit for 500 ms
            MOTOR_RUNNING = 0;  //assignment wants no run after 500 ms
                                //calculated for 1MHz clock
        }
        
    }
    return;
}

//modify your clock?
//base speed is 1 MHz
//wire together the pins need to make this work!!!
//right now its external. Just focus on internal.
void Timer0_init(void){
    T0CONbits.TMR0ON = 0;       //timer off
    T0CONbits.T08BIT = 1;       //set for 8-bits (0 to 255)
    T0CONbits.T0CS = 1;         //set on external clocks
                                //the encoding is being treated like a clock
    T0CONbits.T0SE = 0;         //counts a falling pulse edge 
    T0CONbits.PSA = 0;          //not using a prescaler, ignore the lower ones
                                //makes the scaling 1:1
    T0CONbits.T0PS2 = 0;        //next bits control the prescaling 
    T0CONbits.T0PS1 = 0;        //check datasheet for more information
    T0CONbits.T0PS0 = 0;
    
    TMR0L = 255;                //pre-bias to trigger on the first fall
    TMR0H = 0;
}

void Timer1_init(void){
    T1CONbits.RD16 = 0;         //set for 8-bit mode
    T1CONbits.T1RUN = 0;        //set for an outside source (encoder)
    T1CONbits.T1CKPS1 = 0;      //prescaler for inputs
    T1CONbits.T1CKPS0 = 0;
    T1CONbits.T1OSCEN = 0;      //shuts off the internal oscillator
    T1CONbits.T1SYNC = 1;       //no external sync
    T1CONbits.TMR1CS = 1;       //external clock chosen
    T1CONbits.TMR1ON = 0;       //timer off
    TMR1L = 255;                //pre-bias for trigger
    TMR1H = 0;
    // using this as motor watchdog.
    //runs in 8-bit mode
    //find the counting register and set it to 128
}

void CCP2_PWM_init(void){ //fix a little later?
    //Timer 2 configuration
    T2CONbits.T2OUTPS3 = 0;     //postscale the output
    T2CONbits.T2OUTPS2 = 0;
    T2CONbits.T2OUTPS1 = 0;
    T2CONbits.T2OUTPS0 = 0;
    T2CONbits.TMR2ON = 0;       //timer off
    T2CONbits.T2CKPS1 = 0;      //prescale 
    T2CONbits.T2CKPS0 = 0;
    //use this as an output register maybe?
    //look at part three for specifics on outputs
    //CCP configuration
    TRISCbits.RC1 = 1;          //set RC1 for PWM for CCP
    
    CCP2CONbits.CCP2M3 = 1;     //set the mode
    CCP2CONbits.CCP2M2 = 1;     //set the mode
    CCP2CONbits.CCP2M1 = 1;     //Don't Care for PWM
    CCP2CONbits.CCP2M0 = 1;     //Don't Care for PWM
    
    PR2 = 12;                   //biasing register for desired frequency
    
    CCPR2L = 26;                //bias for 50% duty cycle
    
    T2CONbits.TMR2ON = 1;       //start the timer
    
}

void Timer3_init(void){
    T3CONbits.RD16 = 1;         //16 bits
    T3CONbits.T3CCP2 = 0;       //Don't Care
    T3CONbits.T3CCP1 = 0;       //Don't Care
    T3CONbits.T3CKPS1 = 0;      //pre-scale
    T3CONbits.T3CKPS0 = 0;      //set to 1:1
    T3CONbits.T3SYNC = 0;       //ignored
    T3CONbits.TMR3CS = 0;       //internal clock
    T3CONbits.TMR3ON = 0;       //timer off
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
    
    PIE1bits.TMR1IE = 1;        //TMR1 enable
    PIE1bits.TMR2IE = 1;        //TMR2 match enable
    PIE2bits.TMR3IE = 1;        //TMR3 enable
    PIR1bits.TMR1IF = 0;        //clear TMR1 flag
    PIR1bits.TMR2IF = 0;        //clear TMR2 flag
    PIR2bits.TMR3IF = 0;        //clear TMR3 flag

}

void init_general(void){
	TRISBbits.TRISB0 = 1;       //enables button port
	TRISD = 0x0;                //PORTD is outputs: this section is unneeded
    INTCON2bits.RBPU = 0;       //set pull-ups for the button
    init_interrupt();
    Timer1_init();
    CCP2_PWM_init();
    Timer0_init();
    Timer3_init(); 
    
    T0CONbits.TMR0ON = 1;       //start the external timer
    T1CONbits.TMR1ON = 1;       //start the external timer
}

void interrupt excitment(void){
    
    if(PIR1bits.TMR1IF = 1){    //trigger for a rising edge from the motor
        T3CONbits.TMR3ON = 1;   //start count for the pulse length
        THETA_TICKS++;          //increase counter
        MOTOR_RUNNING = 1;      //reset when a new rising edge introduced
        STILL_RUNNING = 0;      //reset when a new rising edge introduced
        PIR1bits.TMR1IF = 0;    //reset flag
        TMR1L = 255;            //re-bias timer 1
        TMR0L = 255;            //ensure that timer 0 is ready for trigger
                                //consider remove redundancy if it affects time
                                //too much        
    }
    if(INTCONbits.TMR0IF = 1){  //triggers on the a falling edge from motor
        T3CONbits.TMR3ON = 0;   //stop counting
        INTCONbits.TMR0IF = 0;  //clear timer0 flag
        TMR1L = 255;            //ensure a trigger on a rise
        TMR0L = 255;            //redundant re-bias
    }
    if(PIR2bits.TMR3IF = 1){    //triggers and increments motor timer
        STILL_RUNNING++;        //this is what is used to determine running time
        PIR2bits.TMR3IF = 0;    //clear timer3 flag
    }
    if(PIR1bits.TMR2IF = 1){
        TRISCbits.RC1 = 0;      //makes an output for the PWM
        PIR1bits.TMR2IF = 0;    //clear flag
    }
    
}