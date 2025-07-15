// G8RTOS_Threads.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for thread functions.

/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"

/************************************Includes***************************************/

/*************************************Defines***************************************/
// Assumes that the system clock is 16 MHz.
#define delay_0_1_s     (1600000/3)
/*************************************Defines***************************************/

/********************************Public Functions***********************************/

// Thread0, reads accel_x data, adjusts BLUE led duty cycle.
void Thread0(void) {
   int16_t accelReading;
    float duty;
    while(1){
        // Start Crit Section
        G8RTOS_WaitSemaphore(&sem_I2CA);
            // Get Optical Sensor Data
        accelReading = BMI160_AccelXGetResult();
        // End Crit Section
        G8RTOS_SignalSemaphore(&sem_I2CA);
        SysCtlDelay(delay_0_1_s);
        //Adjust Green PWM
        if(accelReading < 0)
            duty = (-1*accelReading)/20000.0;
        else
            duty = (accelReading)/20000.0;
        //Make Duty a ratio

        LaunchpadLED_PWMSetDuty(GPIO_PIN_2, duty);
        //Delay
    }
}

// Thread1, reads gyro_x data, adjusts RED led duty cycle.
void Thread1(void) {
    int16_t gyroReading;
     float duty;
     while(1){
         // Start Crit Section
         G8RTOS_WaitSemaphore(&sem_I2CA);
             // Get Optical Sensor Data
         gyroReading = BMI160_GyroXGetResult();
         // End Crit Section
         G8RTOS_SignalSemaphore(&sem_I2CA);
         SysCtlDelay(delay_0_1_s);
         //Adjust Green PWM
         if(gyroReading < 0)
             duty = (-1*gyroReading)/15000.0;
         else
             duty = (gyroReading)/15000.0;
         //Make Duty a ratio


         LaunchpadLED_PWMSetDuty(GPIO_PIN_1, duty);
         //Delay
     }
}

// Thread2, reads optical sensor values, adjusts GREEN led duty cycle.
void Thread2(void) {
    uint16_t sensorReading;
    float duty;
    while(1){
        // Start Crit Section
        G8RTOS_WaitSemaphore(&sem_I2CA);
            // Get Optical Sensor Data
        sensorReading = OPT3001_GetResult();
        // End Crit Section
        G8RTOS_SignalSemaphore(&sem_I2CA);
        SysCtlDelay(delay_0_1_s);
        //Adjust Green PWM
        duty = (sensorReading)/1000.0;
        //Make Duty a ratio

        LaunchpadLED_PWMSetDuty(GPIO_PIN_3, duty);
        //Delay
    }
}

// Thread3, reads and output button 1 status using polling
void Thread3(void) {
    while(1){
            G8RTOS_WaitSemaphore(&sem_UART);
            if(LaunchpadButtons_ReadSW1())
                UARTprintf("Button 1 pressed!\n");
            G8RTOS_SignalSemaphore(&sem_UART);
            SysCtlDelay(delay_0_1_s);
    }
}

// Thread4, reads and output button 2 status using polling
void Thread4(void) {

}

/********************************Public Functions***********************************/
