
/* Neftali Percastegui & nperc001@ucr.edu:
* Discussion Section: 024
* Assignment: Lab # 8 Exercise # 1
*
*
* I acknowledge all content contained herein, excluding template or example code, is
my own original work.
*
* https://youtu.be/XbzvwEzVO30
*/
#include "timerISR.h"
#include "helper.h"
#include "periph.h"
#include <Arduino.h>
#include "serialATmega.h"
#include "spiAVr.h"
#include "LCD.h"

#define C5_Sharp 3609//554
#define E5_Flat 3034//659
#define F5_Sharp 2702//740
#define B4_Flat 4048//494
#define A4_Flat 4544//440
#define A4_Sharp 4291 //466
#define F4_Sharp 5404//370
#define G4_Sharp 4818//415
#define D4_Sharp 6430 //311
#define C4_Sharp 7219 // 277

#define NUM_TASKS 3//4 // TODO: Change to the number of tasks being used
// main background song
int I_Want_Billions[45] = {C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, E5_Flat, F5_Sharp, C5_Sharp, C5_Sharp, B4_Flat, A4_Flat, A4_Flat, G4_Sharp, F4_Sharp, F4_Sharp, F4_Sharp, A4_Flat, C5_Sharp, B4_Flat, A4_Flat, F4_Sharp, B4_Flat, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, E5_Flat, F5_Sharp, C5_Sharp, C5_Sharp, B4_Flat, A4_Flat, A4_Flat, G4_Sharp, F4_Sharp, F4_Sharp, F4_Sharp, A4_Flat, C5_Sharp, B4_Flat, A4_Flat, F4_Sharp, B4_Flat};
int I_want_Time[45] = {1, 1, 1, 1, 1, 2, 1, 5, 4, 2, 4, 2, 4, 3, 3, 3, 3, 2, 2, 2, 2, 8, 1, 1, 1, 1, 1, 1, 2, 1, 2, 4, 1, 2, 1, 4, 2, 2, 2, 2, 2, 2,2 ,2 ,6};

// second song if user borrows money
int Run_Away_Baby[41] = {A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, D4_Sharp, D4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, A4_Sharp, G4_Sharp, D4_Sharp, F4_Sharp, G4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, D4_Sharp, D4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, A4_Sharp, G4_Sharp};
int Run_Away_Time[41] = {2, 2, 1, 1, 2, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 4, 1, 1, 1, 2, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4};


int suits[] = {1,2,3,4};
// int club_cards[] = {1,2,3,4,5,6,7,8,9,10,11};
// int heart_cards[] = {1,2,3,4,5,6,7,8,9,10,11};
// int diamond_cards[] = {1,2,3,4,5,6,7,8,9,10,11};
// int spade_cards[] = {1,2,3,4,5,6,7,8,9,10,11};
int card_values[] = {1,2,3,4,5,6,7,8,9,10,11};

int value;
// value = club_cards[rand()%13];
// serial_println(value);

unsigned char is_up;
unsigned char is_down;
unsigned char j;
unsigned char i;
unsigned char player_win;
unsigned char player_loss;
unsigned int player_money;
unsigned int player_bet;
unsigned int player_loan;
unsigned int dealer_bet;
unsigned char cardCount;
unsigned char player_suit;
unsigned char dealer_suit;
unsigned int player_face;
unsigned int dealer_face;
volatile unsigned char is_bet;
char *dealer_card;
char *player_card;
char *dealer_cardF;
char *player_cardF;

// Task struct for concurrent synchSMs implmentations
typedef struct _task
{
    signed char state;         // Task's current state
    unsigned long period;      // Task period
    unsigned long elapsedTime; // Time elapsed since last task tick
    int (*TickFct)(int);       // Task tick function
} task;

// TODO: Define Periods for each task
//  e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long JS_Period = 100;
const unsigned long Card_Period = 1000;
const unsigned long Background_Period = 200;
const unsigned long Bet_Period = 1000;

const unsigned long Direction_Period = 500;
const unsigned long GCD_PERIOD = findGCD(JS_Period, Bet_Period); // TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks

void HardwareReset()
{
    PORTD = SetBit(PORTD, 4, 0);// setResetPinToLow;
    _delay_ms(200);
    PORTD = SetBit(PORTD, 4, 1);// setResetPinToHigh;
    _delay_ms(200);
}

void ST7735_init()
{
    HardwareReset();
    PORTB = SetBit(PORTB, 2, 0); //cs
    PORTD = SetBit(PORTD, 5, 0); //a0
    SPI_SEND(0x01);// Send_Command(SWRESET);
    _delay_ms(150);
    SPI_SEND(0x11);// Send_Command(SLPOUT);
    _delay_ms(200);
    SPI_SEND(0x3A);// Send_Command(COLMOD);
    PORTD = SetBit(PORTD, 5, 1); //a0
    SPI_SEND(0x06);// Send_Data(0x06); // for 18 bit color mode. You can pick any color mode you want
    _delay_ms(10);
    // Send_Command(DISPON);
    PORTD = SetBit(PORTD, 5, 0); //a0
    SPI_SEND(0x29);// Send_Command(DISPON);
    _delay_ms(200);
    //set x lines
    SPI_SEND(0x2A);
    PORTD = SetBit(PORTD, 5, 1); //a0
    _delay_ms(200);
    SPI_SEND(0x00);
    SPI_SEND(0x11);
    _delay_ms(200);
    SPI_SEND(0x00);
    SPI_SEND(0xFF);
    _delay_ms(200);
    PORTD = SetBit(PORTD, 5, 0); //a0
    SPI_SEND(0x2C);
    _delay_ms(200);
    PORTD = SetBit(PORTD, 5, 1); //a0

}

void TimerISR()
{
    for (unsigned int i = 0; i < NUM_TASKS; i++)
    { // Iterate through each task in the task array
        if (tasks[i].elapsedTime == tasks[i].period)
        {                                                      // Check if the task is ready to tick
            tasks[i].state = tasks[i].TickFct(tasks[i].state); // Tick and set the next state for this task
            tasks[i].elapsedTime = 0;                          // Reset the elapsed time for the next tick
        }
        tasks[i].elapsedTime += GCD_PERIOD; // Increment the elapsed time by GCD_PERIOD
    }
}

int stages[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001}; // Stepper motor phases

// TODO: Create your tick functions for each task

// enum direction_state{idle_direction, CW_Direction, CCW_Direction};
// int TickFtn_direction(int state);
enum JS_state{idle_state, up_state, down_state};
int TickFtn_JS(int state);

enum Back_state{idleMusic_state, note_state, play_state};
int TickFtn_back(int state);

enum bet_state{idle_money, bet_state, loan_state};
int TickFtn_Bet(int state);

// enum card_state{idle_card, card_deal, suit_state, value_state};
// int TickFtn_Card(int state);


int main(void)
{
    // TODO: initialize all your inputs and ouputs

    ADC_init(); // initializes ADC
    lcd_init();
 
    //  Output: DDR = 1, PORT = 0
    //  Input: DDR = 0, PORT = 1
    DDRC = 0b000000; PORTC = 0b111111;
    DDRB = 0b111111; PORTB = 0b000000;
    DDRD = 0b11111111; PORTD = 0b00000000;
    serial_init(9600);
    SPI_INIT();
    ST7735_init();

    // //TODO: Initialize the buzzer timer/pwm(timer0)
    OCR0A = 128; // sets duty cycle to 50% since TOP is always 256

    // //TODO: Initialize the servo timer/pwm(timer1)
    TCCR1A |= (1 << WGM11) | (1 << COM1A1);              // COM1A1 sets it to channel A
    TCCR1B |= (1 << WGM12) | (1 << WGM13) | (1 << CS11); // CS11 sets the prescaler to be 8
    // //WGM11, WGM12, WGM13 set timer to fast pwm mode

    // ICR1 = 39999; //20ms pwm period

    // OCR1A =  1999/* set your value here */ ;

    // TODO: Initialize tasks here
    //  e.g.
    //  tasks[0].period = ;
    //  tasks[0].state = ;
    //  tasks[0].elapsedTime = ;
    //  tasks[0].TickFct = ;
    //  task 1
    unsigned char i = 0;
    tasks[i].state = idle_state;
    tasks[i].period = JS_Period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_JS;
    i++;
    tasks[i].state = idleMusic_state;
    tasks[i].period = Background_Period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_back;
    i++;
    tasks[i].state = idle_money;
    tasks[i].period = Bet_Period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_Bet;
    // i++;
    // tasks[i].state = idle_card;
    // tasks[i].period = Card_Period;
    // tasks[i].elapsedTime = tasks[i].period;
    // tasks[i].TickFct = &TickFtn_Card;
    
    

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1)
    {
        // for(int i = 0; i<I_Want_Billions[i]; i++){
        //     ICR1 = I_Want_Billions[i];
        //     OCR1A = ICR1/2;
        //     _delay_ms(500);
        // }
    }

    return 0;
}

// enum JS_Period{idle_state, up_state, down_state};
int TickFtn_JS(int state)
{
    switch (state)
    {
    case idle_state:
        if (ADC_read(0) >= 600)
        {
            is_up = 0;
            state = up_state;
        }
        else if (ADC_read(0) < 300)
        {
            is_down = 0;
            state = down_state;
        }
        else
        {
            state = idle_state;
        }
        break;

    case up_state:
        if (ADC_read(0) >= 600)
        {
            state = up_state;
        }
        else if (ADC_read(0) < 600)
        {
            is_up = 0;
            state = idle_state;
        }
        break;

    case down_state:
        if (ADC_read(0) <= 300)
        {
            state = down_state;
        }
        else if (ADC_read(0) > 300)
        {
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
        break;

    case up_state:
        is_up = 1;
        // serial_println(is_up);
        break;

    case down_state:
        is_down = 1;
        // serial_println(is_down);
        break;

    default:
        break;
    }
    return state;
}

// enum bet_state{idle_money, bet_state, loan_state};
int TickFtn_Bet(int state){
    switch (state)
    {
    case idle_money:
        player_money = 1000;
        player_bet = 0;
        player_loan = 0;
        is_bet = 0;
        if(is_up){
            serial_println("press up to increase current bet");
            state = bet_state;
        }
        else{
            state = idle_money;
        }
        break;
    case bet_state:
        if(player_money > 0 && is_up){
            player_bet = player_bet + 100;
            dealer_bet = player_bet;
            serial_println("current bet is");
            serial_println(player_bet);
            state = bet_state;
        }
        else if(player_money <= 0){
            serial_println("how much would you like to borrow?");
            player_loan = 0;
            player_bet = 0;
            is_bet = 0;
            state = loan_state;
        }
        break;
    case loan_state:
        if(is_up){
            serial_println("currently borrowed");
            serial_println(player_loan);
            state = loan_state;
        }
        else if(is_down){
            state = idle_money;
        }
        else if(player_loan > 0 && !((PINC >> 2)&0x01)){
            is_bet = 0;
            player_money = player_loan;
            state = bet_state;
        }
        break;

    default:
        break;
    }
    switch (state)
    {
    case idle_money:
        break;
    case bet_state:
        //serial_println("do you want to make a bet");
        if(!((PINC >> 2)&0x01)){
                is_bet = 1;
                serial_println("hello");
        }
        if(player_loss){
            player_money = player_money - player_bet;
        }
        else if(player_win){
            player_money = player_money + dealer_bet;
        }
        break;
    case loan_state:
        player_loan = player_loan + 100;
        break;
    default:
        break;
    }
    return state;
}

// // enum card_state{idle_card, suit_state, value_state};
// int TickFtn_Card(int state){
//     switch (state)
//     {
//     case idle_card:
//         cardCount = 52;
//         dealer_suit = 0;
//         dealer_face = 0;
//         player_suit = 0;
//         player_face = 0;
//         state = card_deal;
//     break;

//     case card_deal:
//         if(is_up){
//             dealer_suit = suits[rand()%3];
//             player_suit = suits[rand()%3];
//             state = suit_state;
//         }
//         else{
//             state = idle_card;
//         }
//     break;

//     case suit_state:
//         dealer_face = card_values[rand()%13];
//         player_face = card_values[rand()%13];
//         state = value_state;
//     break;

//     case value_state:
//         state = idle_card;
//     break;

//     default:
//         break;
//     }

//     switch (state)
//     {
//     case idle_card:
//     break;

//     case card_deal:
//         serial_println(dealer_cardF);
//         serial_println(dealer_card);
//         serial_println("-------------");
//         serial_println(player_cardF);
//         serial_println(player_card);
//         if(dealer_face > player_face){
//             serial_println("dealer wins!!");
//         }
//         else if(dealer_face < player_face){
//             serial_println("Player wins!!!");
//         }
//     break;


//     case suit_state:
//         if( dealer_suit == 1){
//             dealer_card = "clubs";
//         }
//         else if(dealer_suit == 2){
//             dealer_card = "hearts";
//         }
//         else if(dealer_suit == 3){
//             dealer_card = "diamonds";
//         }
//         else{
//             dealer_card = "spades";
//         }
//         if( player_suit == 1){
//             player_card = "clubs";
//         }
//         else if(player_suit == 2){
//             player_card = "hearts";
//         }
//         else if(player_suit == 3){
//             player_card = "diamonds";
//         }
//         else{
//             player_card = "spades";
//         }
//     break;

//     case value_state:
//     if( dealer_face == 1){
//             dealer_cardF = "Ace";
//         }
//         else if(dealer_face == 2){
//             dealer_cardF = "2 of";
//         }
//         else if(dealer_face == 3){
//             dealer_cardF = "3 of";
//         }
//         else if(dealer_face == 4){
//             dealer_cardF = "4 of";
//         }
//         else if(dealer_face == 5){
//             dealer_cardF = "5 of";
//         }
//         else if(dealer_face == 6){
//             dealer_cardF = "6 of";
//         }
//         else if(dealer_face == 7){
//             dealer_cardF = "7 of";
//         }
//         else if(dealer_face == 8){
//             dealer_cardF = "8 of";
//         }
//         else if(dealer_face == 9){
//             dealer_cardF = "3 of";
//         }
//         else if(dealer_face == 10){
//             dealer_cardF = "10 of";
//         }
//         else if(dealer_face == 11){
//             dealer_cardF = "Jack of";
//         }
//         else if(dealer_face == 12){
//             dealer_cardF = "Queen of";
//         }
//         else if(dealer_face == 13){
//             dealer_cardF = "King of";
//         }
//         else{
//             dealer_cardF = "";
//         }
//         if( player_face == 1){
//             player_cardF = "Ace of";
//         }
//         else if(player_face == 2){
//             player_cardF = "2 of";
//         }
//         else if(player_face == 3){
//             player_cardF = "3 of";
//         }
//         else if(player_face == 4){
//             player_cardF = "4 of";
//         }
//         else if(player_face == 5){
//             player_cardF = "5 of";
//         }
//         else if(player_face == 6){
//             player_cardF = "6 of";
//         }
//         else if(player_face == 7){
//             player_cardF = "7 of";
//         }
//         else if(player_face == 8){
//             player_cardF = "8 of";
//         }
//         else if(player_face == 9){
//             player_cardF = "9 of";
//         }
//         else if(player_face == 10){
//             player_cardF = "10 of";
//         }
//         else if(player_face == 11){
//             player_cardF = "Jack of";
//         }
//         else if(player_face == 12){
//             player_cardF = "Queen of";
//         }
//         else if(player_face == 13){
//             player_cardF = "King of";
//         }
//         else{
//             player_cardF = "";
//         }
//     break;

//     default:
//         break;
//     }

//     return state;
// }

// enum Back_state{idleMusic_state, note_state, play_state};
int TickFtn_back(int state){
    switch (state)
    {
    case idleMusic_state:
        i = 0;
        state = note_state;
    break;

    case note_state:
    if(i < 45){
        state = play_state;
    }
    else{
        state = idleMusic_state;
    }
        
    break;

    case play_state:
        if(j > 0){
            state = play_state;
        }
        else{
            i++;
            state = note_state;
        }
    default:
        break;
    }
    switch (state)
    {
    case idleMusic_state:
    i = 0;
    break;

    case note_state:
        ICR1 = I_Want_Billions[i];
        j = I_want_Time[i];
    break;

    case play_state:
        OCR1A = ICR1/2;
        j--;
    
    default:
        break;
    }
    return state;
}

