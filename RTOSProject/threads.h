// threads.h
// Date Created: 2023-07-26
// Date Updated: 2023-07-26
// Threads

#ifndef THREADS_H_
#define THREADS_H_

/************************************Includes***************************************/

#include "./G8RTOS/G8RTOS.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/
#define Accel_FIFO          3
#define JOYSTICK_FIFO       1


/*************************************Defines***************************************/

/***********************************Semaphores**************************************/

semaphore_t sem_I2CA;
semaphore_t sem_SPIA;
semaphore_t sem_PCA9555_Debounce;
semaphore_t sem_Joystick_Debounce;
semaphore_t sem_StartGame;
semaphore_t sem_LostGame;
semaphore_t sem_UART;
semaphore_t sem_accel;
semaphore_t sem_joy;

/***********************************Semaphores**************************************/

/***********************************Structures**************************************/
/***********************************Structures**************************************/


/*******************************Background Threads**********************************/

void Idle_Thread(void);
void Ship_Thread(void);
void Move_Thread(void);
void Read_Buttons(void);
void DisplayScore(void);
void enemy(void);
void Restart(void);
void Lose(void);
void obs_spawn(void);
void Decay_Vol(void);

void Start(void);
//
///*******************************Background Threads**********************************/
//
///********************************Periodic Threads***********************************/
//
//void Print_WorldCoords(void);
void Get_JoystickX(void);
void Get_AccelX(void);
void Bullet_Thread(void);

//
///********************************Periodic Threads***********************************/
//
///*******************************Aperiodic Threads***********************************/
//
void GPIOE_Handler(void);
void DAC_Timer_Handler(void);

/*******************************Aperiodic Threads***********************************/


#endif /* THREADS_H_ */

