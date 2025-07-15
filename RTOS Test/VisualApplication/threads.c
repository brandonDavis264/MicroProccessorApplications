// G8RTOS_Threads.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for thread functions.

/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"
#include "./MiscFunctions/Shapes/inc/cube.h"
#include "./MiscFunctions/LinAlg/inc/linalg.h"
#include "./MiscFunctions/LinAlg/inc/quaternions.h"
#include "./MiscFunctions/LinAlg/inc/vect3d.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "inc/hw_nvic.h"

// Change this to change the number of points that make up each line of a cube.
// Note that if you set this too high you will have a stack overflow!
#define Num_Interpolated_Points 10

//sizeof(float) * num_lines * (Num_Interpolated_Points + 2) = ?

#define MAX_NUM_CUBES           (MAX_THREADS - 4)


/*********************************Global Variables**********************************/

Quat_t world_camera_pos = {0, 0, 0, 50};
Quat_t world_camera_frame_offset = {0, 0, 0, 50};
Quat_t world_camera_frame_rot_offset;
Quat_t world_view_rot = {1, 0, 0, 0};
Quat_t world_view_rot_inverse = {1, 0, 0, 0};

// How many cubes?
int num_cubes = 0;

// y-axis controls z or y
uint8_t joystick_y = 1;

// Kill a cube?
uint8_t kill_cube = 0;

/*********************************Global Variables**********************************/

/*************************************Threads***************************************/

void Idle_Thread(void) {
    //time_t t;
    //srand((unsigned) time(&t));
    while(1);
}

void printThread(void){
    UARTprintf("Thread SuccessFully Added");
    while(1);
}

void CamMove_Thread(void) {
    // Initialize / declare any variables here
    int16_t x;
    int16_t y;

    uint32_t fifo_out;
    while(1) {
        // Get result from joystick
        fifo_out = G8RTOS_ReadFIFO(JOYSTICK_FIFO);
        x = (((int16_t) ((fifo_out >> 16) & 0xFFFF)));
        y = (((int16_t) ((fifo_out >> 0) & 0xFFFF)));

       Quat_t cord = {0,0,0,0};

       //Since the deadzone is around
       cord.x = x - 2000;
       if(joystick_y){
           cord.y = (y - 2000) * -1;
       } else {
           cord.z = (y - 2000) * -1;
       }
//       cord.x = x;
//
//       if(joystick_y)
//           cord.y = y;
//       else
//           cord.z = y;

        // If joystick axis within deadzone, set to 0. Otherwise normalize it.
       if(x > 1800 && x < 2500)
           cord.x = 0;
       //else if (x <= 1800)
         //  cord.x *= -1;

       if(y > 1800 && y < 2500){
           cord.y = 0;
           cord.z = 0;
       }//else if (x >= 2500){
          // cord.y *= -1;
           //cord.z *= -1;
       //}
       Quat_Normalize(&cord);
        // Update world camera position. Update y/z coordinates depending on the joystick toggle.

       Quat_t temp_pos;
       getRotatedQuat(&temp_pos, &cord, &world_view_rot); // Rotate the input based on the camera's current rotation
       world_camera_pos.x += temp_pos.x;
       world_camera_pos.y += temp_pos.y;
       world_camera_pos.z += temp_pos.z;
        // sleep
        sleep(100);
    }
}

void Cube_Thread(void) {
    cube_t cube;

    /*************YOUR CODE HERE*************/
    // Get spawn coordinates from FIFO, set cube.x, cube.y, cube.z
    cube.x_pos = G8RTOS_ReadFIFO(SPAWNCOOR_FIFO);
    cube.y_pos = G8RTOS_ReadFIFO(SPAWNCOOR_FIFO);
    cube.z_pos = G8RTOS_ReadFIFO(SPAWNCOOR_FIFO);

    cube.width = 50;
    cube.height = 50;
    cube.length = 50;

    Quat_t v[8];
    Quat_t v_relative[8];

    Cube_Generate(v, &cube);

    uint32_t m = Num_Interpolated_Points + 1;
    Vect3D_t interpolated_points[12][Num_Interpolated_Points + 2];
    Vect3D_t projected_point;

    Quat_t camera_pos;
    Quat_t camera_frame_offset;
    Quat_t view_rot_inverse;

    uint8_t kill = 0;

    while(1) {
        /*************YOUR CODE HERE*************/
        // Check if kill ball flag is set.
        G8RTOS_WaitSemaphore(&sem_KillCube);
        if(kill_cube){
            kill= 1;
            kill_cube = 0;
        }
        G8RTOS_SignalSemaphore(&sem_KillCube);

        camera_pos.x = world_camera_pos.x;
        camera_pos.y = world_camera_pos.y;
        camera_pos.z = world_camera_pos.z;

        camera_frame_offset.x = world_camera_frame_offset.x;
        camera_frame_offset.y = world_camera_frame_offset.y;
        camera_frame_offset.z = world_camera_frame_offset.z;

        view_rot_inverse.w = world_view_rot_inverse.w;
        view_rot_inverse.x = world_view_rot_inverse.x;
        view_rot_inverse.y = world_view_rot_inverse.y;
        view_rot_inverse.z = world_view_rot_inverse.z;

        G8RTOS_WaitSemaphore(&sem_SPIA);
        // Clears cube from screen
        int i;
        for (i = 0; i < 12; i++) {
            int j;
            for (j = 0; j < m+1; j++) {
                getViewOnScreen(&projected_point, &camera_frame_offset, &(interpolated_points[i][j]));
                /*************YOUR CODE HERE*************/
                // Wait on SPI bus


                ST7789_DrawPixel(projected_point.x, projected_point.y, ST7789_BLACK);

                /*************YOUR CODE HERE*************/
                // Signal that SPI bus is available


            }
        }
        G8RTOS_SignalSemaphore(&sem_SPIA);

        /*************YOUR CODE HERE*************/
        // If ball marked for termination, kill the thread.
        if(kill){
            G8RTOS_WaitSemaphore(&sem_UART);
            UARTprintf("DELETING CUBE\n");
            G8RTOS_SignalSemaphore(&sem_UART);
            G8RTOS_KillSelf();
            kill = 0;
        }

        // Calculates view relative to camera position / orientation
        for (i = 0; i < 8; i++) {
            getViewRelative(&(v_relative[i]), &camera_pos, &(v[i]), &view_rot_inverse);
        }

        // Interpolates points between vertices
        interpolatePoints(interpolated_points[0], &v_relative[0], &v_relative[1], m);
        interpolatePoints(interpolated_points[1], &v_relative[1], &v_relative[2], m);
        interpolatePoints(interpolated_points[2], &v_relative[2], &v_relative[3], m);
        interpolatePoints(interpolated_points[3], &v_relative[3], &v_relative[0], m);
        interpolatePoints(interpolated_points[4], &v_relative[0], &v_relative[4], m);
        interpolatePoints(interpolated_points[5], &v_relative[1], &v_relative[5], m);
        interpolatePoints(interpolated_points[6], &v_relative[2], &v_relative[6], m);
        interpolatePoints(interpolated_points[7], &v_relative[3], &v_relative[7], m);
        interpolatePoints(interpolated_points[8], &v_relative[4], &v_relative[5], m);
        interpolatePoints(interpolated_points[9], &v_relative[5], &v_relative[6], m);
        interpolatePoints(interpolated_points[10], &v_relative[6], &v_relative[7], m);
        interpolatePoints(interpolated_points[11], &v_relative[7], &v_relative[4], m);

        G8RTOS_WaitSemaphore(&sem_SPIA);
        for (i = 0; i < 12; i++) {
            int j;
            for (j = 0; j < m+1; j++) {
                getViewOnScreen(&projected_point, &camera_frame_offset, &(interpolated_points[i][j]));

                if (interpolated_points[i][j].z < 0) {
                    /*************YOUR CODE HERE*************/
                    // Wait on SPI bus

                    ST7789_DrawPixel(projected_point.x, projected_point.y, ST7789_BLUE);

                    /*************YOUR CODE HERE*************/
                    // Signal that SPI bus is available

                }
            }
        }

        G8RTOS_SignalSemaphore(&sem_SPIA);

        /*************YOUR CODE HERE*************/
        // Sleep
        sleep(100);

    }
}

void Read_Buttons() {
    // Initialize / declare any variables here
    uint8_t btnValues;
    while(1) {
        // Wait for a signal to read the buttons on the Multimod board.
        G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);
        // Sleep to debounce
        sleep(30);
        // Read the buttons status on the Multimod board.
        G8RTOS_WaitSemaphore(&sem_I2CA);
        btnValues = MultimodButtons_Get();
        G8RTOS_SignalSemaphore(&sem_I2CA);
        // Process the buttons and determine what actions need to be performed.
        //G8RTOS_WaitSemaphore(&sem_UART);
        if(~btnValues &  SW1){

            int16_t x = (rand() % 201) - 100;
            int16_t y = (rand() % 201) - 100;
            int16_t z = (rand() % 101) - 120;

            G8RTOS_WriteFIFO(SPAWNCOOR_FIFO, x);
            G8RTOS_WriteFIFO(SPAWNCOOR_FIFO, y);
            G8RTOS_WriteFIFO(SPAWNCOOR_FIFO, z);



            if(num_cubes < MAX_NUM_CUBES){
                G8RTOS_AddThread(Cube_Thread, 250, "CubeAdder");
                UARTprintf("CUBE SPWAND\n\n");
                //G8RTOS_AddThread(printThread, 254, "CubeAdder");
                num_cubes++;
            }
        }
        if(~btnValues &  SW2){
            //UARTprintf("SW2 Pressed \n\n");
            if(num_cubes > 0){
                kill_cube = 1;
                num_cubes--;
            }
        }
        //G8RTOS_SignalSemaphore(&sem_UART);


        //while((~MultimodButtons_Get() & (SW1 | SW2 | SW3 | SW4)));
        // Clear the interrupt
         GPIOIntClear(GPIO_PORTE_BASE,  BUTTONS_INT_PIN);
        // Re-enable the interrupt so it can occur again.
        GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);

//        IntEnable(INT_GPIOE);
    }
}

void Read_JoystickPress() {
    // Initialize / declare any variables here
    uint8_t joystickRead;
    while(1) {
        // Wait for a signal to read the joystick press
        G8RTOS_WaitSemaphore(&sem_Joystick_Debounce);
        // Sleep to debounce
        sleep(30);
        // Read the joystick switch status on the Multimod board.
        joystickRead = JOYSTICK_GetPress();
        // Toggle the joystick_y flag.
        if(joystick_y)
            joystick_y = 0;
        else
            joystick_y = 1;
        // Clear the interrupt
        GPIOIntClear(GPIO_PORTD_BASE,  JOYSTICK_INT_PIN);
        // Re-enable the interrupt so it can occur again.
        GPIOIntEnable(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
    }
}



/********************************Periodic Threads***********************************/

void Print_WorldCoords(void) {
    // Print the camera position through UART to display on console.
    G8RTOS_WaitSemaphore(&sem_UART);
        UARTprintf("Camera Position\t x: %d y: %d z: %d\n",  (int16_t)world_camera_pos.x,  (int16_t)world_camera_pos.y,  (int16_t)world_camera_pos.z);
    G8RTOS_SignalSemaphore(&sem_UART);
}

void Get_Joystick(void) {
    // Read the joystick
    uint32_t readXY = JOYSTICK_GetXY();
    // Send through FIFO.
    G8RTOS_WriteFIFO(JOYSTICK_FIFO , readXY);
}



/*******************************Aperiodic Threads***********************************/

void GPIOE_Handler() {
    // Disable interrupt
    GPIOIntDisable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
    GPIOIntClear(BUTTONS_INT_GPIO_BASE,  BUTTONS_INT_PIN);
    // Signal relevant semaphore
    G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);
}

void GPIOD_Handler() {
    // Disable interrupt
    GPIOIntDisable(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
    GPIOIntClear(JOYSTICK_INT_GPIO_BASE, JOYSTICK_INT_PIN);
    // Signal relevant semaphore
    G8RTOS_SignalSemaphore(&sem_Joystick_Debounce);
}
