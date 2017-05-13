/*
 * File:   master_file.c
 * Author: Ian Church Weber
 *
 * Created on March 18, 2016, 12:19 PM
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

//PORT A -> analog inputs
//PORT C -> SPI uses
//PORT D -> Slave select outputs

#define Slave_Sonar PORTDbits.RD1       //these refer to the enable pins used
#define Slave_IRP PORTDbits.RD2
#define Slave_Motor_Gen PORTDbits.RD3
//incorporate the motor reading in this.

unsigned char data_out;
unsigned char data_in;
int irp_data[8];
int sonar_data[8];

int motorA_speed_data[64000];
int motorB_speed_data[64000];
int int_cnt;

void spi_master_config(void);
void init_interrupt(void);
void init_ADC(void);
void init(void);

void recieve(void);
void transmit(void);

void main(void) {
    init();
    T0CONbits.TMR0ON = 1;//start timer for motor check
    while(1){
        
        data_out = 80;  //set to value
        Slave_IRP = 0;  //select
        transmit();     //start transmission
        for(char x = 0;x>8;x++){// 8 sensors, 8 pieces of data
            recieve();//recieve
            irp_data[x] = data_in;//transfer into an array
        }
        data_out = 100  //set value
        Slave_Sonar = 0;//select
        transmit();     //start transmission
        for(char x = 0;x>8;x++){// 8 sensors, 8 pieces of data
            recieve();//recieve
            sonar_data[x] = data_in;//transfer into an array
        }
        data_out = 128;//130,132,134 are also acceptable. Each goto a subroutine
        Slave_Motor_Gen = 0;//select
        transmit();
        recieve();      //returns no useful data
    
    }
    return;
}
//set your clock here to 16 MHZ
void spi_master_config(void){
    SSPCON1bits.SSPEN = 1;
    SSPCON1bits.CKP = 0;    //idle clock low
    SSPCON1bits.SSPM3 = 0;  //current slave clock set up -> 4/Fosc
    SSPCON1bits.SSPM2 = 0;
    SSPCON1bits.SSPM1 = 0;
    SSPCON1bits.SSPM0 = 0;
    
    TRISCbits.RC3 = 0;  //sets clock output on SCK pin
    TRISCbits.RC4 = 1;  //sets SDI as input
    TRISCbits.RC5 = 0;  //sets SDO as output
    
    SSPSTATbits.SMP = 0;    //sample from middle of pulse  
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
}
void init_ADC(void){
    ANSEL = 11111111; //removes digital buffer for ALL ANSX 
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
void init(void){
    OSCCONbits.IRCF = 0b111;    //set our clock to 16 MHz
    init_ADC(); //initalize ADC
    init_interrupt();   //initialize interrupts
    spi_master_config();    //start master config
    Slave_Sonar = 1;    //Slave select is active low. 
    Slave_IRP = 1;      //so turning these high disable them
    Slave_Motor_Gen = 1;
}
void transmit(void){    //data out
    SSPBUF = data_out;
    while(!SSPSTATbits.BF);
}
void recieve(void){     //data in
    data_in = SSPBUF;
    while(!SSPSTATbits.BF);
}

void read_motorA (void){
    ADCON0bits.CHS = 0b0000;    //read AN0 
    ADC_time();                 //read ADC
    motorA_speed_data[int_cnt] = ADRESH;            //Assign 8 MSB to var   
}
void read_motorB (void){
    ADCON0bits.CHS = 0b0001;    //read AN1
    ADC_time();                 //read ADC
    motorB_speed_data[int_cnt] = ADRESH;            //Assign 8 MSB to var
}
void Timer0_init(void){
    T0CONbits.TMR0ON = 0;       //timer off
    T0CONbits.T08BIT = 1;       //set for 8-bits (0 to 255)
    T0CONbits.T0CS = 0;         //set for internal clock
    T0CONbits.T0SE = 0;         //counts a falling pulse edge 
    T0CONbits.PSA = 1;          //not using a prescaler, ignore the lower ones
    T0CONbits.T0PS2 = 1;        //next bits control the prescaling 
    T0CONbits.T0PS1 = 1;        //check datasheet for more information
    T0CONbits.T0PS0 = 1;        //set 1:32 right now
    
}

void interrupt end_game(void){
    if(INTCONbits.TMR0IF = 1){ //interrupt to check every 1 millisecond
        int_cnt++;          //allows lots of data to be stored
        read_motorA();      //may cause slow down
        read_motorB();
        INTCONbits.TMR0IF = 0;  //reset flag
    }
}