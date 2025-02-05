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
      ST7789_Fill(ST7789_BLACK);

      //Background Threads
      G8RTOS_AddThread( Idle_Thread, 255, "idle\0");
      G8RTOS_AddThread( Read_Buttons, 2, "buttons\0");
      G8RTOS_AddThread( Start, 1, "Start\0");

      //APeriodic Threads
      G8RTOS_Add_APeriodicEvent(GPIOE_Handler, 5, 20);
      G8RTOS_Add_APeriodicEvent(DAC_Timer_Handler, 4, DAC_INTERRUPT);

      //Periodic Threads
      G8RTOS_Add_PeriodicEvent(Get_JoystickX, 50, 1);
      G8RTOS_Add_PeriodicEvent(Get_AccelX, 50, 1);

      //Semaphores
      G8RTOS_InitSemaphore(&sem_PCA9555_Debounce, 0);
      G8RTOS_InitSemaphore(&sem_SPIA, 1);
      G8RTOS_InitSemaphore(&sem_I2CA, 1);
      G8RTOS_InitSemaphore(&sem_StartGame, 0);
      G8RTOS_InitSemaphore(&sem_LostGame, 0);

        //FIFOs
        G8RTOS_InitFIFO(JOYSTICK_FIFO);
        G8RTOS_InitFIFO(Accel_FIFO);
    G8RTOS_Launch();
    while (1);
}

/************************************MAIN*******************************************/
