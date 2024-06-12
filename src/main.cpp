
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

#define NUM_TASKS 7 // TODO: Change to the number of tasks being used
// main background song
int I_Want_Billions[45] = {C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, E5_Flat, F5_Sharp, C5_Sharp, C5_Sharp, B4_Flat, A4_Flat, A4_Flat, G4_Sharp, F4_Sharp, F4_Sharp, F4_Sharp, A4_Flat, C5_Sharp, B4_Flat, A4_Flat, F4_Sharp, B4_Flat, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, E5_Flat, F5_Sharp, C5_Sharp, C5_Sharp, B4_Flat, A4_Flat, A4_Flat, G4_Sharp, F4_Sharp, F4_Sharp, F4_Sharp, A4_Flat, C5_Sharp, B4_Flat, A4_Flat, F4_Sharp, B4_Flat};
int I_want_Time[45] = {1, 1, 1, 1, 1, 2, 1, 5, 4, 2, 4, 2, 4, 3, 3, 3, 3, 2, 2, 2, 2, 8, 1, 1, 1, 1, 1, 1, 2, 1, 2, 4, 1, 2, 1, 4, 2, 2, 2, 2, 2, 2,2 ,2 ,6};

// second song if user borrows money
int Run_Away_Baby[41] = {A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, D4_Sharp, D4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, A4_Sharp, G4_Sharp, D4_Sharp, F4_Sharp, G4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, D4_Sharp, D4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, A4_Sharp, G4_Sharp};
int Run_Away_Time[41] = {2, 2, 1, 1, 2, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 4, 1, 1, 1, 2, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4};


int suits[4] = {1,2,3,4};
int card_values[13] = {1,2,3,4,5,6,7,8,9,10,11,12,13};

// String D_Card_Suits[4] = {"Clubs","Hearts","Spades","Diamonds"};
// char *D_CARD_Face[13] = {"1","2","3","4","5","6","7","8","9","10","J","Q","K"};
// int Dealer_vals[11] = {0,0,0,0,0,0,0,0,0,0,0};

// int value;
// value = club_cards[rand()%13];
// serial_println(value);

unsigned char is_up;
unsigned char is_down;
unsigned char j;
unsigned char i;
unsigned char ii;
unsigned char jj;
unsigned char k;
unsigned char l;
unsigned char m;
unsigned long t;
unsigned long tt;
unsigned long t2;
unsigned long tt2;
unsigned char ii2;
unsigned char jj2;
unsigned char k2;
unsigned char l2;
unsigned char m2;
unsigned char needCards2;
unsigned char player_total;
unsigned char p_card_total;
unsigned char display_value;
unsigned char dealer_total;
unsigned char d_card_total;
unsigned char player_win;
unsigned char player_loss;
int player_money;
unsigned int player_bet;
unsigned int player_loan;
unsigned int dealer_bet;
unsigned char cardCount;
unsigned int player_suit;
unsigned int dealer_suit;
unsigned int player_face;
unsigned int dealer_face;
unsigned char is_bet;
unsigned char newGame;
unsigned char game_start;
unsigned char player_limit;
unsigned char needCards;
unsigned char hit_me;
unsigned char player_choice;
// int temp_bet;
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
const unsigned long D_Card_Period = 1000;
const unsigned long P_Card_Period = 1000;
const unsigned long Background_Period = 200;
const unsigned long Bet_Period = 1000;
const unsigned long timer_period = 100;
const unsigned long timer2_period = 100;
const unsigned long Direction_Period = 500;
const unsigned long Winner_Period = 1000;

const unsigned long GCD_PERIOD = findGCD(JS_Period, D_Card_Period); // TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks

void HardwareReset()
{
    PORTC = SetBit(PORTC, 4, 0);// setResetPinToLow;
    _delay_ms(200);
    PORTC = SetBit(PORTC, 4, 1);// setResetPinToHigh;
    _delay_ms(200);
}

void ST7735_init()
{
    HardwareReset();
    PORTB = SetBit(PORTB, 2, 0); //cs
    PORTC = SetBit(PORTC, 3, 0); //a0
    SPI_SEND(0x01);// Send_Command(SWRESET);
    _delay_ms(150);
    SPI_SEND(0x11);// Send_Command(SLPOUT);
    _delay_ms(200);
    SPI_SEND(0x3A);// Send_Command(COLMOD);
    PORTC = SetBit(PORTC, 3, 1); //a0
    SPI_SEND(0x06);// Send_Data(0x06); // for 18 bit color mode. You can pick any color mode you want
    _delay_ms(10);
    // Send_Command(DISPON);
    PORTC = SetBit(PORTC, 3, 0); //a0
    SPI_SEND(0x29);// Send_Command(DISPON);
    _delay_ms(200);
    //////////set x lines///////////
    SPI_SEND(0x2A);
    PORTC = SetBit(PORTC, 3, 1); //a0
    _delay_ms(200);
    SPI_SEND(0x00);
    SPI_SEND(0x00);
    SPI_SEND(0x00);
    SPI_SEND(0x11);
    PORTC = SetBit(PORTC, 3, 0);
    _delay_ms(200);
    //////////set y lines//////////////////
    SPI_SEND(0x2B);
    PORTC = SetBit(PORTC, 3, 1);
    SPI_SEND(0x00);
    SPI_SEND(0x00);
    SPI_SEND(0x00);
    SPI_SEND(0x11);
    PORTC = SetBit(PORTC, 3, 0);
    //////////set color /////////////////
    SPI_SEND(0x2C);
    PORTC = SetBit(PORTC, 3, 1);
    for(int x = 0; x<289; x++){
        SPI_SEND(0xFF);
        SPI_SEND(0xFF);
        SPI_SEND(0xFF);
    }
    PORTC =SetBit(PORTC,3,0);


    // SPI_SEND(0x00);
    // SPI_SEND(0xFF);
    // _delay_ms(200);
    // PORTC = SetBit(PORTC, 3, 0); //a0
    // SPI_SEND(0x2C);
    // _delay_ms(200);
    // PORTC = SetBit(PORTC, 3, 1); //a0

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

enum JS_state{idle_state, up_state, down_state};
int TickFtn_JS(int state);

enum Back_state{idleMusic_state, note_state, play_state};
int TickFtn_back(int state);

enum bet_state{idle_money, betting_state, loan_state, money_state};
int TickFtn_Bet(int state);

enum card_state{D_idle_card, D_suit_state, D_face_state, D_val_state};
int TickFtn_Dealer(int state);

enum time_state{T_idle_state, timer_state};
int TickFtn_timer(int state);

enum timer2_state{T2_idle_state, timer2_state};
int TickFtn_timer2(int state);

enum Pcard_state{P_idle_state, P_suit_state, P_face_state, P_val_state};
int TickFtn_Player(int state);

// enum Winner_state{wait_state, decide_state, display_state};
// int TickFtn_Winner(int state);

int main(void)
{
    // TODO: initialize all your inputs and ouputs

    ADC_init(); // initializes ADC
    lcd_init();
 
    //  Output: DDR = 1, PORT = 0
    //  Input: DDR = 0, PORT = 1
    DDRC = 0b011000; PORTC = 0b100111;
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
    i++;
    tasks[i].state = D_idle_card;
    tasks[i].period = D_Card_Period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_Dealer;
    i++;
    tasks[i].state = T_idle_state;
    tasks[i].period = timer_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_timer;
    i++;
    tasks[i].state = P_idle_state;
    tasks[i].period = P_Card_Period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_Player;
    i++;
    tasks[i].state = T2_idle_state;
    tasks[i].period = timer2_period;
    tasks[i].elapsedTime = tasks[i].period;
    tasks[i].TickFct = &TickFtn_timer2;
    // i++;
    // tasks[i].state = wait_state;
    // tasks[i].period = Winner_Period;
    // tasks[i].elapsedTime = tasks[i].period;
    // tasks[i].TickFct = &TickFtn_Winner;

    TimerSet(GCD_PERIOD);
    TimerOn();

    while (1)
    {
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

// enum bet_state{idle_money, betting_state, loan_state, money_state};
int TickFtn_Bet(int state){
    switch (state)
    {
    case idle_money:
        if (!is_up)
        {
            lcd_clear();
            lcd_write_str("Place Bets");
            state = idle_money;
        }
        else if(is_up){
            lcd_clear();
            player_money = 1000;
            serial_println(player_money);
            player_bet = 0;
            player_loan = 0;
            newGame = 0;
            state = betting_state;
        }
    break;

    case betting_state:
        if (is_up || is_down)
        {
            state = betting_state;
        }
        else if (!((PINC >> 2) & 0x01))
        {
            lcd_clear();
            lcd_write_str("Game Start!");
            // serial_println("moving on to money");
            player_choice = 0;
            game_start = 1;
            state = money_state;
        }
    break;

    case money_state:
        if(player_money > 0){
            // serial_println("were in the money");
            state = money_state;
        }
        else if(newGame){
            // serial_println("going back to betting");
            newGame = 0;
            lcd_clear();
            player_bet = 0;
            state = betting_state;
        }
        else if(player_money <= 0){
            // serial_println("going to get loans");
            lcd_clear();
            lcd_write_str("Up = Loan");
            lcd_goto_xy(1,0);
            lcd_write_str("down = stop");
            state = loan_state;
        }
    break;

    case loan_state:
        if(is_up){
            // serial_println("still getting loans");
            state = loan_state;
        }
        else if(is_down){
            // serial_println("going to idle");
            state = idle_money;
        }
        else if(!((PINC >> 2)&0x01)){
            player_money = player_loan;
            // serial_println("going to money handler");
            state = money_state;
        }
    break;
    
    default:
    break;
    }

    switch (state)
    {
    case idle_money:
    break;

    case betting_state:
        if(is_up)
        {
            // serial_println("still her in betting");
            player_bet = player_bet + 100;
            // serial_println(player_bet);
            lcd_clear();
            lcd_write_str("your total");
            lcd_goto_xy(1, 0);
            lcd_write_str("bet is");
            if (player_bet == 0)
            {
                lcd_write_str(" 0");
            }
            else if (player_bet == 100)
            {
                lcd_write_str(" 100");
            }
            else if (player_bet == 200)
            {
                lcd_write_str(" 200");
            }
            else if (player_bet == 300)
            {
                lcd_write_str(" 300");
            }
            else if (player_bet == 400)
            {
                lcd_write_str(" 400");
            }
            else if (player_bet == 500)
            {
                lcd_write_str(" 500");
            }
            else if (player_bet == 600)
            {
                lcd_write_str(" 600");
            }
            else if (player_bet == 700)
            {
                lcd_write_str(" 700");
            }
            else if (player_bet == 800)
            {
                lcd_write_str(" 800");
            }
            else if (player_bet == 900)
            {
                lcd_write_str(" 900");
            }
            else if (player_bet == 1000)
            {
                lcd_write_str(" 1000");
            }
            else if (player_bet > 1000)
            {
                lcd_clear();
                lcd_write_str("Limit reached");
                lcd_goto_xy(1, 0);
                lcd_write_str("BET = 1000");
                player_bet = 1000;
                player_limit = 1;
            }
        }
        else if(is_down){
            // serial_println("still her in betting");
            player_bet = player_bet - 100;
            // serial_println(player_bet);
            lcd_clear();
            lcd_write_str("your total");
            lcd_goto_xy(1, 0);
            lcd_write_str("bet is");
            if (player_bet == 0)
            {
                lcd_write_str(" 0");
            }
            else if (player_bet == 100)
            {
                lcd_write_str(" 100");
            }
            else if (player_bet == 200)
            {
                lcd_write_str(" 200");
            }
            else if (player_bet == 300)
            {
                lcd_write_str(" 300");
            }
            else if (player_bet == 400)
            {
                lcd_write_str(" 400");
            }
            else if (player_bet == 500)
            {
                lcd_write_str(" 500");
            }
            else if (player_bet == 600)
            {
                lcd_write_str(" 600");
            }
            else if (player_bet == 700)
            {
                lcd_write_str(" 700");
            }
            else if (player_bet == 800)
            {
                lcd_write_str(" 800");
            }
            else if (player_bet == 900)
            {
                lcd_write_str(" 900");
            }
            else if (player_bet == 1000)
            {
                lcd_write_str(" 1000");
            }
            else if (player_bet > 1000)
            {
                lcd_clear();
                lcd_write_str("Limit reached");
                lcd_goto_xy(1, 0);
                lcd_write_str("BET = 1000");
                player_bet = 1000;
                player_limit = 1;
            }
        }
    break;

    case money_state:
        if (player_choice)
        {
            if (player_total > 21)
            {
                player_money = player_money - player_bet;
                player_choice = 0;
                serial_println(player_money);
            }
            else if (dealer_total > 21)
            {
                player_money = player_money + player_bet;
                player_choice = 0;
                serial_println(player_money);
            }
            else if (player_total > dealer_total)
            {
                player_money = player_money + player_bet;
                player_choice = 0;
                serial_println(player_money);
            }
            else if (player_total < dealer_total)
            {
                player_money = player_money - player_bet;
                player_choice = 0;
                serial_println(player_money);
            }
            if (!player_choice)
            {
                newGame = 1;
                serial_println("new game");
                
            }
        }
        
    /////////////////////
        // if(player_loss){
        //     player_money = player_money - player_bet;
        //     serial_println(player_money);
        // }
        // else if(player_win){
        //     player_money = player_money + player_bet;
        //     serial_println(player_money);
        // }
        
    break;

    case loan_state:
        player_loan = player_loan + 100;
        lcd_write_str("Total Loan");
        lcd_goto_xy(1,0);
        if(player_loan == 100){
           lcd_write_str("$100");
        }
        else if( player_loan == 200){
            lcd_write_str("$200");
        }
        else if( player_loan == 300){
            lcd_write_str("$300");
        }
        else if( player_loan == 400){
            lcd_write_str("$400");
        }
        else if( player_loan == 500){
            lcd_write_str("$500");
        }
        else if( player_loan == 600){
            lcd_write_str("$600");
        }
        else if( player_loan == 600){
            lcd_write_str("$600");
        }
        else if( player_loan == 700){
            lcd_write_str("$700");
        }
        else if( player_loan == 800){
            lcd_write_str("$800");
        }
        else if( player_loan == 900){
            lcd_write_str("$900");
        }
        else if (player_loan == 1000)
        {
            lcd_write_str("$1000");
        }
        else if(player_loan> 1000){
            lcd_clear();
            lcd_write_str("Limit reached");
            lcd_goto_xy(1,0);
            lcd_write_str("Loan = 1000");
            player_loan = 1000;
            // player_limit = 1;
        }
    break;

    default:
    break;
    }
    return state;
}

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

// enum card_state{D_idle_card, D_suit_state, D_face_state, D_val_state};
int TickFtn_Dealer(int state){
    switch (state)
    {
    case D_idle_card:
        if(!game_start){
            state = D_idle_card;
        }
        else{
            ii = 1;
            k = 0;
            l = 0;
            m = 0;
            dealer_suit = 0;
            dealer_face = 0;
            needCards = 0;
            dealer_total = 0;
            d_card_total = 0;
            // serial_println("here");
            state = D_suit_state;
        }
    break;

    case D_suit_state:
        if(ii > 0){
            // serial_println("heeree");
            state = D_suit_state;
        }
        else{
            ii = 1;
            state = D_face_state;
        }
    break;
    
    case D_face_state:
        if(ii > 0){
            state = D_face_state;
        }
        else{
            if(dealer_suit == 1){
            // serial_println("clubs");
            dealer_card = "clubs";
            l++;
        }
        else if(dealer_suit == 2){
            // serial_println("hearts");
            dealer_card = "hearts";
            l++;
        }
        else if(dealer_suit == 3){
            // serial_println("spades");
            dealer_card = "spades";
            l++;
        }
        else if(dealer_suit == 4){
            // serial_println("diamonds");
            dealer_card = "diamonds";
            l++;
        }

        if(dealer_face == 1){
            // serial_println("A");
            dealer_cardF = "A";
            dealer_total = dealer_total + 11;
            k++;
        }
        else if(dealer_face == 2){
            // serial_println("2");
            dealer_cardF = "2";
            dealer_total = dealer_total + 2;
            k++;
        }
        else if(dealer_face == 3){
            // serial_println("3");
            dealer_cardF = "3";
            dealer_total = dealer_total + 3;
            k++;
        }
        else if(dealer_face == 4){
            // serial_println("4");
            dealer_cardF = "4";
            dealer_total = dealer_total + 4;
            k++;
        }
        else if(dealer_face == 5){
            // serial_println("5");
            dealer_cardF = "5";
            dealer_total = dealer_total + 5;
            k++;
        }
        else if(dealer_face == 6){
            // serial_println("6");
            dealer_cardF = "6";
            dealer_total = dealer_total + 6;
            k++;
        }
        else if(dealer_face == 7){
            // serial_println("7");
            dealer_cardF = "7";
            dealer_total = dealer_total + 7;
            k++;
        }
        else if(dealer_face == 8){
            // serial_println("8");
            dealer_cardF = "8";
            dealer_total = dealer_total + 8;
            k++;
        }
        else if(dealer_face == 9){
            // serial_println("9");
            dealer_cardF = "9";
            dealer_total = dealer_total + 9;
            k++;
        }
        else if(dealer_face == 10){
            // serial_println("10");
            dealer_cardF = "10";
            dealer_total = dealer_total + 10;
            k++;
        }
        else if(dealer_face == 11){
            // serial_println("J");
            dealer_cardF = "J";
            dealer_total = dealer_total + 10;
            k++;
        }
        else if(dealer_face == 12){
            // serial_println("Q");
            dealer_cardF = "Q";
            dealer_total = dealer_total + 10;
            k++;
        }
        else if(dealer_face == 13){
            // serial_println("K");
            dealer_cardF = "K";
            dealer_total = dealer_total + 10;
            k++;
        }
        // lcd_clear();
        // lcd_write_str(dealer_cardF);
        // lcd_goto_xy(1,0);
        // lcd_write_str(dealer_card);
        // serial_println(dealer_total);
        needCards = 1;
        ii = 0;
        state = D_val_state;
        }
    break;

    case D_val_state:
        if(!player_limit){
            dealer_suit = 0;
            dealer_face = 0;
            ii = 1;
            needCards = 0;
            state = D_suit_state;
        }
        else if(player_limit){
            state = D_val_state;
        }
        else if(!game_start && player_limit){
            needCards = 0;
            state = D_idle_card;
        }
    break;

    default:
        break;
    }

    switch (state)
    {
    case D_idle_card:
    break;

    case D_suit_state:
        dealer_suit = rand()%t + 1;
        // serial_println(dealer_suit);
        ii--;
    break;
    
    case D_face_state:
        dealer_face = rand()%tt + 1;
        // serial_println(dealer_face);
        d_card_total++;
        ii--;
    break;

    case D_val_state:
        if(dealer_total <= 17){
            player_limit = 0;
        }
        else{
            player_limit = 1;
        }
        
    break;

    default:
        break;
    }
    return state;
}

// enum time_state{T_idle_state, timer_state};
int TickFtn_timer(int state){
    switch (state)
    {
    case T_idle_state:
        if(is_up){
            t = 1;
            tt = 1;
            state = timer_state;
        }
        else{
        state = T_idle_state;
        }
    break;
    
    case timer_state:
        if(newGame){
            state = T_idle_state;
        }
        else{
            state = timer_state;
        }

    default:
        break;
    }

    switch (state)
    {
    case T_idle_state:
    break;
    
    case timer_state:
        if(t <= 4){
            t++;
        }
        else if(t > 4){
            t = 1;
        }
        if(tt <= 13){
            tt++;
        }
        else if(tt > 13){
            tt = 1;
        }
    default:
        break;
    }
    return state;
}

// enum Pcard_state{P_idle_state, P_suit_state, P_face_state, P_val_state};
int TickFtn_Player(int state){
    switch (state)
    {
    case P_idle_state:
        if(!game_start){
            state = P_idle_state;
        }
        else{
            hit_me = 0;
            ii2 = 1;
            k2 = 0;
            l2 = 0;
            m2 = 0;
            player_suit = 0;
            player_face = 0;
            needCards2 = 0;
            player_total = 0;
            p_card_total = 0;
            //player_choice = 0;
            // serial_println("here");
            state = P_suit_state;
        }
    break;

    case P_suit_state:
        if(ii2 > 0){
            // serial_println("heeree");
            state = P_suit_state;
        }
        else{
            ii2 = 1;
            state = P_face_state;
        }
    break;
    
    case P_face_state:
        if(ii2 > 0){
            state = P_face_state;
        }
        else{
            if(player_suit == 1){
            serial_println("clubs");
            player_card = "clubs";
            l2++;
        }
        else if(player_suit == 2){
            serial_println("hearts");
            player_card = "hearts";
            l2++;
        }
        else if(player_suit == 3){
            serial_println("spades");
            player_card = "spades";
            l2++;
        }
        else if(player_suit == 4){
            serial_println("diamonds");
            player_card = "diamonds";
            l2++;
        }

        if(player_face == 1){
            serial_println("A");
            player_cardF = "A";
            player_total = player_total + 11;
            k2++;
        }
        else if(player_face == 2){
            serial_println("2");
            player_cardF = "2";
            player_total = player_total + 2;
            k2++;
        }
        else if(player_face == 3){
            serial_println("3");
            player_cardF = "3";
            player_total = player_total + 3;
            k2++;
        }
        else if(player_face == 4){
            serial_println("4");
            player_cardF = "4";
            player_total = player_total + 4;
            k2++;
        }
        else if(player_face == 5){
            serial_println("5");
            player_cardF = "5";
            player_total = player_total + 5;
            k2++;
        }
        else if(player_face == 6){
            serial_println("6");
            player_cardF = "6";
            player_total = player_total + 6;
            k2++;
        }
        else if(player_face == 7){
            serial_println("7");
            player_cardF = "7";
            player_total = player_total + 7;
            k2++;
        }
        else if(player_face == 8){
            serial_println("8");
            player_cardF = "8";
            player_total = player_total + 8;
            k2++;
        }
        else if(player_face == 9){
            serial_println("9");
            player_cardF = "9";
            player_total = player_total + 9;
            k2++;
        }
        else if(player_face == 10){
            serial_println("10");
            player_cardF = "10";
            player_total = player_total + 10;
            k2++;
        }
        else if(player_face == 11){
            serial_println("J");
            player_cardF = "J";
            player_total = player_total + 10;
            k2++;
        }
        else if(player_face == 12){
            serial_println("Q");
            player_cardF = "Q";
            player_total = player_total + 10;
            k2++;
        }
        else if(player_face == 13){
            serial_println("K");
            player_cardF = "K";
            player_total = player_total + 10;
            k2++;
        }
        lcd_clear();
        lcd_write_str(player_cardF);
        lcd_goto_xy(0, 7);
        lcd_goto_xy(1,0);
        lcd_write_str(player_card);
        serial_println(player_total);
        needCards2 = 1;
        ii2 = 0;
        state = P_val_state;
        }
    break;

    case P_val_state:
        if(hit_me){
            player_suit = 0;
            player_face = 0;
            ii2 = 1;
            needCards2 = 0;
            hit_me = 0;
            state = P_suit_state;
        }
        else if(!hit_me){
            state = P_val_state;
        }
        else if(!game_start && !hit_me){
            needCards2 = 0;
            state = P_idle_state;
        }
    break;

    default:
        break;
    }

    switch (state)
    {
    case P_idle_state:
    break;

    case P_suit_state:
        player_suit = rand()%t2 + 1;
        serial_println(player_suit);
        ii2--;
    break;
    
    case P_face_state:
        player_face = rand()%tt2 + 1;
        serial_println(player_face);
        p_card_total++;
        ii2--;
    break;

    case P_val_state:
        if(!((PINC >> 2) & 0x01)){
            hit_me = 1;
        }
        else if(is_down){
            player_choice = 1;
        }
        else{
            hit_me = 0;
        }
        
    break;

    default:
        break;
    }
    return state;
}

// enum time_state2{T2_idle_state, timer2_state};
int TickFtn_timer2(int state){
    switch (state)
    {
    case T2_idle_state:
        if(is_up){
            t2 = 1;
            tt2 = 1;
            state = timer2_state;
        }
        else{
        state = T2_idle_state;
        }
    break;
    
    case timer2_state:
        if(newGame){
            state = T2_idle_state;
        }
        else{
            state = timer2_state;
        }

    default:
        break;
    }

    switch (state)
    {
    case T2_idle_state:
    break;
    
    case timer2_state:
        if(t2 <= 4){
            t2++;
        }
        else if(t2 > 4){
            t2 = 1;
        }
        if(tt2 <= 13){
            tt2++;
        }
        else if(tt2 > 13){
            tt2 = 1;
        }
    default:
        break;
    }
    return state;
}

// // enum Winner_state{wait_state, decide_state, display_state};
// int TickFtn_Winner(int state){
//     switch (state)
//     {
//     case wait_state:
//         if(player_choice){
//             player_win = 0;
//             player_loss = 0;
//             state = decide_state;
//         }
//         else{
//             state = wait_state;
//         }
//     break;

//     case decide_state:
//         state = display_state;
//     break;

//     case display_state:
//         state = wait_state;
//     break;
    
//     default:
//         break;
//     }
//     switch (state)
//     {
//     case wait_state:
//     break;

//     case decide_state:
//         if (player_total > 21)
//         {
//             player_loss = 1;
//         }
//         else if(dealer_total > 21){
//             player_win = 1;
//         }
//         else if(player_total > dealer_total){
//             player_win = 1;
//         }
//         else if(player_total < dealer_total){
//             player_loss = 1;
//         }
//     break;

//     case display_state:
//         // if(player_win){
//         //     lcd_clear();
//         //     lcd_write_str("You win!");
//         // }
//         // else if(player_loss){
//         //     lcd_clear();
//         //     lcd_write_str("YOU LOSE HAHAHA");
//         // }
//     break;
    
//     default:
//         break;
//     }
//     return state;
// }