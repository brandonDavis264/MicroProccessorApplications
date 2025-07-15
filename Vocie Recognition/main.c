// Lab 5, uP2 Fall 2024
// Created: 2023-07-31
// Updated: 2024-08-01
// Lab 5 is intended to introduce you to basic signal processing. In this, you will
// - continuously sample audio data from a microphone
// - process the audio data stream using the Goertzel algorithm
// - output audio data to headphones via a DAC
// - provide user feedback via the display

/************************************Includes***************************************/

#include "G8RTOS/G8RTOS.h"
#include "./MultimodDrivers/multimod.h"

#include "./threads.h"
#include "driverlib/interrupt.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/
/*************************************Defines***************************************/

/********************************Public Variables***********************************/
/********************************Public Variables***********************************/


/************************************MAIN*******************************************/

int main(void)
{
    // Sets clock speed to 80 MHz. You'll need it!
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    IntMasterDisable();
    multimod_init();

    // you might want a delay here (~10 ms) to make sure the display has powered up
    SysCtlDelay(8000000/1000);

    // initialize the G8RTOS framework
    G8RTOS_Init();

    // Add semaphores, threads, FIFOs here
    G8RTOS_InitSemaphore(&sem_PCA9555_Debounce, 0);
    G8RTOS_InitSemaphore(&sem_SPIA, 1);
    G8RTOS_InitSemaphore(&sem_I2CA, 1);
    G8RTOS_InitSemaphore(&sem_UART, 1);

    G8RTOS_InitFIFO(JOYSTICK_FIFO);
    G8RTOS_InitFIFO(BUTTONS_FIFO);
    G8RTOS_InitFIFO(FREQ1_FIFO);
    G8RTOS_InitFIFO(FREQ2_FIFO);
    G8RTOS_InitFIFO(DISPLAY_FIFO);

    G8RTOS_AddThread( Idle_Thread, 255, "idle\0");
    G8RTOS_AddThread( Mic_Thread, 2, "Mic\0");
    G8RTOS_AddThread( Display_Thread, 5, "Display\0");
    G8RTOS_AddThread( Read_Buttons, 1, "Buttons\0");
    G8RTOS_AddThread( Speaker_Thread, 250, "Speaker\0");
    G8RTOS_AddThread( Volume_Thread, 5, "Volume\0");

    // add periodic and aperiodic events here (check multimod_mic.h and multimod_buttons.h for defines)
    G8RTOS_Add_PeriodicEvent(Update_Volume, 50, 1);

    G8RTOS_Add_APeriodicEvent(Mic_Handler, 1, MIC_INTERRUPT);
    G8RTOS_Add_APeriodicEvent(Button_Handler, 2, BUTTON_INTERRUPT);
    G8RTOS_Add_APeriodicEvent(DAC_Timer_Handler, 4, DAC_INTERRUPT);

    G8RTOS_Launch();
    while (1);
}

/************************************MAIN*******************************************/
