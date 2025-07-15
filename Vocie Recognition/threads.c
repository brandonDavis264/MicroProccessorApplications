// G8RTOS_Threads.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for thread functions.

/************************************Includes***************************************/

#include "./threads.h"

#include "./MultimodDrivers/multimod.h"
#include "./MiscFunctions/Signals/inc/goertzel.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "driverlib/timer.h"
#include "driverlib/adc.h"

#define MAX_NUM_SAMPLES      (200)
#define SIGNAL_STEPS         (2)


/*********************************Global Variables**********************************/
uint16_t dac_step = 0;
int16_t dac_signal[SIGNAL_STEPS] = {0x001,0x000};
int16_t current_volume = 0x0;
uint32_t freqData = 0;



/*********************************Global Variables**********************************/

/********************************Public Functions***********************************/

int16_t Goertzel_ReadSample (int FIFO_index) {
    // read sample from FIFO
    int16_t sample = G8RTOS_ReadFIFO(FIFO_index);
    // return sample value
    return sample;
}

/*************************************Threads***************************************/

void Idle_Thread(void) {
    while(1);
}

void Mic_Thread(void) {

    // Input sample rate.
    const double sample_rate_hz = MIC_SAMPLE_RATE_HZ;

    // Frequency of DFT bins to calculate.
    const double detect_hz[2] = {1000.0,2000.0};

    // Number of samples for detection
    const int N = MAX_NUM_SAMPLES;

    while(1)
    {
        // use goertzel function to calculate magnitudes for FREQ_1 and FREQ_2
        // NOTE: make sure you have implemented the FIFO read function in Goertzel_ReadSample
        float magnitude_f1 = (float)(goertzel(detect_hz[0], sample_rate_hz, N, Goertzel_ReadSample, FREQ1_FIFO));
        float magnitude_f2 = (float)(goertzel(detect_hz[1], sample_rate_hz, N, Goertzel_ReadSample, FREQ2_FIFO));

        // calculate magnitudes of FREQ_1 and FREQ_2
        magnitude_f1 = fabs(2.0*Y_MAX*(magnitude_f1/N));
        magnitude_f2 = fabs(2.0*Y_MAX*(magnitude_f2/N));

        // pack magnitude into 32-bit integer
        uint32_t packed_result = (int16_t)(magnitude_f1) << 16 | (int16_t)(magnitude_f2);

        // push newly magnitude ratio to display FIFO
        if (packed_result) G8RTOS_WriteFIFO(DISPLAY_FIFO, packed_result);
    }
}

void Speaker_Thread(void) {

    uint8_t buttons = 0;
    uint32_t timer_period = SysCtlClockGet() / 1000;
    uint16_t freq = DAC_SAMPLE_FREQUENCY_HZ;

    while (1)
    {

        // Get buttons
       buttons = G8RTOS_ReadFIFO(BUTTONS_FIFO);

        // check which buttons are pressed
       if(~buttons &  SW1){
           //UARTprintf("SW1 Pressed \n\n");
           // set DAC output rate to 1000Hz
           freq = 1000;
           timer_period = SysCtlClockGet() / freq;
           TimerDisable(TIMER1_BASE, TIMER_A);
           TimerLoadSet(TIMER1_BASE, TIMER_A, timer_period - 1);
           TimerEnable(TIMER1_BASE, TIMER_A);
       }
       if(~buttons &  SW2){
           //UARTprintf("SW2 Pressed \n\n");
           //set DAC output rate to 2000Hz
           freq = 2000;
           timer_period = SysCtlClockGet() / freq;
           TimerDisable(TIMER1_BASE, TIMER_A);
           TimerLoadSet(TIMER1_BASE, TIMER_A, timer_period - 1);
           TimerEnable(TIMER1_BASE, TIMER_A);
       }
       if(~buttons &  SW3){
           //UARTprintf("SW3 Pressed \n\n");
           // set DAC output rate to 3000Hz
           freq = 3000;
           timer_period = SysCtlClockGet() / freq;
           TimerDisable(TIMER1_BASE, TIMER_A);
           TimerLoadSet(TIMER1_BASE, TIMER_A, timer_period - 1);
           TimerEnable(TIMER1_BASE, TIMER_A);
       }
       if(~buttons &  SW4){
           //UARTprintf("SW4 Pressed \n\n");
           // Disable DAC
           TimerDisable(TIMER1_BASE, TIMER_A);
       }
       sleep(20);
    }
}

void Volume_Thread(void) {

    // define variables
    uint16_t y;
    while(1)
    {
        // read joystick values
        y = G8RTOS_ReadFIFO(JOYSTICK_FIFO);

        if(y < 1800){
            current_volume -= 25;
            if(current_volume < 0)
                current_volume = 0;
        }
        else if (y > 2500){
            current_volume += 25;
            if(current_volume > 500)
                current_volume = 500;
        }
        sleep(20);
        UARTprintf("Current Vol is: %d\n", current_volume);

    }
}

void Display_Thread(void) {

    // Initialize / declare any variables here
    int32_t sample;

    uint16_t magnitude_f1;
    uint16_t magnitude_f2;

    uint16_t prevMagf1 = 0;
    uint16_t prevMagf2 = 0;

    while(1) {

        // read display FIFO for updated magnitude ratio
        sample = G8RTOS_ReadFIFO(DISPLAY_FIFO);

        // unpack result values
        magnitude_f1 = (uint16_t)(sample >> 16);   // Extract the upper 16 bits (bits 31-16)
        magnitude_f2 = (uint16_t)(sample & 0xFFFF); // Extract the lower 16 bits (bits 15-0)

        // draw the magnitudes on the display

            // limit the magnitude values to the display range
            if(magnitude_f1 > Y_MAX)
                magnitude_f1 = Y_MAX;
            if(magnitude_f2 > Y_MAX)
                magnitude_f2 = Y_MAX;

            // Draw
            //G8RTOS_WaitSemaphore(&sem_SPIA);
            if(magnitude_f1 != prevMagf1){
                if(magnitude_f1 < prevMagf1)
                    ST7789_DrawRectangle(80, magnitude_f1, 40,  (prevMagf1 - magnitude_f1), ST7789_BLACK);
                if(magnitude_f1 > prevMagf1)
                    ST7789_DrawRectangle(80, prevMagf1, 40,  (magnitude_f1 - prevMagf1), ST7789_BLUE);
            }

            if(magnitude_f2 != prevMagf2){
                if(magnitude_f2 < prevMagf2)
                    ST7789_DrawRectangle(120, magnitude_f2, 40,  (prevMagf2 - magnitude_f2), ST7789_BLACK);
                if(magnitude_f2 > prevMagf2)
                    ST7789_DrawRectangle(120, prevMagf2, 40,  (magnitude_f2 - prevMagf2), ST7789_RED);
            }

            //G8RTOS_SignalSemaphore(&sem_SPIA);

            // update previous value
           prevMagf1 = magnitude_f1;
           prevMagf2 = magnitude_f2;

           sleep(20);
    }
}

void Read_Buttons(void) {

    // Initialize / declare any variables here
    uint8_t buttons;

    while(1) {

        // Wait for a signal to read the buttons on the Multimod board.
        G8RTOS_WaitSemaphore(&sem_PCA9555_Debounce);

        // debounce buttons
        sleep(20);

        // wait for button semaphore
        //G8RTOS_WaitSemaphore(&sem_I2CA);

        // Get buttons
        buttons = MultimodButtons_Get();
        //G8RTOS_SignalSemaphore(&sem_I2CA);



        // update current_buttons value
        G8RTOS_WriteFIFO(BUTTONS_FIFO, buttons);

        // Clear the interrupt
        GPIOIntClear(GPIO_PORTE_BASE,  BUTTONS_INT_PIN);
        // Re-enable the interrupt so it can occur again.
        GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
    }
}


/********************************Periodic Threads***********************************/

void Update_Volume(void) {

    // Read the joystick
    uint16_t readY = JOYSTICK_GetY();
    // Send through FIFO.
    G8RTOS_WriteFIFO(JOYSTICK_FIFO , readY);
}


/*******************************Aperiodic Threads***********************************/


void Mic_Handler() {

    uint32_t micData[4] = {0};

    // Clear the ADC interrupt
    ADCIntClear(ADC0_BASE, 1);

    // Read ADC Value
    ADCSequenceDataGet(ADC0_BASE, 1, micData);

    // write new sample to audio FIFOs


            G8RTOS_WriteFIFO(FREQ1_FIFO, micData[0]);
            G8RTOS_WriteFIFO(FREQ2_FIFO, micData[0]);

            G8RTOS_WriteFIFO(DAC_FIFO, micData[0]);

   //



}

void Button_Handler() {

    // Disable interrupt
    GPIOIntDisable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
    GPIOIntClear(BUTTONS_INT_GPIO_BASE,  BUTTONS_INT_PIN);
    // Signal relevant semaphore
    G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);
}

void DAC_Timer_Handler() {

    // clear the timer interrupt
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // read next output sample
    uint32_t output = (current_volume)*(dac_signal[dac_step++ % SIGNAL_STEPS]);

    // BONUS: stream microphone input to DAC output via FIFO
 //   freqData = G8RTOS_ReadFIFO(DAC_FIFO);
//    uint16_t f1 = (uint16_t)(sample >> 16);   // Extract the upper 16 bits (bits 31-16)
//    uint16_t f2 = (uint16_t)(sample & 0xFFFF);
//
    // write the output value to the dac
    MutimodDAC_Write(DAC_OUT_REG, output);
}
