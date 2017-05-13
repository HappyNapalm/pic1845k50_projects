/*
 * File:   spi_slave_irq.c
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

unsigned char data_in;
unsigned char data_out;
char a[]{0,1,2,3,4,5,6,7};

void main(void) {
    init();
    while(1){
        recieve();
        transmit();
    }
    return;
}
void init(void){
    spi_slave_config(void);
    init_ADC(void);
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
void init_ADC(void){
    ANSEL = 11111111; //removes digital buffer for ANS0
    TRISD = 0b00000000; //set outputs for PORTD
    TRISA = 0xff;       //set inputs for PORTA
    ADCON1bits.VCFG0=0; //sets and reference value
    ADCON1bits.VCFG1=0;
    ADCON2 = 0b0001000; //left justified, 2TAD, FOCS/2
    ADCON0bits.ADON=1;  //sets and enables ADC	
}
void ADC_time(void){
    GO_nDONE = 1; //starts ADC
    while(GO_nDONE);//waits for conversion
}
void transmit(void){//double check this one
    for(int x = 0;x>8;x++){//go through the entire array and provide the info
        SSPBUF = a[x];
        while(!SSPSTATbits.BF);
    }
}
void recieve(void){
    data_in = SSPBUF;
    while(!SSPSTATbits.BF);
    if(data_in = 80){
        read_irp();
    }
}
void read_irp(void){
    ADCON0bits.CHS = 0b0000;    //select port (AN0 in this cass)
    ADC_time();                 //start timer
    a[0] = ADRESH;              //store value
    
    ADCON0bits.CHS = 0b0001;
    ADC_time();
    a[1] = ADRESH;
    
    ADCON0bits.CHS = 0b0010;
    ADC_time();
    a[2] = ADRESH;
    
    ADCON0bits.CHS = 0b0011;
    ADC_time();
    a[3] = ADRESH;
    
    ADCON0bits.CHS = 0b0100;
    ADC_time();
    a[4] = ADRESH;
    
    ADCON0bits.CHS = 0b0110;
    ADC_time();
    a[5] = ADRESH;
    
    ADCON0bits.CHS = 0b0111;
    ADC_time();
    a[6] = ADRESH;
    
    ADCON0bits.CHS = 0b1000;
    ADC_time();
    a[7] = ADRESH;
    
}