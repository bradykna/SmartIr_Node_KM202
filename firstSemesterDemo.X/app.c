 #include <xc.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "app.h"
#include "system.h"
#include "adc1.h"
#include "uart.h"
#include "gpio.h"


/**********************************************************
 * 
 * APP Macros
 * 
*************************************************************/

// SAMPLE_FRQ = 1, = 1 SECOND; 
#define SAMPLE_FRQ 5
#define ADC1_ENABLE 0x0020     // PORTB5
#define LED1        0x0040     // PORTB6


/**********************************************************
 * 
 * APP functions
 * 
*************************************************************/
void application_init(void);
bool getConversions(void);
void timer_init(void);

/**********************************************************
 * 
 * APP variables
 * 
*************************************************************/

APP_DATA appData;
uint16_t ticker = 0;



void application_init(void)
{
    timer_init();
    ADC1_init();
    uart_init();
    GPIO_enable(LED1);
    GPIO_enable(ADC1_ENABLE);
    appData.ms = 0;
    appData.seconds = 0;
    appData.sampleFlag = false;
    appData.U1RxByte = 0x30;
}


bool getConversions(void)
{
    uint16_t i = 0;
    
    if(((appData.seconds%SAMPLE_FRQ) == 0) &&  appData.sampleFlag == true)
    {  
        GPIO_hi(ADC1_ENABLE);
        for( i = 40 ; i > 0 ; i-- ){}

                // soil moisture conversion
                appData.soilConversion = ADC1_getConversion(ADC1_SOIL_SENSOR);

                // photo sensor conversion
                appData.liteConversion = ADC1_getConversion(ADC1_PHOTORESISTOR);           

                // temperature conversion
                appData.tempConversion = ADC1_getConversion(ADC1_TEMPERATURE);


                sprintf(U1TxBuffer,"%i,",(uint16_t)appData.soilConversion);
                uart_write_string(U1TxBuffer); 

                sprintf(U1TxBuffer,"%i,",(uint16_t)appData.liteConversion);   
                uart_write_string(U1TxBuffer);              
        
                sprintf(U1TxBuffer,"%i,",(uint16_t)appData.tempConversion);
                uart_write_string(U1TxBuffer);

        GPIO_lo(ADC1_ENABLE);                 
        appData.sampleFlag = false;
    }
    return true;

}

// 
void timer_init(void)
{

    TMR1 = 0x0000;     
    T1CON = 0x8000;
    PR1 = 0x0185;
    IPC0bits.T1IP = 1;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;     
    T1CONbits.TON = 1;           
}

/* Timer 1 ISR */
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    
    IFS0bits.T1IF = 0;
    TMR1 = 0x0000;    
    ticker++;
    
    // counts seconds
    if ( ticker%10000 == 0 )   
        
    {  appData.sampleFlag = true;
       (appData.seconds == 59)? (appData.seconds=0): (appData.seconds++);
       appData.ms = 0;
       ticker = 0;      
    }
    
    // counts milliseconds
    if ( ticker%10 == 0 )
    {
        (appData.ms==999)? appData.ms=0: appData.ms++;        
    }
    
    //delay countdown
    if (appData.delay > 0)
    {
        appData.delay--;       
    }
    
}




