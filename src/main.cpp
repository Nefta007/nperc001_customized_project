
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

#define NUM_TASKS 4 // TODO: Change to the number of tasks being used
// main background song
int I_Want_Billions[45] = {C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, E5_Flat, F5_Sharp, C5_Sharp, C5_Sharp, B4_Flat, A4_Flat, A4_Flat, G4_Sharp, F4_Sharp, F4_Sharp, F4_Sharp, A4_Flat, C5_Sharp, B4_Flat, A4_Flat, F4_Sharp, B4_Flat, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, C5_Sharp, E5_Flat, F5_Sharp, C5_Sharp, C5_Sharp, B4_Flat, A4_Flat, A4_Flat, G4_Sharp, F4_Sharp, F4_Sharp, F4_Sharp, A4_Flat, C5_Sharp, B4_Flat, A4_Flat, F4_Sharp, B4_Flat};
int I_want_Time[45] = {1, 1, 1, 1, 1, 2, 1, 5, 4, 2, 4, 2, 4, 3, 3, 3, 3, 2, 2, 2, 2, 8, 1, 1, 1, 1, 1, 1, 2, 1, 2, 4, 1, 2, 1, 4, 2, 2, 2, 2, 2, 2,2 ,2 ,6};

// second song if user borrows money
int Run_Away_Baby[41] = {A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, D4_Sharp, D4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, A4_Sharp, G4_Sharp, D4_Sharp, F4_Sharp, G4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, D4_Sharp, D4_Sharp, A4_Sharp, A4_Sharp, A4_Sharp, G4_Sharp, F4_Sharp, A4_Sharp, G4_Sharp};
int Run_Away_Time[41] = {2, 2, 1, 1, 2, 1, 2, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 4, 1, 1, 1, 2, 2, 1, 1, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4};


int suits[4] = {1,2,3,4};
int card_values[13] = {1,2,3,4,5,6,7,8,9,10,11,12,13};

char *D_Card_Suits[4] = {"Clubs","Hearts","Spades","Diamonds"};
char *D_CARD_Face[13] = {"1","2","3","4","5","6","7","8","9","10","J","Q","K"};
int Dealer_vals[11] = {0,0,0,0,0,0,0,0,0,0,0};

int value;
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
unsigned char t;
unsigned char display_value;
unsigned char dealer_total;
unsigned char d_card_total;
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
unsigned char is_bet;
unsigned char newGame;
unsigned char game_start;
unsigned char player_limit;
int temp_bet;
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
const unsigned long Background_Period = 200;
const unsigned long Bet_Period = 1000;

const unsigned long Direction_Period = 500;
const unsigned long GCD_PERIOD = findGCD(JS_Period, D_Card_Period); // TODO:Set the GCD Period

task tasks[NUM_TASKS]; // declared task array with 5 tasks

// void HardwareReset()
// {
//     PORTD = SetBit(PORTD, 4, 0);// setResetPinToLow;
//     _delay_ms(200);
//     PORTD = SetBit(PORTD, 4, 1);// setResetPinToHigh;
//     _delay_ms(200);
// }

// void ST7735_init()
// {
//     HardwareReset();
//     PORTB = SetBit(PORTB, 2, 0); //cs
//     PORTD = SetBit(PORTD, 5, 0); //a0
//     SPI_SEND(0x01);// Send_Command(SWRESET);
//     _delay_ms(150);
//     SPI_SEND(0x11);// Send_Command(SLPOUT);
//     _delay_ms(200);
//     SPI_SEND(0x3A);// Send_Command(COLMOD);
//     PORTD = SetBit(PORTD, 5, 1); //a0
//     SPI_SEND(0x06);// Send_Data(0x06); // for 18 bit color mode. You can pick any color mode you want
//     _delay_ms(10);
//     // Send_Command(DISPON);
//     PORTD = SetBit(PORTD, 5, 0); //a0
//     SPI_SEND(0x29);// Send_Command(DISPON);
//     _delay_ms(200);
//     //set x lines
//     SPI_SEND(0x2A);
//     PORTD = SetBit(PORTD, 5, 1); //a0
//     _delay_ms(200);
//     SPI_SEND(0x00);
//     SPI_SEND(0x11);
//     _delay_ms(200);
//     SPI_SEND(0x00);
//     SPI_SEND(0xFF);
//     _delay_ms(200);
//     PORTD = SetBit(PORTD, 5, 0); //a0
//     SPI_SEND(0x2C);
//     _delay_ms(200);
//     PORTD = SetBit(PORTD, 5, 1); //a0

// }

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
    // SPI_INIT();
    // ST7735_init();

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
            player_bet = 0;
            player_loan = 0;
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
            serial_println("moving on to money");
            game_start = 1;
            state = money_state;
        }
    break;

    case money_state:
        if(!newGame && player_money > 0){
            serial_println("were in the money");
            state = money_state;
        }
        else if(newGame && player_money > 0){
            serial_println("going back to betting");
            state = betting_state;
        }
        else if(player_money <= 0){
            serial_println("going to get loans");
            lcd_clear();
            lcd_write_str("Up = Loan");
            lcd_goto_xy(1,0);
            lcd_write_str("down = stop");
            state = loan_state;
        }
    break;

    case loan_state:
        if(is_up){
            serial_println("still getting loans");
            state = loan_state;
        }
        else if(is_down){
            serial_println("going to idle");
            state = idle_money;
        }
        else if(!((PINC >> 2)&0x01)){
            player_money = player_loan;
            serial_println("going to money handler");
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
            serial_println("still her in betting");
            player_bet = player_bet + 100;
            serial_println(player_bet);
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
            serial_println("still her in betting");
            player_bet = player_bet - 100;
            serial_println(player_bet);
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
        if(player_loss){
            player_money = player_money - player_bet;
        }
        else if(player_win){
            player_money = player_money + player_bet;
        }
        else if(!((PINC >> 2)&0x01)){
            newGame = 1;
        }
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
            lcd_clear;
            lcd_write_str("Limit reached");
            lcd_goto_xy(1,0);
            lcd_write_str("Loan = 1000");
            player_loan = 1000;
            // player_limit = 1;
        }
    break;

    // case L_check_state:
        // lcd_write_str("Total Loan");
        // lcd_goto_xy(1,0);
        // // if(player_loan == 100){
        // //    lcd_write_str("$100");
        // // }
        // // else if( player_loan == 200){
        // //     lcd_write_str("$200");
        // // }
        // // else if( player_loan == 300){
        // //     lcd_write_str("$300");
        // // }
        // // else if( player_loan == 400){
        // //     lcd_write_str("$400");
        // // }
        // // else if( player_loan == 500){
        // //     lcd_write_str("$500");
        // // }
        // // else if( player_loan == 600){
        // //     lcd_write_str("$600");
        // // }
        // // else if( player_loan == 600){
        // //     lcd_write_str("$600");
        // // }
        // // else if( player_loan == 700){
        // //     lcd_write_str("$700");
        // // }
        // // else if( player_loan == 800){
        // //     lcd_write_str("$800");
        // // }
        // // else if( player_loan == 900){
        // //     lcd_write_str("$900");
        // // }
        // // else if (player_loan == 1000)
        // // {
        // //     lcd_write_str("$1000");
        // // }
        // // else if(player_loan> 1000){
        // //     lcd_clear;
        // //     lcd_write_str("Limit reached");
        // //     lcd_goto_xy(1,0);
        // //     lcd_write_str("Loan = 1000");
        // //     player_loan = 1000;
        // //     player_limit = 1;
        // // }
    //     t++;
    // break;
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

// // enum card_state{D_idle_card, D_suit_state, D_face_state, D_val_state};
// int TickFtn_Dealer(int state){
//     switch (state)
//     {
//     case D_idle_card:
//         if(!is_bet){
//             state = D_idle_card;
//         }
//         else if(is_bet){
//             dealer_suit = suits[rand()%4];
//             serial_println(dealer_suit);
//             serial_println("first area");
//             ii = 2;
//             jj = 0;
//             k = 0;
//             l = 0;
//             m = 0;
//             dealer_total = 0;
//             d_card_total = 0;
//             display_value = 0;
//             state = D_suit_state;
//         }
//         // else{
//         //     state = D_idle_card;
//         // }
//     break;
    
//     case D_suit_state:
//         if(ii > 0){
//             state = D_suit_state;
//         }
//         else if(ii <= 0){
//             dealer_face = card_values[rand()%13];
//             serial_println(dealer_face);
//             state = D_face_state;
//         }
//     break;

//     case D_face_state:
//         if(l > 0){
//             state = D_face_state;
//         }
//         else if(l <= 0 && dealer_total <= 17){
//             ii = 0;
//             state = D_suit_state;
//         }
//         else if(dealer_total > 17){
//             ii = 0;
//             state = D_val_state;
//         }
//     break;

//     case D_val_state:
//         if(d_card_total > 0){
//             state = D_val_state;
//         }
//         else if(!is_bet){
//             state = D_idle_card;
//         }
//     break;
//     default:
//         break;
//     }
    
//     switch (state)
//     {
//     case D_idle_card:
//     break;
    
//     case D_suit_state:
//         if(dealer_suit == 1){
//             D_Card_Suits[k] = "clubs";
//             serial_println(D_Card_Suits[k]);
//             serial_println("second area");
//             k++;
//             l++;
//             ii--;
//         }
//         else if(dealer_suit == 2){
//             D_Card_Suits[k] = "hearts";
//             serial_println('hearts');
//             serial_println("third area");
//             k++;
//             l++;
//             ii--;
//         }
//         else if(dealer_suit == 3){
//             D_Card_Suits[k] = "spades";
//             serial_println('spades');
//             serial_println("fourth area");
//             k++;
//             l++;
//             ii--;
//         }
//         else if(dealer_suit == 4){
//             //dealer_cardF = "diamond";
//             D_Card_Suits[k] = dealer_cardF;
//             serial_println('diamonds');
//             serial_println("fifth area");
//             k++;
//             l++;
//             ii--;
//         }
//     break;

//     case D_face_state:
//         if(dealer_face == 1){
//             D_CARD_Face[m] = "A";
//             display_value = 11;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 2){
//             D_CARD_Face[m] = "2";
//             display_value = 2;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 3){
//             D_CARD_Face[m] = "3";
//             display_value = 3;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 4){
//             D_CARD_Face[m] = "4";
//             display_value = 4;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 5){
//             D_CARD_Face[m] = "5";
//             display_value = 5;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 6){
//             D_CARD_Face[m] = "6";
//             display_value = 6;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 7){
//             D_CARD_Face[m] = "7";
//             display_value = 7;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 8){
//             D_CARD_Face[m] = "8";
//             display_value = 8;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 9){
//             D_CARD_Face[m] = "9";
//             display_value = 9;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 10){
//             D_CARD_Face[m] = "10";
//             display_value = 10;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 11){
//             D_CARD_Face[m] = "J";
//             display_value = 10;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 12){
//             D_CARD_Face[m] = "Q";
//             display_value = 10;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//         else if(dealer_face == 13){
//             D_CARD_Face[m] = "K";
//             display_value = 10;
//             dealer_total = dealer_total + display_value;
//             d_card_total++;
//             m++;
//             l--;
//         }
//     break;

//     case D_val_state:
//         // dealer_card = D_CARD_Face[i];
//        //for(i = 0; i < d_card_total; i++){
//         serial_println(D_CARD_Face[ii]);
//         serial_println("sixth area");
//         ii++;
//         d_card_total--;
//         if(ii >= 3){
//             is_bet = 0;
//         }
//         //dealer_cardF = D_CARD_Face[i];
//         //serial_println(dealer_cardF);
//        //}
//         // serial_println
//     break;
//     default:
//         break;
//     }
//     return state;
// }
