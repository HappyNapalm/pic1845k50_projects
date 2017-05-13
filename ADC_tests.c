/*
 * File:   ADC_tests.c
 * Author: Church
 *
 * Created on February 22, 2016, 5:04 AM
 */
#include <stdio.h>
#include <stdlib.h>
#include <xc.h>
#include <htc.h>

/** C O N F I G U R A T I O N   B I T S ******************************/
#pragma config FOSC = INTIO67
//#pragma config WDTEN = OFF, LVP = OFF, MCLRE = OFF
#pragma config WDTEN = OFF, LVP = OFF

/** I N C L U D E S **************************************************/
#include "p18f45k20.h"

#define Dial PORTAbits.RA0
#define Button PORTBbits.RB0
int a;
int i;
int j;
int k;
int n;
int state;
void delay(int time);
void ADC_time(void);
void delay(int time) {
    int counter = 0;
    while (counter < time){
        while(TMR0L==128);          //timer goes for 256x and then TMR0IF changes to 1
        TMR0L = 128;
    counter++;
    }
}

void ADC_time(void){
    GO_nDONE = 1; //starts ADC
    while(GO_nDONE);//waits for conversion
    a = ADRESH;
}
void main(void) {
    a=0;
    i=0;
    j=0;
    k=1;
    n=1;
    state=0;
    TRISBbits.TRISB0 = 1;
    ANSELbits.ANS0 = 1;//removes digital buffer for ANS0
    TRISD = 0b00000000;//set outputs for PORTD
    TRISA = 0xff;//set inputs for PORTA
    ADCON1bits.VCFG0=0;//sets and reference value
    ADCON1bits.VCFG1=0;
    ADCON2 = 0b0001000;//left justified, 2TAD, FOCS/2
    ADCON0bits.ADON=1;//sets ADC to pin RA0 and enables ADC
    
    // Initialize Timer
    INTCONbits.TMR0IF = 0;          // clear roll-over interrupt flag
    T0CON = 0b11010100;             // prescale - increments every instruction clock
    //T0CON = 0b00000001;           // prescale 1:4 - four times the delay.
    TMR0H = 0;                      // clear timer - always write upper byte first
    TMR0L = 128;
    T0CONbits.TMR0ON = 1;           // start timer
    
    while(1){
        if(k>16){
            k=0;
        }
        LATD=0;
        ADC_time();
        delay(a);
        switch(k){
            case 1:
                LATDbits.LATD0=1;
                break;
            case 2:
                LATDbits.LATD1=1;
                break;
            case 3:
                LATDbits.LATD2=1;
                break;
            case 4:
                LATDbits.LATD3=1;
                break;
            case 5:
                LATDbits.LATD4=1;
                break;
            case 6:
                LATDbits.LATD5=1;
                break;
            case 7:
                LATDbits.LATD6=1;
                break;
            case 8:
                LATDbits.LATD7=1;
                break;
            case 9:
                LATDbits.LATD7=1;
                break;
            case 10:
                LATDbits.LATD6=1;
                break;
            case 11:
                LATDbits.LATD5=1;
                break;
            case 12:
                LATDbits.LATD4=1;
                break;
            case 13:
                LATDbits.LATD3=1;
                break;
            case 14:
                LATDbits.LATD2=1;
                break;
            case 15:
                LATDbits.LATD1=1;
                break;
            case 16:
                LATDbits.LATD0=1;
                break;
        }
        k++;
        delay(a);
    }  
    return;
}
