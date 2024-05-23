/*        Neftali Percastegui & nperc001@ucr.edu:
 *          Discussion Section: 024
 *         Assignment: Lab # 7 Exercise # 3
 *
 *
 *         I acknowledge all content contained herein, excluding template or example code, is my own original work.
 *
 *         https://youtu.be/19CRrsWQhew
 */
#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include <Arduino.h>
#include "serialATmega.h"
#include "spiAVr.h"

#define C5_Sharp 554
#define E5_Flat 659
#define F5_Sharp 740
#define B4_Flat 494
#define A4_Flat 440
#define F4_Sharp 370
#define G4_Sharp 415

int I_Want_Billions[] = {C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, E5_Flat, F5_Sharp, C5_Sharp, C5_Sharp, B4_Flat, A4_Flat, A4_Flat, G4_Sharp, F4_Sharp, F4_Sharp, F4_Sharp, A4_Flat, C5_Sharp, B4_Flat, A4_Flat, F4_Sharp, B4_Flat}; 

int I_want_Time[] = {1, 1, 1, 1, 1, 2, 1, 5, 4, 2, 4, 2, 4, 3, 3, 3, 3, 2, 2, 2, 2, 8};

unsigned char is_up;
unsigned char is_down;

#define NUM_TASKS 1 //TODO: Change to the number of tasks being used


// Task struct for concurrent synchSMs implmentations
typedef struct _task{
	signed 	 char state; 		//Task's current state
	unsigned long period; 		//Task period
	unsigned long elapsedTime; 	//Time elapsed since last task tick
	int (*TickFct)(int); 		//Task tick function
} task;


//TODO: Define Periods for each task
// e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long JS_Period = 1;

// const unsigned long Direction_Period = 500;
const unsigned long GCD_PERIOD = JS_Period;//TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks


void TimerISR() {
	for ( unsigned int i = 0; i < NUM_TASKS; i++ ) {                   // Iterate through each task in the task array
		if ( tasks[i].elapsedTime == tasks[i].period ) {           // Check if the task is ready to tick
			tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
			tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
		}
		tasks[i].elapsedTime += GCD_PERIOD;                        // Increment the elapsed time by GCD_PERIOD
	}
}


int stages[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};//Stepper motor phases

//TODO: Create your tick functions for each task

// enum direction_state{idle_direction, CW_Direction, CCW_Direction};
// int TickFtn_direction(int state);
enum JS_Period{idle_state, up_state, down_state};
int TickFtn_JS(int state);

int main(void) {
    //TODO: initialize all your inputs and ouputs

    ADC_init();   // initializes ADC
    SPI_INIT ();
    //  Output: DDR = 1, PORT = 0
    //  Input: DDR = 0, PORT = 1
    DDRC = 0b000000; PORTC = 0b111111;
    DDRB = 0b111111; PORTB = 0b000000;
    DDRD = 0b11111111; PORTD = 0b00000000;
    serial_init(9600);

    // //TODO: Initialize the buzzer timer/pwm(timer0)
    // OCR0A = 128; //sets duty cycle to 50% since TOP is always 256

    // //TODO: Initialize the servo timer/pwm(timer1)
    // TCCR1A |= (1 << WGM11) | (1 << COM1A1); //COM1A1 sets it to channel A
    // TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); //CS11 sets the prescaler to be 8
    // //WGM11, WGM12, WGM13 set timer to fast pwm mode

    // ICR1 = 39999; //20ms pwm period

    //OCR1A =  1999/* set your value here */ ;


    //TODO: Initialize tasks here
    // e.g. 
    // tasks[0].period = ;
    // tasks[0].state = ;
    // tasks[0].elapsedTime = ;
    // tasks[0].TickFct = ;
    // task 1
    unsigned char i = 0;
    tasks[i].state = idle_state;
    tasks[i].period = JS_Period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_JS;

    // TimerSet(GCD_PERIOD);
    // TimerOn();

    while (1) {
    }

    return 0;
}

// enum JS_Period{idle_state, up_state, down_state};
int TickFtn_JS(int state){
    switch (state)
    {
    case idle_state:
        if(ADC_read(0) >= 600){
            is_up = 0;
            state = up_state;
        }
        else if(ADC_read(0) <300){
            is_down = 0;
            state = down_state;
        }
        else{
            state = idle_state;
        }
    break;

    case up_state:
        if(ADC_read(0) >= 600){
            state = up_state;
        }
        else if(ADC_read(0) < 600){
            is_up = 0;
            state = idle_state;
        }
    break;

    case down_state:
        if (ADC_read(0) <= 300)
        {
            state = down_state;
        }
        else if(ADC_read(0) > 300){
            is_down = 0;
            state = idle_state;
        }
    break;

    default:
        break;
    }

    switch (state)
    {
    case idle_state:
        serial_println(is_up);
        serial_println(is_down);
    break;

    case up_state:
        is_up = 1;
        serial_println(ADC_read(0));
    break;

    case down_state:
        is_down = 1;
        serial_println(is_down);
    break;

    default:
        break;
    }
    return state;
}