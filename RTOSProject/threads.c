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
#include "GFX_Library.h"
#include "driverlib/timer.h"


#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#include "inc/hw_nvic.h"

// Change this to change the number of points that make up each line of a cube.
// Note that if you set this too high you will have a stack overflow!
#define Num_Interpolated_Points 10

//sizeof(float) * num_lines * (Num_Interpolated_Points + 2) = ?


#define MAX_NUM_OBS           (255)
#define MAX_NUM_SAMPLES      (200)
#define SIGNAL_STEPS         (2)
/******************************** AUDIO *********************************************/
uint16_t dac_step = 0;
int16_t dac_signal[SIGNAL_STEPS] = {0x001,0x000};
int16_t current_volume = 0;
uint32_t freqData = 0;

/******************************** LED MATRIX *****************************************/
uint8_t led_matrix[20] = {
    0x13, 0x0F, 0x0B, 0x07, 0x03,
    0x12, 0x0E, 0x0A, 0x06,
    0x15, 0x11, 0x0D, 0x09, 0x05,
    0x14, 0x10, 0x0C, 0x08, 0x04, 0x02
};



/*********************************BIT MAPS******************************************/

uint16_t playerSp[10] = {
    0b0001100000, // Row 1
    0b0011110000, // Row 2
    0b0111111000, // Row 3
    0b1111111100, // Row 4
    0b1001100100, // Row 5
    0b1001100100, // Row 6
    0b0111111000, // Row 7
    0b0011110000, // Row 8
    0b0001100000, // Row 9
    0b0000000000  // Row 10
};

uint16_t mushroomSp[10] = {
    0b0000000000, // Row 1
    0b0001111000, // Row 2
    0b0011111100, // Row 3
    0b0011111100, // Row 4
    0b0111111110, // Row 5 (cap of the mushroom)
    0b0111111110, // Row 6 (stem starts)
    0b1111111111, // Row 7
    0b0000110000, // Row 8
    0b0000110000, // Row 9
    0b0000110000  // Row 10 (bottom of the stem)
};

uint16_t centipedeSp[10] = {
    0b0000000000, // Row 1: Top corners for legs
    0b1100000011, // Row 2: Top of the circle
    0b0111001110, // Row 3: Upper circle
    0b0111111110, // Row 4: Middle of the circle
    0b1111111111, // Row 5: Middle of the circle
    0b1111111111, // Row 6: Middle of the circle
    0b0111111110, // Row 7: Bottom of the circle
    0b0111001110, // Row 8: Lower circle
    0b1100000011, // Row 9: Bottom of the circle
    0b0000000000  // Row 10: Bottom corners for legs
};


/*********************************Global Variables**********************************/
//Struct for a Centipede
typedef struct part_t{
    int16_t x;
    int16_t y;
    int16_t h;
    int16_t w;

    bool shot;
}part_t;
typedef struct cent_t{
    // How many links hard code it for now 5
    uint8_t links;
    // Array of max centipede Parts
    part_t centipede_parts[100];
    // Direction (Won't be used till after Made)
    uint8_t partCount;
}cent_t;
uint8_t speed = 2;

//Struct for a obstacle
typedef struct obs_t{
    int16_t x;
    int16_t y;
    int16_t h;
    int16_t w;

    bool shot;
    bool hasDrawn;
}obs_t;
uint8_t linkCount = 5;
obs_t obs[MAX_NUM_OBS];
uint8_t numObs = 0;
cent_t centipede;

//Restart Game
bool restart = false;


// y-axis controls z or y
uint8_t joystick_y = 1;

// Kill a gameObj?
uint8_t kill_gameObj = 0;
uint8_t kill_obs = 0;

//Scoring
uint32_t score = 0;

//Enemy
uint16_t x_global_en = 0;
uint16_t y_global_en = 240;
uint16_t c_length = 5;

//Player
uint16_t x_global = 110;
bool isJoyStick = false;
Quat_t cord = {0,0,0,0};


//Persistent States
uint8_t lost = 0;
uint8_t lose_State = 0;
uint8_t start_State = 1;
uint8_t win = 0;


/*********************************Functions*****************************************/
/* calc_value:
 * Description: Converts a raw acceleration to floatingpoint number
 * Parameters:
 * 1. Raw acceleration Value
 * 2. final value address
 * 3. boolean if it is acceleration tracking
 * */
void calc_value(int16_t raw_value, float *final_value, bool isAccel) {
    // Convert with respect to the value being temperature or acceleration reading
    float scaling;
    float senstivity = 0.004f; // g per unit

    if (isAccel == true) {
        scaling = 64 / senstivity;
    } else {
        scaling = 64;
    }

    // raw_value is signed
    *final_value = (float) ((int16_t) raw_value) / scaling;
}

/* check_collision:
 * Description: Checks and returns if something collides with an obstacle
 * Parameters:
 * 1. x position
 * 2. y position
 * 3. width of object
 * 4. height of object
 * Return: True or false weather something colliedes or not
 * */
bool check_collision(int x, int y, int w, int h) {
    for (int i = 0; i < numObs; i++) {
        if ((x < obs[i].x + obs[i].w) && (x + w > obs[i].x) &&
            (y < obs[i].y + obs[i].h) && (y + h > obs[i].y)) {
            return true;
        }
    }
    return false;
}

/*************************************Background Threads***************************************/

/* Idle Thread
 * Description: Does Nothing
 *  So RTOS does not crash
 * */
void Idle_Thread(void) {
    //time_t t;
    //srand((unsigned) time(&t));
    while(1);
}

/* Display Score
 * Description: Displays the score of the game on
 *          the right corner of the screen
 *          using the GFX library.
 * */
void DisplayScore(void){
    int prev_score;
    while(1){

        char score_str[5]; // 5 digits + null terminator
        // Convert the score to a string
        sprintf(score_str, "%d", score);

        //Score:
        display_drawChar(10, 265, 'S', 0xffff, 0xffff, 2);
        display_drawChar(21, 265, 'c', 0xffff, 0xffff, 2);
        display_drawChar(32, 265, 'o', 0xffff, 0xffff, 2);
        display_drawChar(43, 265, 'r', 0xffff, 0xffff, 2);
        display_drawChar(54, 265, 'e', 0xffff, 0xffff, 2);
        display_drawChar(65, 265, ':', 0xffff, 0xffff, 2);

        //DIGIT NUMBER
        // Display the score digits
        int x = 76;
        // Starting position for the score digits
        for (int i = 0; score_str[i] != '\0'; i++) {
            display_drawChar(x, 265, score_str[i], 0xffff, 0xffff, 2);
            x += 12; // Move to the next position }
        }
        x = 76;
        // Starting position for the score digits
        if(prev_score != score)
            for (int i = 0; score_str[i] != '\0'; i++) {
                display_drawChar(x, 265, score_str[i], 0x0000, 0x0000, 2);
                x += 12; // Move to the next position }
            }

        prev_score = score;
    }
}

/* Ship Thread
 * Description: Displays the ship that is moved on the x-axsis by bounds of the screen
 *          either by:
 *          1. Acceleration in the X direction
 *          2. The Joy Stick
 * */
void Ship_Thread(void) {
    int16_t x;
    uint32_t fifo_out;
    uint16_t prev_x_global = x_global; // Track the previous x position

    float accel_x;

    while (1) {
        if(isJoyStick){
            fifo_out = G8RTOS_ReadFIFO(JOYSTICK_FIFO);
            x = (((int16_t)((fifo_out >> 16) & 0xFFFF)));

            cord.x = x - 2000;

            // Deadzone adjustment
            if (x > 1800 && x < 2500)
                cord.x = 0;
        }else{
            x = G8RTOS_ReadFIFO(Accel_FIFO);
            calc_value(x, &accel_x, true);
            cord.x = accel_x;
            // Deadzone adjustment
            if (accel_x > - 0.1 && accel_x  < 0.1)
                cord.x = 0;
            if(x_global <= 0)
                x_global += 5;
            if(x_global >= 210)
                x_global -= 5;
        }

        Quat_Normalize(&cord);
        G8RTOS_WaitSemaphore(&sem_SPIA);

        // Clear the previous bitmap
        uint8_t y_clear = 65;
        for (int i = 0; i < 8; i++) { // Loop over 8 rows
            uint16_t x_clear = prev_x_global; // Clear from previous position
            for (int j = 9; j >= 0; j--) { // Clear 10 bits wide
                ST7789_DrawPixel(x_clear, y_clear, ST7789_BLACK);
                x_clear++;
            }
            y_clear++;
        }

        // Update x position
        if (cord.x < 0 && x_global < 210) { // Move left
            if(isJoyStick)
                x_global += 5;
            else
                x_global -= 5;
        } else if (cord.x > 0 && x_global > 0) { // Move right
            if(isJoyStick)
                x_global -= 5;
            else
                x_global += 5;
        }

        prev_x_global = x_global;

        // Draw the bitmap at the new position
        uint8_t y_global = 65;
        for (int i = 0; i < 8; i++) { // Loop over 8 rows
            uint16_t value = playerSp[i];
            uint16_t x = x_global; // Temporary variable for horizontal placement
            for (int j = 9; j >= 0; j--) { // Iterate through 10 bits
                if ((value >> j) & 1) {
                    ST7789_DrawPixel(x, y_global, 0xffff);
                } else if ((i == 4 || i == 5) && j > 1) {
                    ST7789_DrawPixel(x, y_global, ST7789_RED);
                }else {
                    ST7789_DrawPixel(x, y_global, ST7789_BLACK);
                }
                x++; // Increment temporary x position
            }
            y_global++; // Move to the next row
        }

        G8RTOS_SignalSemaphore(&sem_SPIA);

        // Kill the thread if lost
        if (lost)
            G8RTOS_KillSelf();

        sleep(1);
    }
}


/* Read Buttons
 * Description: Reads buttons from the GPIOE_handler execution
 *          SW1. Fire a Bullet
 *                 - Plays a Sound Affect when shooting a bullet
 *          SW2. Setting to move the Character via joy-stick
 *          SW3. Setting to move the Character via Acceleration
 * */
void Read_Buttons() {
    // Initialize / declare any variables here
    uint8_t btnValues;
    uint32_t timer_period = SysCtlClockGet() / 3000;
    uint16_t freq = DAC_SAMPLE_FREQUENCY_HZ;
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
        if((~btnValues &  SW1) && !lost && !start_State){
                current_volume = 200;
                G8RTOS_AddThread(Bullet_Thread, 1, "Bullet");
                freq = 3000;
                timer_period = SysCtlClockGet() / freq;
                TimerDisable(TIMER1_BASE, TIMER_A);
                TimerLoadSet(TIMER1_BASE, TIMER_A, timer_period - 1);
                TimerEnable(TIMER1_BASE, TIMER_A);
                G8RTOS_AddThread(Decay_Vol, 1, "Bt");
        }

        if((~btnValues &  SW2) && start_State){
            // Signal to Start
            G8RTOS_SignalSemaphore(&sem_StartGame);
            isJoyStick = true;
            start_State = 0;
        }
        if((~btnValues &  SW3) && start_State){
            // Signal to Start
            G8RTOS_SignalSemaphore(&sem_StartGame);
            isJoyStick = false;
            start_State = 0;
        }

        // Clear the interrupt
        GPIOIntClear(GPIO_PORTE_BASE,  BUTTONS_INT_PIN);
        // Re-enable the interrupt so it can occur again.
        GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);

    }
}

/* Decay Volume
 * Description: Decay's Volume from a max volume to 0
 *      Killed once volume reached 0
 * */
void Decay_Vol(void){
    while(1){
        while(current_volume > 0){
            current_volume -= 10;
            sleep(10);
        }
        G8RTOS_KillSelf();
    }
}

/* Bullet Thread
 * Description: Handles Bullet Behavior
 *      - Collision to objects
 *          - Set up spawning an obstacle from collide with centipede
 *          - Remove an obstacle if it is shot
 *      - When to Kill the thread
 * */
void Bullet_Thread(void) {

    uint16_t y = 76;
    uint16_t y_old = y;
    uint16_t x_bullet = x_global;
    while(1){

        G8RTOS_WaitSemaphore(&sem_SPIA);

        if(y > 240 || win || lost){
            ST7789_DrawRectangle(x_bullet, y_old, 10, 10, ST7789_BLACK);
            G8RTOS_SignalSemaphore(&sem_SPIA);
            G8RTOS_KillSelf();
        }else{
            y += 1;
            ST7789_DrawRectangle(x_bullet, y, 5, 5, ST7789_RED);
            ST7789_DrawRectangle(x_bullet, y_old, 5, 5, ST7789_BLACK);
        }
        //Collides with a centipede part
        for(int i = 0; i < centipede.links; i++){
            if((x_bullet+5 > centipede.centipede_parts[i].x && y > centipede.centipede_parts[i].y) && (x_bullet+5 < centipede.centipede_parts[i].x + 10  && y < centipede.centipede_parts[i].y + 10)){
                if(!centipede.centipede_parts[i].shot){
                    ST7789_DrawRectangle(x_bullet, y, 10, 10, ST7789_BLACK);
                    G8RTOS_SignalSemaphore(&sem_SPIA);
                    centipede.centipede_parts[i].shot = true;

                    score += 100;

                    obs[numObs].x = centipede.centipede_parts[i].x;
                    obs[numObs].y = centipede.centipede_parts[i].y;
                    obs[numObs].w = centipede.centipede_parts[i].w;
                    obs[numObs].h = centipede.centipede_parts[i].h;
                    obs[numObs].shot = false;
                    obs[numObs].hasDrawn = false;

                    PCA9556b_SetLED(led_matrix[i], 0, 0);

                    centipede.partCount--;
                    numObs++;
                    G8RTOS_KillSelf();
                }
            }
        }

        //Collides with an object
        for(int i = 0; i < numObs; i++){
            if(!obs[i].shot){
                if ((x_bullet < obs[i].x + obs[i].w) && (x_bullet + 5 > obs[i].x) &&
                    (y < obs[i].y + obs[i].h) && (y + 5 > obs[i].y)){
                    obs[i].shot = true;
                    score += 50;
                    ST7789_DrawRectangle(x_bullet, y, 10, 10, ST7789_BLACK);
                    G8RTOS_SignalSemaphore(&sem_SPIA);
                    G8RTOS_KillSelf();
                }
            }
        }

        G8RTOS_SignalSemaphore(&sem_SPIA);
        y_old = y;
        sleep(1);
    }
}

/* Enemy Thread
 * Description: Handles Enemy Behavior
 *      - Moving the centipede across and down the screen
 *      - Collision to objects:
 *          - Moving around the object
 *      - Drawing obstacle over an enemy if shot
 *      - When to kill thread
 * */
void enemy(void) {
    // Initialize the centipede
    //cent_t centipede;
    centipede.links = linkCount;
    centipede.partCount = linkCount;
    // Set initial positions for each part of the centipede
    for (uint8_t i = 0; i < centipede.links; i++) {
        centipede.centipede_parts[i].x = x_global_en - i * 15; // Space out each part horizontally
        centipede.centipede_parts[i].y = y_global_en;
        centipede.centipede_parts[i].w = 10;
        centipede.centipede_parts[i].h = 10;
        centipede.centipede_parts[i].shot = false;
    }

    while (1) {
        for(int i = 0; i < centipede.links; i++)
            if(!centipede.centipede_parts[i].shot)
                PCA9556b_SetLED(led_matrix[i], 0xff, 0xff);

        // Clear the old positions of the centipede
        G8RTOS_WaitSemaphore(&sem_SPIA);
        for (uint8_t i = 0; i < centipede.links; i++) {
            if(!centipede.centipede_parts[i].shot){
                if(centipede.centipede_parts[i].x >= 0){
                    ST7789_DrawRectangle(
                        centipede.centipede_parts[i].x,
                        centipede.centipede_parts[i].y,
                        centipede.centipede_parts[i].w,
                        centipede.centipede_parts[i].h,
                        ST7789_BLACK
                    );
                }

                // Move Part
                if (centipede.centipede_parts[i].x > 200) {
                    centipede.centipede_parts[i].y -= 10;
                    centipede.centipede_parts[i].x = 0;
                } else {
                    centipede.centipede_parts[i].x += speed;
                }

                // Collision detection
                for(int j = 0; j < numObs; j++){
                    if (((centipede.centipede_parts[i].x + 10 > obs[j].x && centipede.centipede_parts[i].y + 10 > obs[j].y) &&
                         (centipede.centipede_parts[i].x < obs[j].x + 10 && centipede.centipede_parts[i].y < obs[j].y + 10)) &&
                        !obs[j].shot) {
                        centipede.centipede_parts[i].y -= 10;
                    }
                }

                //Redraw
                if(centipede.centipede_parts[i].x >= 0){
                    uint8_t y_global = centipede.centipede_parts[i].y; // Start at the obstacle's Y coordinate
                    for (int row = 9; row >= 0; row--) { // Loop over 10 rows of the bitmap
                        uint16_t value = centipedeSp[row];
                        uint8_t x_global = centipede.centipede_parts[i].x; // Start at the obstacle's X coordinate
                        for (int col = 9; col >= 0; col--) { // Loop over 10 bits in the row
                            if ((row == 1 || row == 8) && (col == 0 || col == 1 || col == 9 || col == 8)){
                                ST7789_DrawPixel(x_global, y_global, 0xffff);
                            } else if ((value >> col) & 1) { // Check if the bit is set
                                ST7789_DrawPixel(x_global, y_global, ST7789_BLUE); // Draw sprite pixel
                            } else {
                                ST7789_DrawPixel(x_global, y_global, ST7789_BLACK); // Draw background
                            }
                            x_global++; // Increment X for the next pixel
                        }
                        y_global++; // Increment Y for the next row
                    }
                }
                // Check game over condition
                if (centipede.centipede_parts[i].y <= 70) {
                    G8RTOS_SignalSemaphore(&sem_SPIA);
                    G8RTOS_AddThread(Lose, 1, "Bullet");
                    G8RTOS_KillSelf();
                }
            }
        }

        G8RTOS_SignalSemaphore(&sem_SPIA);

        G8RTOS_WaitSemaphore(&sem_SPIA);

        for(int i = 0; i < numObs; i++){
            if(obs[i].shot){
                ST7789_DrawRectangle(
                    obs[i].x,
                    obs[i].y,
                    obs[i].w,
                    obs[i].h,
                    ST7789_BLACK
                );
                obs[i].w = 0;

            }else{
                // Draw the bitmap sprite
                if(!obs[i].hasDrawn){
                    ST7789_DrawRectangle(
                        obs[i].x,
                        obs[i].y,
                        obs[i].w,
                        obs[i].h,
                        ST7789_BLACK
                    );
                    uint8_t y_global = obs[i].y; // Start at the obstacle's Y coordinate
                    for (int row = 9; row >= 0; row--) { // Loop over 10 rows of the bitmap
                        uint16_t value = mushroomSp[row];
                        uint8_t x_global = obs[i].x; // Start at the obstacle's X coordinate
                        for (int col = 9; col >= 0; col--) { // Loop over 10 bits in the row
                            if ((value >> col) & 1) { // Check if the bit is set
                                ST7789_DrawPixel(x_global, y_global, ST7789_GREEN); // Draw sprite pixel
                            } else {
                                ST7789_DrawPixel(x_global, y_global, ST7789_BLACK); // Draw background
                            }
                            x_global++; // Increment X for the next pixel
                        }
                        y_global++; // Increment Y for the next row
                    }
                }
            }
        }

        G8RTOS_SignalSemaphore(&sem_SPIA);



        if(centipede.partCount <= 0){
            G8RTOS_AddThread( Restart, 1, "Start\0");
            G8RTOS_KillSelf();
        }

        sleep(10);
    }
}

/* Obstacle Spawn
 * Description: Spawns 25 obstacles on the screen on random locations
 *      - After all are spawned the thread is killed
 * */
void obs_spawn(void) {
    for (int i = 0; i < 35; i++) {
        int x, y;
        do {
            x = (rand() % (220 - 20 - 10)) + 20;
            y = (rand() % (250 - 90 - 10)) + 90;
        } while (check_collision(x, y, 10, 10)); // Ensure no overlap

        obs[i].x = x;
        obs[i].y = y;
        obs[i].w = 10; // Width of the bitmap
        obs[i].h = 10; // Height of the bitmap
        obs[i].shot = 0;

        G8RTOS_WaitSemaphore(&sem_SPIA);

        // Draw the bitmap sprite
        uint8_t y_global = obs[i].y; // Start at the obstacle's Y coordinate
        for (int row = 9; row >= 0; row--) { // Loop over 10 rows of the bitmap
            uint16_t value = mushroomSp[row];
            uint8_t x_global = obs[i].x; // Start at the obstacle's X coordinate
            for (int col = 9; col >= 0; col--) { // Loop over 10 bits in the row
                if ((value >> col) & 1) { // Check if the bit is set
                    ST7789_DrawPixel(x_global, y_global, ST7789_GREEN); // Draw sprite pixel
                } else {
                    ST7789_DrawPixel(x_global, y_global, ST7789_BLACK); // Draw background
                }
                x_global++; // Increment X for the next pixel
            }
            y_global++; // Increment Y for the next row
        }

        G8RTOS_SignalSemaphore(&sem_SPIA);
        numObs++;
        obs[i].hasDrawn = true;
    }

    G8RTOS_KillSelf(); // End the thread when done
}


/* Restart
 * Description: Restarts a new instance of the game so it can go on forever
 *      - Each new time another link is added to the centipede
 *      - Loading animation is played on the LED Matrix
 *      - After one execution thread is killed
 * */
void Restart(void){
    linkCount++;
    numObs = 0;
    current_volume = 0;
    //speed+=1;
    while(1){
        G8RTOS_WaitSemaphore(&sem_SPIA);
        ST7789_Fill(ST7789_BLACK);
        G8RTOS_SignalSemaphore(&sem_SPIA);


        for(int i = 0; i < 20; i++){
            PCA9556b_SetLED(led_matrix[i], 0xff, 0xff);
            sleep(30);
        }

        for(int i = 19; i >= 0; i--){
            PCA9556b_SetLED(led_matrix[i], 0x00, 0x00);
            sleep(30);
        }

        PCA9956b_SetAllOff();
        G8RTOS_AddThread(enemy, 1, "en\0");
        G8RTOS_AddThread( obs_spawn, 1, "sp\0");


        G8RTOS_KillSelf();
    }
}

/* Start
 * Description: Starts a new instance of the game so it can go on forever
 *      - Loads 5 links to the centipede
 *      - Loading animation is played on the LED Matrix
 *      - Draws Start Text
 *      - Adds all threads from calling this in Main
 *      - After one execution thread is killed
 * */
void Start(void){
    G8RTOS_WaitSemaphore(&sem_SPIA);
    ST7789_Fill(ST7789_BLACK);
    G8RTOS_SignalSemaphore(&sem_SPIA);
    while(1){
        //Display Letters
        display_drawChar(30, 160, 'S', 0xffff, 0xffff, 5);
        display_drawChar(65, 160, 'T', 0xffff, 0xffff, 5);
        display_drawChar(100, 160, 'A', 0xffff, 0xffff, 5);
        display_drawChar(135, 160, 'R', 0xffff, 0xffff, 5);
        display_drawChar(170, 160, 'T', 0xffff, 0xffff, 5);

        display_drawChar(30, 120, 'P', 0xffff, 0xffff, 2);
        display_drawChar(41, 120, 'R', 0xffff, 0xffff, 2);
        display_drawChar(52, 120, 'E', 0xffff, 0xffff, 2);
        display_drawChar(63, 120, 'S', 0xffff, 0xffff, 2);
        display_drawChar(74, 120, 'S', 0xffff, 0xffff, 2);
        display_drawChar(85, 120, ' ', 0xffff, 0xffff, 2);
        display_drawChar(96, 120, 'S', 0xffff, 0xffff, 2);
        display_drawChar(107, 120, 'W', 0xffff, 0xffff, 2);
        display_drawChar(118, 120, '2', 0xffff, 0xffff, 2);
        display_drawChar(129, 120, '/', 0xffff, 0xffff, 2);
        display_drawChar(140, 120, 'S', 0xffff, 0xffff, 2);
        display_drawChar(151, 120, 'W', 0xffff, 0xffff, 2);
        display_drawChar(162, 120, '3', 0xffff, 0xffff, 2);

        display_drawChar(30, 80, 'S', 0xffff, 0xffff, 1);
        display_drawChar(35, 80, 'W', 0xffff, 0xffff, 1);
        display_drawChar(40, 80, '1', 0xffff, 0xffff, 1);
        display_drawChar(45, 80, ' ', 0xffff, 0xffff, 1);
        display_drawChar(50, 80, 'T', 0xffff, 0xffff, 1);
        display_drawChar(55, 80, 'O', 0xffff, 0xffff, 1);
        display_drawChar(60, 80, ' ', 0xffff, 0xffff, 1);
        display_drawChar(65, 80, 'F', 0xffff, 0xffff, 1);
        display_drawChar(70, 80, 'I', 0xffff, 0xffff, 1);
        display_drawChar(75, 80, 'R', 0xffff, 0xffff, 1);
        display_drawChar(80, 80, 'E', 0xffff, 0xffff, 1);

        display_drawChar(30, 70, 'S', 0xffff, 0xffff, 1);
        display_drawChar(35, 70, 'W', 0xffff, 0xffff, 1);
        display_drawChar(40, 70, '2', 0xffff, 0xffff, 1);
        display_drawChar(50, 70, '-', 0xffff, 0xffff, 1);
        display_drawChar(60, 70, 'J', 0xffff, 0xffff, 1);
        display_drawChar(65, 70, 'O', 0xffff, 0xffff, 1);
        display_drawChar(70, 70, 'Y', 0xffff, 0xffff, 1);
        display_drawChar(75, 70, 'S', 0xffff, 0xffff, 1);
        display_drawChar(80, 70, 'T', 0xffff, 0xffff, 1);
        display_drawChar(85, 70, 'I', 0xffff, 0xffff, 1);
        display_drawChar(90, 70, 'C', 0xffff, 0xffff, 1);
        display_drawChar(95, 70, 'K', 0xffff, 0xffff, 1);
        display_drawChar(100, 70, ' ', 0xffff, 0xffff, 1);
        display_drawChar(105, 70, 'T', 0xffff, 0xffff, 1);
        display_drawChar(110, 70, 'O', 0xffff, 0xffff, 1);
        display_drawChar(115, 70, ' ', 0xffff, 0xffff, 1);
        display_drawChar(120, 70, 'M', 0xffff, 0xffff, 1);
        display_drawChar(125, 70, 'O', 0xffff, 0xffff, 1);
        display_drawChar(130, 70, 'V', 0xffff, 0xffff, 1);
        display_drawChar(135, 70, 'E', 0xffff, 0xffff, 1);

        display_drawChar(30, 60, 'S', 0xffff, 0xffff, 1);
        display_drawChar(35, 60, 'W', 0xffff, 0xffff, 1);
        display_drawChar(40, 60, '3', 0xffff, 0xffff, 1);
        display_drawChar(50, 60, '-', 0xffff, 0xffff, 1);
        display_drawChar(60, 60, 'A', 0xffff, 0xffff, 1);
        display_drawChar(65, 60, 'C', 0xffff, 0xffff, 1);
        display_drawChar(70, 60, 'E', 0xffff, 0xffff, 1);
        display_drawChar(75, 60, 'L', 0xffff, 0xffff, 1);
        display_drawChar(80, 60, 'L', 0xffff, 0xffff, 1);
        display_drawChar(85, 60, ' ', 0xffff, 0xffff, 1);
        display_drawChar(90, 60, 'T', 0xffff, 0xffff, 1);
        display_drawChar(95, 60, 'O', 0xffff, 0xffff, 1);
        display_drawChar(100, 60, ' ', 0xffff, 0xffff, 1);
        display_drawChar(105, 60, 'M', 0xffff, 0xffff, 1);
        display_drawChar(110, 60, 'O', 0xffff, 0xffff, 1);
        display_drawChar(115, 60, 'V', 0xffff, 0xffff, 1);
        display_drawChar(125, 60, 'E', 0xffff, 0xffff, 1);

        display_drawChar(30, 50, 'L', 0xffff, 0xffff, 1);
        display_drawChar(35, 50, 'E', 0xffff, 0xffff, 1);
        display_drawChar(40, 50, 'D', 0xffff, 0xffff, 1);
        display_drawChar(45, 50, ' ', 0xffff, 0xffff, 1);
        display_drawChar(50, 50, 'M', 0xffff, 0xffff, 1);
        display_drawChar(55, 50, 'A', 0xffff, 0xffff, 1);
        display_drawChar(60, 50, 'T', 0xffff, 0xffff, 1);
        display_drawChar(65, 50, 'R', 0xffff, 0xffff, 1);
        display_drawChar(70, 50, 'I', 0xffff, 0xffff, 1);
        display_drawChar(75, 50, 'X', 0xffff, 0xffff, 1);
        display_drawChar(80, 52, ':', 0xffff, 0xffff, 2);
        display_drawChar(85, 50, ' ', 0xffff, 0xffff, 1);
        display_drawChar(90, 50, 'E', 0xffff, 0xffff, 1);
        display_drawChar(95, 50, 'N', 0xffff, 0xffff, 1);
        display_drawChar(100, 50, 'E', 0xffff, 0xffff, 1);
        display_drawChar(105, 50, 'M', 0xffff, 0xffff, 1);
        display_drawChar(110, 50, 'I', 0xffff, 0xffff, 1);
        display_drawChar(115, 50, 'E', 0xffff, 0xffff, 1);
        display_drawChar(120, 50, 'S', 0xffff, 0xffff, 1);
        display_drawChar(125, 50, ' ', 0xffff, 0xffff, 1);
        display_drawChar(130, 50, 'L', 0xffff, 0xffff, 1);
        display_drawChar(135, 50, 'E', 0xffff, 0xffff, 1);
        display_drawChar(140, 50, 'F', 0xffff, 0xffff, 1);
        display_drawChar(145, 50, 'T', 0xffff, 0xffff, 1);


        //Wait to Start Semaphore
        G8RTOS_WaitSemaphore(&sem_StartGame);

        ST7789_Fill(ST7789_BLACK);

        start_State = 0;
        //Add All Background Threads
        G8RTOS_AddThread( Ship_Thread, 1, "camera\0");

        for(int i = 0; i < 20; i++){
            PCA9556b_SetLED(led_matrix[i], 0xff, 0xff);
            sleep(30);
        }

        for(int i = 19; i >= 0; i--){
            PCA9556b_SetLED(led_matrix[i], 0x00, 0x00);
            sleep(30);
        }

        G8RTOS_AddThread(enemy, 1, "en\0");
        G8RTOS_AddThread( obs_spawn, 2, "sp\0");
        G8RTOS_AddThread( DisplayScore, 5, "sc\0");

        G8RTOS_KillSelf();
    }
}

/* Lose
 * Description: If centipede raches player lose condition follows
 *      - Kills all threads except button, and idle thread
 *      - Draws Lose Text
 * */
void Lose(void){
    lost = 1;
    current_volume = 0;
    lose_State = 1;
    G8RTOS_WaitSemaphore(&sem_SPIA);
    ST7789_Fill(ST7789_BLACK);
    G8RTOS_SignalSemaphore(&sem_SPIA);
    while(1){
        G8RTOS_WaitSemaphore(&sem_SPIA);
        display_drawChar(80, 200, 'Y', 0xffff, 0xffff, 5);
        display_drawChar(115, 200, 'O', 0xffff, 0xffff, 5);
        display_drawChar(150, 200, 'U', 0xffff, 0xffff, 5);

        display_drawChar(65, 160, 'L', 0xffff, 0xffff, 5);
        display_drawChar(100, 160, 'O', 0xffff, 0xffff, 5);
        display_drawChar(135, 160, 'S', 0xffff, 0xffff, 5);
        display_drawChar(170, 160, 'T', 0xffff, 0xffff, 5);

        display_drawChar(70, 120, 'P', 0xffff, 0xffff, 2);
        display_drawChar(81, 120, 'R', 0xffff, 0xffff, 2);
        display_drawChar(92, 120, 'E', 0xffff, 0xffff, 2);
        display_drawChar(103, 120, 'S', 0xffff, 0xffff, 2);
        display_drawChar(114, 120, 'S', 0xffff, 0xffff, 2);
        display_drawChar(125, 120, ' ', 0xffff, 0xffff, 2);
        display_drawChar(136, 120, 'R', 0xffff, 0xffff, 2);
        display_drawChar(147, 120, 'E', 0xffff, 0xffff, 2);
        display_drawChar(158, 120, 'S', 0xffff, 0xffff, 2);
        display_drawChar(169, 120, 'E', 0xffff, 0xffff, 2);
        display_drawChar(180, 120, 'T', 0xffff, 0xffff, 2);

        char score_str[5]; // 5 digits + null terminator
        // Convert the score to a string
        sprintf(score_str, "%d", score);

        //Score:
        display_drawChar(10, 265, 'S', 0xffff, 0xffff, 2);
        display_drawChar(21, 265, 'c', 0xffff, 0xffff, 2);
        display_drawChar(32, 265, 'o', 0xffff, 0xffff, 2);
        display_drawChar(43, 265, 'r', 0xffff, 0xffff, 2);
        display_drawChar(54, 265, 'e', 0xffff, 0xffff, 2);
        display_drawChar(65, 265, ':', 0xffff, 0xffff, 2);


        //DIGIT NUMBER
        // Display the score digits
        int x = 76;
        // Starting position for the score digits
        for (int i = 0; score_str[i] != '\0'; i++) {
            display_drawChar(x, 265, score_str[i], 0xffff, 0xffff, 2);
            x += 12; // Move to the next position }
        }
        G8RTOS_SignalSemaphore(&sem_SPIA);

        G8RTOS_WaitSemaphore(&sem_LostGame);
        lose_State = 0;
        G8RTOS_AddThread( Start, 1, "Start\0");
        G8RTOS_KillSelf();
    }
}
/********************************Periodic Threads***********************************/

/* Get_JoystickX
 * Description: Periodically Gets X data from joystic
 *      - Sends to the Ship thread over a FIFO
 * */
void Get_JoystickX(void) {
    // Read the joystick
    uint32_t readXY = JOYSTICK_GetXY();
    // Send through FIFO.
    if(isJoyStick)
        G8RTOS_WriteFIFO(JOYSTICK_FIFO , readXY);
}

/* Get_AccelX
 * Description: Periodically Gets X data from Acceleration
 *      - Sends to the Ship thread over a FIFO
 * */
void Get_AccelX(void) {
    int16_t AccelX = BMI160_AccelXGetResult();
    if(!isJoyStick)
        G8RTOS_WriteFIFO(Accel_FIFO , AccelX);
}



/*******************************Aperiodic Threads***********************************/

/* GPIOE_Handler()
 * Description: Runs on the press of a multimod button
 *      - Sends to Read Buttons
 * */
void GPIOE_Handler() {
    // Disable interrupt
    GPIOIntDisable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
    GPIOIntClear(BUTTONS_INT_GPIO_BASE,  BUTTONS_INT_PIN);
    // Signal relevant semaphore
    G8RTOS_SignalSemaphore(&sem_PCA9555_Debounce);
}

/* DAC_Timer_Handler
 * Description: Runs when switch one is pressed to collect frequency data to output
 *      - Sends to Read Buttons for shooting sound effect
 * */
void DAC_Timer_Handler() {

    // clear the timer interrupt
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // read next output sample
    uint32_t output = (current_volume)*(dac_signal[dac_step++ % SIGNAL_STEPS]);

    // write the output value to the dac
    MutimodDAC_Write(DAC_OUT_REG, output);
}

