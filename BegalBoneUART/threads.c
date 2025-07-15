// G8RTOS_Threads.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for thread functions.

/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/************************************Includes***************************************/

/*************************************Defines***************************************/


/*************************************Defines***************************************/

/*********************************Global Variables**********************************/

// Global data structure for keeping track of data

/*********************************Global Variables**********************************/

/*************************************Threads***************************************/

void Idle_Thread(void) {
    while(1);
}

void DrawBox_Thread(void) {
    SysCtlDelay(1);


    // Declare variables
    uint8_t Data[7];

    uint16_t color;
    uint16_t y;
    uint16_t h;
    uint16_t x;
    uint16_t w;
    while(1) {
        // Wait for data
        G8RTOS_WaitSemaphore(&sem_Data);
        // Read in data
        for(int i = 0; i < 7; i++){
            Data[i] = G8RTOS_ReadFIFO(DATA_FIFO);
            UARTprintf("Data @ %d is:\t %d\n", i, Data[i]);
        }

        //Process Data Color
        if(Data[6] == 1)
            color = ST7789_RED;
        else if(Data[6] == 2)
            color = ST7789_BLUE;
        else
            color = ST7789_GREEN;
        x = Data[0];
        w = Data[3];
        y = (Data[1] << 8) | Data[2];
        h = (Data[4] << 8) | Data[5];

        // Draw rectangle
       ST7789_DrawRectangle(x, y, w, h, color);

    }
}

/********************************Periodic Threads***********************************/


/********************************Periodic Threads***********************************/


/*******************************Aperiodic Threads***********************************/

void UART4_Handler() {

    // Prepare to read data
    uint8_t ui8DataRx;

    // Get interrupt status
    uint32_t intStatus = UARTIntStatus(UART4_BASE, true);

    // Continue reading if there is still data
        // Store current data value
    while(UARTCharsAvail(UART4_BASE)){
        ui8DataRx = UARTCharGetNonBlocking(UART4_BASE);
        G8RTOS_WriteFIFO(DATA_FIFO, ui8DataRx);
        UARTprintf("Character Received: %d\n", ui8DataRx);
    }

    // Signal data ready
    G8RTOS_SignalSemaphore(&sem_Data);

    // Clear the asserted interrupts
    UARTIntClear (UART4_BASE, intStatus);

}


/*******************************Aperiodic Threads***********************************/
