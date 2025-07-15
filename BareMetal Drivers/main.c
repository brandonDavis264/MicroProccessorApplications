// Lab 1, uP2 Fall 2023
// Created: 2023-07-31
// Updated: 2023-08-01
// Lab 1 is intended to introduce you to I2C, the sensor backpack, how to interface with them,
// and output the values through UART.
// It is recommended to use 16-bit timers 0a, 0b, 1a, 1b as your timers and output the values through UART.

// c. sample sensor values at a specific interval using timer modules
//      - use timers to sample from opt3001 and accelerometer (or others) at specific rates

/************************************Includes***************************************/

#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>

#include <driverlib/uartstdio.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/interrupt.h>
#include <driverlib/timer.h>

#include "MultimodDrivers/multimod.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/
/*************************************Defines***************************************/

/********************************Public Variables***********************************/

uint8_t read_imu_flag = 0;
uint8_t read_opt_flag = 0;
uint8_t toggle_led_flag = 0;
uint8_t print_uart_flag = 0;

/********************************Public Variables***********************************/

/********************************Init Functions***********************************/

void TIMER0A_Handler(void);
void TIMER0B_Handler(void);
void TIMER1A_Handler(void);
void TIMER1B_Handler(void);

void Timer_Init() {
    // Initialize timers
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1));
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));

    // Disable timers
    TimerDisable(TIMER1_BASE, TIMER_BOTH);
    TimerDisable(TIMER0_BASE, TIMER_BOTH);

    // Configure timers as half-width, periodic 16-bit or (32-bit if using 64-bit timers) timers for a total of 4 timers
    TimerConfigure(TIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC | TIMER_CFG_A_PERIODIC);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PERIODIC | TIMER_CFG_A_PERIODIC);

    // Set prescalers
    TimerPrescaleSet(TIMER1_BASE, TIMER_BOTH, 200);
    TimerPrescaleSet(TIMER0_BASE, TIMER_BOTH, 200);

    // Load initial timer values
        // Sysclock / prescaler * desired seconds = timer period
        //UART: SysclockGet() / TimerPrescalerGet() * 500 * 10^(-3)
    TimerLoadSet(TIMER1_BASE, TIMER_B, (SysCtlClockGet()*500)/(TimerPrescaleGet(TIMER1_BASE, TIMER_B) * 1000));
    TimerLoadSet(TIMER1_BASE, TIMER_A, (SysCtlClockGet()*100)/(TimerPrescaleGet(TIMER1_BASE, TIMER_B) * 1000));
    TimerLoadSet(TIMER0_BASE, TIMER_B, (SysCtlClockGet()*150)/(TimerPrescaleGet(TIMER0_BASE, TIMER_B) * 1000));
    TimerLoadSet(TIMER0_BASE, TIMER_A, (SysCtlClockGet()*100)/(TimerPrescaleGet(TIMER0_BASE, TIMER_B) * 1000));

    // Enable timer interrupts
    TimerIntEnable(TIMER1_BASE, TIMER_TIMB_TIMEOUT | TIMER_TIMA_TIMEOUT);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMB_TIMEOUT | TIMER_TIMA_TIMEOUT);
    // Enable timers
    TimerEnable(TIMER1_BASE, TIMER_BOTH);
    TimerEnable(TIMER0_BASE, TIMER_BOTH);
}

void Int_Init(void) {
    IntMasterDisable();

    // Enable timer interrupt,
    IntEnable(INT_TIMER1B);
    IntEnable(INT_TIMER1A);
    IntEnable(INT_TIMER0B);
    IntEnable(INT_TIMER0A);

    //set interrupt priorities
    IntPrioritySet(INT_TIMER1B | INT_TIMER1A | INT_TIMER0B | INT_TIMER0A, 7);

    // Point to relevant timer handler function
    IntRegister(INT_TIMER1B, TIMER1B_Handler);
    IntRegister(INT_TIMER1A, TIMER1A_Handler);
    IntRegister(INT_TIMER0B, TIMER0B_Handler);
    IntRegister(INT_TIMER0A, TIMER0A_Handler);

    IntMasterEnable();
}

void LED_Init(void) {
    // Enable clock to port F
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    // Set pins [2..0] to output, set as digital
    GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1, 0x00);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1);
}


/********************************Public Functions***********************************/

/************************************MAIN*******************************************/
// Use timers to enforce specific schedules for each event.
int main(void) {
    Int_Init();
    Timer_Init();
    UART_Init();

    LED_Init();
    BMI160_Init();
    OPT3001_Init();

    //int16_t x_accel_value = 0;
    //uint16_t opt_value = 0;
    uint8_t red = GPIO_PIN_1;
    while(1) {
        // Write code to read the x-axis accelerometer value,
        if(read_imu_flag){
            UARTprintf("Accel x Value: %d\n", BMI160_AccelXGetResult());
            read_imu_flag = 0;
        }
        // opt3001 sensor,
        if(read_opt_flag){
            UARTprintf("Optical Value: %d\n", OPT3001_GetResult());
            read_opt_flag = 0;
        }
        //toggle the red led, and print
        if(toggle_led_flag){
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1, red);
            if(red)
                red = 0;
            else
                red = GPIO_PIN_1;
            toggle_led_flag = 0;
        }
        //Print
        if(print_uart_flag){
            UARTprintf("Hello World\n\n");
            print_uart_flag = 0;
        }
    }
}


/************************************MAIN*******************************************/

/********************************Public Functions***********************************/

/*******************************Interrupt Handlers**********************************/

// Timer handlers are provided to you.
void TIMER0A_Handler(void) {
    read_imu_flag = 1;
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    return;
}

void TIMER0B_Handler(void) {
    read_opt_flag = 1;
    TimerIntClear(TIMER0_BASE, TIMER_TIMB_TIMEOUT);
    return;
}

void TIMER1A_Handler(void) {
    toggle_led_flag = 1;
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);
    return;
}

void TIMER1B_Handler(void) {
    print_uart_flag = 1;
    TimerIntClear(TIMER1_BASE, TIMER_TIMB_TIMEOUT);
    return;
}
/*******************************Interrupt Handlers**********************************/
