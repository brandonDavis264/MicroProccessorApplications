// G8RTOS_IPC.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for FIFO functions for interprocess communication

//Feedback Create a read write Semaphore instead of currentSize

#include "../G8RTOS_IPC.h"

/************************************Includes***************************************/

#include "../G8RTOS_Semaphores.h"

/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/

typedef struct G8RTOS_FIFO_t {
    int32_t buffer[FIFO_SIZE];
    int32_t *head;
    int32_t *tail;
    uint32_t lostData;
    semaphore_t currentSize;
    semaphore_t mutex;
} G8RTOS_FIFO_t;


/***********************************Externs*****************************************/

/********************************Private Variables***********************************/

static G8RTOS_FIFO_t FIFOs[MAX_NUMBER_OF_FIFOS];


/********************************Public Functions***********************************/


// G8RTOS_InitFIFO
// Initializes FIFO, points head & taiL to relevant buffer
// memory addresses. Returns - 1 if FIFO full, 0 if no error
// Param uint32_t "FIFO_index": Index of FIFO block
// Return: int32_t
int32_t G8RTOS_InitFIFO(uint32_t FIFO_index) {
    // Check if FIFO index is out of bounds
    if(FIFO_index >= MAX_NUMBER_OF_FIFOS)
        return -1;
    // Init head, tail pointers
    FIFOs[FIFO_index].head = &FIFOs[FIFO_index].buffer[0];
    FIFOs[FIFO_index].tail = &FIFOs[FIFO_index].buffer[0];
    // Init the mutex, current size
    G8RTOS_InitSemaphore(&FIFOs[FIFO_index].mutex, 1);
    G8RTOS_InitSemaphore(&FIFOs[FIFO_index].currentSize, 0);
    // Init lost data
    FIFOs[FIFO_index].lostData = 0;
    // Finished
    return 0;

}

// G8RTOS_ReadFIFO
// Reads data from head pointer of FIFO.
// Param uint32_t "FIFO_index": Index of FIFO block
// Return: int32_t
int32_t G8RTOS_ReadFIFO(uint32_t FIFO_index) {
    // Wait before reading FIFO
        // In case FIFO is empty
    G8RTOS_WaitSemaphore(&FIFOs[FIFO_index].currentSize);

    // Wait before reading FIFO
        // In case FIFO is being read from another thread
    G8RTOS_WaitSemaphore(&FIFOs[FIFO_index].mutex);

    // Read the data
    int32_t* data = FIFOs[FIFO_index].head;

    // Update the head pointer
    if (FIFOs[FIFO_index].head == &FIFOs[FIFO_index].buffer[FIFO_SIZE - 1])
        //Wrap Around if the FIFO is past the buffer length
        FIFOs[FIFO_index].head = &FIFOs[FIFO_index].buffer[0];
    else
        FIFOs[FIFO_index].head++;

    // Signal Mutex Semaphore so other waiting
        // threads can Read
    G8RTOS_SignalSemaphore(&FIFOs[FIFO_index].mutex);

    //Return the data
    return *data;

    // Be mindful of boundary conditions!
}

// G8RTOS_WriteFIFO
// Writes data to tail of buffer.
// 0 if no error, -1 if out of bounds, -2 if full
// Param uint32_t "FIFO_index": Index of FIFO block
// Param uint32_t "data": data to be written
// Return: int32_t
int32_t G8RTOS_WriteFIFO(uint32_t FIFO_index, uint32_t data) {
    // Wait before reading FIFO
        // In case FIFO is being Written from another thread
    G8RTOS_WaitSemaphore(&FIFOs[FIFO_index].mutex);

    if(FIFO_index >= MAX_NUMBER_OF_FIFOS)
        return -1;
    //If FIFO is Full of Data
    if(FIFOs[FIFO_index].currentSize > FIFO_SIZE-1){
        //Overwrite Some Data
        *(FIFOs[FIFO_index].tail) = data;
        //Increment that data has been lost
        FIFOs[FIFO_index].lostData++;

        //Data Loss Process
        //Shift the head and tail of the data once
        if (FIFOs[FIFO_index].head == &FIFOs[FIFO_index].buffer[FIFO_SIZE - 1])
            //Wrap Around if the FIFO is past the buffer length
            FIFOs[FIFO_index].head = &FIFOs[FIFO_index].buffer[0];
        else
            FIFOs[FIFO_index].head++;
        if (FIFOs[FIFO_index].tail == &FIFOs[FIFO_index].buffer[FIFO_SIZE - 1])
            //Wrap Around if the FIFO is past the buffer length
            FIFOs[FIFO_index].tail = &FIFOs[FIFO_index].buffer[0];
        else
            FIFOs[FIFO_index].tail++;

        // Signal Mutex Semaphore so other waiting
            // threads can Write
        G8RTOS_SignalSemaphore(&FIFOs[FIFO_index].mutex);

        return -2;
    // Else Write Data to Buffer
    }else{
        //Add one to the current size of the FIFO
        G8RTOS_SignalSemaphore(&FIFOs[FIFO_index].currentSize);

        //Write Data
        *(FIFOs[FIFO_index].tail) = data;

        if (FIFOs[FIFO_index].tail == &FIFOs[FIFO_index].buffer[FIFO_SIZE - 1])
            //Wrap Around if the FIFO is past the buffer length
            FIFOs[FIFO_index].tail = &FIFOs[FIFO_index].buffer[0];
        else
            FIFOs[FIFO_index].tail++;

        // Signal Mutex Semaphore so other waiting
            // threads can Write
        G8RTOS_SignalSemaphore(&FIFOs[FIFO_index].mutex);
        return 0;
    }
}

