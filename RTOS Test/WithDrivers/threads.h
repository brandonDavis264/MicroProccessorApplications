// threads.h
// Date Created: 2023-07-26
// Date Updated: 2023-07-26
// Threads

#ifndef THREADS_H_
#define THREADS_H_

/************************************Includes***************************************/

#include "./G8RTOS/G8RTOS_Semaphores.h"

/************************************Includes***************************************/

/***********************************Semaphores**************************************/

semaphore_t sem_UART;
semaphore_t sem_I2CA;

/***********************************Semaphores**************************************/


/********************************Thread Functions***********************************/

void Thread0(void);
void Thread1(void);
void Thread2(void);
void Thread3(void);
void Thread4(void);

/********************************Thread Functions***********************************/


#endif /* THREADS_H_ */

