// Lab 3, uP2 Fall 2023
// Created: 2023-07-31
// Updated: 2023-08-01
// Lab 3 is intended to introduce you to RTOS concepts. In this, you will
// - configure the systick function
// - write asm functions for context switching
// - write semaphore functions
// - write scheduler functions to add threads / run scheduling algorithms
// - write critical section assembly functions

/************************************Includes***************************************/

#include "G8RTOS/G8RTOS.h"
#include "./MultimodDrivers/multimod.h"

#include "./threads.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/
/*************************************Defines***************************************/

/********************************Public Variables***********************************/
extern uint32_t counter0 = 0;
extern uint32_t counter1 = 0;
extern uint32_t counter2 = 0;


/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

// Complete the functions below as test threads.
void task0() {
    while(1)
    {
        counter0++;
        G8RTOS_WaitSemaphore(&sem_UART);
        UARTprintf("Task 0 counter is at %d\n", counter0);
        G8RTOS_SignalSemaphore(&sem_UART);
        SysCtlDelay(1600000);
    }
}

void task1() {
    while(1)
    {
        counter1++;
        G8RTOS_WaitSemaphore(&sem_UART);
        UARTprintf("Task 1 counter is at %d\n", counter1);
        G8RTOS_SignalSemaphore(&sem_UART);
        SysCtlDelay(1600000);
    }
}

void task2() {
    while(1)
    {
        counter2++;
        G8RTOS_WaitSemaphore(&sem_UART);
        UARTprintf("Task 2 counter is at %d\n", counter2);
        G8RTOS_SignalSemaphore(&sem_UART);
        SysCtlDelay(1600000);
    }
}

/********************************Public Functions***********************************/

/************************************MAIN*******************************************/

// Be sure to add in your source files from previous labs into "MultimodDrivers/src/"!
// If you made any modifications to the corresponding header files, be sure to update
// those, too.
int main(void)
{
    Multimod_Init();
    G8RTOS_Init();

    G8RTOS_AddThread(&Thread0);
    G8RTOS_AddThread(&Thread1);
    G8RTOS_AddThread(&Thread2);
    G8RTOS_AddThread(&Thread3);

    //G8RTOS_AddThread(&task0);
    //G8RTOS_AddThread(&task1);
    //G8RTOS_AddThread(&task2);

    G8RTOS_InitSemaphore(&sem_I2CA, 1);
    G8RTOS_InitSemaphore(&sem_UART, 1);
    // Add threads, initialize semaphores here!
    G8RTOS_Launch();
    while (1){
    }
}

/************************************MAIN*******************************************/
