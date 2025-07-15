// Lab 4, uP2 Fall 2023
// Created: 2023-07-31
// Updated: 2023-08-01
// Lab 4 is intended to introduce you to more advanced RTOS concepts. In this, you will
// - implement blocking, yielding, sleeping
// - Thread priorities, aperiodic & periodic threads
// - IPC using FIFOs
// - Dynamic thread creation & deletion

//Questions For Office Hours
    // 1. Why can't I call the FIFO Read/Write Functions in the UART Semaphores
        //Or Why do the Does it block those threads from runing for ever? ***************** IMPORTANT OVER OTHER

    // 2. Why do I need to start in the semphore's Crit Section at the begging of Signal
        //At the very end of Wait?

/************************************Includes***************************************/

#include "G8RTOS/G8RTOS.h"
#include "./MultimodDrivers/multimod.h"

#include "./threads.h"

/********************************Public Variables***********************************/
extern uint32_t counter0 = 0;
extern uint32_t counter1 = 0;
extern uint32_t counter2 = 0;

/********************************Public Functions***********************************/

// Complete the functions below as test threads.
void taskIdle(){
    while(1);
}

void task0() {
    while(1)
    {
        G8RTOS_WaitSemaphore(&sem_UART);
        UARTprintf("Task 0 counter is at %d\n", counter0);
        G8RTOS_SignalSemaphore(&sem_UART);
        counter0++;
        sleep(1000);
    }
}

void task1() {
    while(1)
    {
        G8RTOS_WaitSemaphore(&sem_UART);
        UARTprintf("Task 1 counter is at %d\n", counter1);
        G8RTOS_SignalSemaphore(&sem_UART);
        counter1++;
        G8RTOS_KillSelf();
        sleep(1000);
    }
}

void task2() {
    while(1)
    {
        G8RTOS_WaitSemaphore(&sem_UART);
        UARTprintf("Task 2 counter is at %d\n", counter2);
        G8RTOS_SignalSemaphore(&sem_UART);
        counter2++;
        sleep(1000);
    }
}

void taskReadFIFO() {
    while(1)
    {
        int i;
        for(i = 0; i < 16; i ++){
            G8RTOS_WaitSemaphore(&sem_UART);
            UARTprintf("Reading from FIFO expecting i: %d\n", G8RTOS_ReadFIFO(0));
            G8RTOS_SignalSemaphore(&sem_UART);
        }
        sleep(2000);
    }
}

void taskWriteFIFO() {
    while(1)
    {
        int i;
        for(i = 0; i < 32; i++){
            G8RTOS_WaitSemaphore(&sem_UART);
            UARTprintf("Wrote To FIFO Expecting 0: %d\n", G8RTOS_WriteFIFO(0, i));
            G8RTOS_SignalSemaphore(&sem_UART);
        }
        sleep(1000);
    }
}


/********************************Public Functions***********************************/


/************************************MAIN*******************************************/
int main(void)
{
    // Sets clock speed to 80 MHz. You'll need it!
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    G8RTOS_Init();
    //UART_Init();
    multimod_init();

    // Add threads, semaphores, here

    G8RTOS_AddThread( Idle_Thread, 255, "idle\0");
    G8RTOS_AddThread( Read_Buttons, 252, "buttons\0");
    G8RTOS_AddThread( CamMove_Thread, 253, "camera\0");

    G8RTOS_AddThread( Read_JoystickPress, 252, "joystick_s\0");

    G8RTOS_Add_APeriodicEvent(GPIOE_Handler, 5, 20);
    G8RTOS_Add_APeriodicEvent(GPIOD_Handler, 5, INT_GPIOD);

    G8RTOS_Add_PeriodicEvent(Get_Joystick, 50, 1);
    G8RTOS_Add_PeriodicEvent( Print_WorldCoords, 100, 2);


    G8RTOS_InitSemaphore(&sem_UART, 1);
    G8RTOS_InitSemaphore(&sem_PCA9555_Debounce, 0);
    G8RTOS_InitSemaphore(&sem_SPIA, 1);
    G8RTOS_InitSemaphore(&sem_I2CA, 1);
    G8RTOS_InitSemaphore(&sem_Joystick_Debounce, 0);
    G8RTOS_InitSemaphore(&sem_KillCube, 1);

    G8RTOS_InitFIFO(SPAWNCOOR_FIFO);
    G8RTOS_InitFIFO(JOYSTICK_FIFO);
    G8RTOS_Launch();
    while (1);
}

/************************************MAIN*******************************************/
