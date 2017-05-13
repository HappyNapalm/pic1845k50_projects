/*
 * File:   Lab_final_344.c
 * Author: Church
 *
 * Created on March 13, 2016, 11:26 PM
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

void main_init(void);
void clock_init(void);
void pwm_init(void);
void interrupt_init(void);
void comm_init(void);
void init_ADC(void);
void use_ADC(void);



void main(void) {
    main_init();
    LATD = 0;
    while(1){
        use_ADC();
        TXREG = ADRESH;
        while(!TRMT);
        while(!RCIF);
        PORTD = RCREG;
        
    }
    //return;
}

void main_init(void){
    clock_init();
    comm_init();
    init_ADC();
}
void clock_init(void){      //set internal clock to desired freq.
    OSCCONbits.IRCF2 = 1;   //desired at 16 MHz
    OSCCONbits.IRCF1 = 1;
    OSCCONbits.IRCF0 = 1;
}
void pwm_init(void){
    
}
void interrupt_init(void){
    INTCON2bits.INTEDG0 = 0; //interrupt on falling edge
	INTCONbits.INT0IF = 0; //clear flags
	INTCONbits.INT0IE = 1; //enables INT0 interrupts
	
	RCONbits.IPEN = 1;//enable priority levels
	INTCONbits.GIE = 1;//interrupts being used
	INTCONbits.PEIE = 1;
	INTCONbits.TMR0IE = 1;//enables timer0 interrupts
	INTCONbits.TMR0IF = 0;//clears flag
    INTCON2bits.TMR0IP = 0;
    
    PIE1bits.TXIE = 1;
    PIE1bits.RCIE = 1;
}
void comm_init(void){
   /* TXSTAbits.SYNC = 0; //asynch 
    TXSTAbits.TXEN = 1; //enable TX
    
    RCSTAbits.SPEN = 1; //configures RX and TX pins as serial ports
    TRISCbits.RC7 = 0;
    TRISCbits.TRISC7=0;
    TRISCbits.RC6 = 1;
    TRISCbits.TRISC6=1;
    RCSTAbits.CREN = 1; //enables RX
    
    BAUDCONbits.DTRXP = 1;  //RX active high
    BAUDCONbits.CKTXP = 1;  //Idle TX is 1*/
    SSPSTATbits.SMP = 0;
    SSPCON1bits.WCOL = 1;
    SSPCON1bits.SSPEN = 1;
    TRISCbits.RC5 = 0;
    TRISCbits.RC3 = 0;
    
}

void init_ADC(void){
    ANSELbits.ANS0 = 1;//removes digital buffer for ANS0
    TRISD = 0b00000000;//set outputs for PORTD
    TRISA = 0xff;//set inputs for PORTA
    ADCON1bits.VCFG0=0;//sets and reference value
    ADCON1bits.VCFG1=0;
    ADCON2 = 0b0001000;//left justified, 2TAD, FOCS/2
    ADCON0bits.ADON=1;//sets ADC to pin RA0 and enables ADC	
}

void use_ADC(void){
    GO_nDONE = 1; //starts ADC
    while(GO_nDONE);//waits for conversion
}
