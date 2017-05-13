/*
 * File:   spi_slave_sonar.c
 * Author: Church
 *
 * Created on March 18, 2016, 12:55 PM
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

//enable sonar bit -> time 50ms -> disable sonar bit -> record information
//use PORT A to read the pins
//use PORT D as output pins

unsigned char data_in;
unsigned char data_out;
char a[8];

void main(void){
    init();
    while(1){
        recieve();
        transmit();
    }
    return;
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
    TRISAbits.RA5 = 1;
}
void init_ADC(void){
    ANSEL = 11111111; //removes digital buffer for ANS0
    TRISD = 0b00000000; //set outputs for PORTD
    TRISA = 0xff;       //set inputs for PORTA
    ADCON1bits.VCFG0=0; //sets and reference value
    ADCON1bits.VCFG1=0;
    ADCON2 = 0b0001000; //left justified, 2TAD, FOCS/2
    ADCON0bits.ADON=1;  //sets ADC to pin RA0 and enables ADC	
}
void ADC_time(void){
    GO_nDONE = 1; //starts ADC
    while(GO_nDONE);//waits for conversion
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
void transmit(void){
    for(int x = 0;x>8;x++){//go through the entire array and provide the info
        SSPBUF = a[x];
        while(!SSPSTATbits.BF);
    }
}
void recieve(void){
    data_in = SSPBUF;
    while(!SSPSTATbits.BF);
    if(data_in = 100){
        read_sonar();
    }
}
void interrupt ass (void){
    if(INTCONbits.TMR0IF){
        timer0++;
        INTCONbits.TMR0IF = 0;
    }
}

void init(void){
    init_interrupt(void);
    spi_slave_config(void);
}

void read_sonar(void){//refine so it doesn't have to read the ones pinged by IRP
    PORTDbits.RD0 = 1;
    ADCON0bits.CHS = 0b0000;
    ADC_time();
    a[0] = ADRESH;
    PORTDbits.RD0 = 0;
    
    PORTDbits.RD1 = 1;
    ADCON0bits.CHS = 0b0001;
    ADC_time();
    a[1] = ADRESH;
    PORTDbits.RD1 = 0;
    
    PORTDbits.RD2 = 1;
    ADCON0bits.CHS = 0b0010;
    ADC_time();
    a[2] = ADRESH;
    PORTDbits.RD2 = 0;
    
    PORTDbits.RD3 = 1;
    ADCON0bits.CHS = 0b0011;
    ADC_time();
    a[3] = ADRESH;
    PORTDbits.RD3 = 0;
    
    PORTDbits.RD4 = 1;
    ADCON0bits.CHS = 0b0100;
    ADC_time();
    a[4] = ADRESH;
    PORTDbits.RD4 = 0;
    
    PORTDbits.RD5 = 1;
    ADCON0bits.CHS = 0b0110;//avoid channel 5. occupied by slave select
    ADC_time();
    a[5] = ADRESH;
    PORTDbits.RD5 = 0;
    
    PORTDbits.RD6 = 1;
    ADCON0bits.CHS = 0b0111;
    ADC_time();
    a[6] = ADRESH;
    PORTDbits.RD6 = 0;
    
    PORTDbits.RD7 = 1;
    ADCON0bits.CHS = 0b1000;
    ADC_time();
    a[7] = ADRESH;
    PORTDbits.RD7 = 0;
}