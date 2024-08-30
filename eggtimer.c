/*
Atmega 328p  |   Uno Board   |  4-digit 7 seg LED
-------------------------------------------------
PD2                 D2                  A
PD3                 D3                  B
PD4                 D4                  C
PD5                 D5                  D
PD6                 D6                  E
PB0                 D8                  F
PC4                 A4                  G
PB2                 D10                 D1
PB3                 D11                 D2
PB4                 D12                 D3
PB5                 D13                 D4
PC0                 A0                  DECIMAL POINT
-------------------------------------------------
PC1 (internnaly pull-up input) --> SET BUTTON
PC2 (internnaly pull-up input)--> START/STOP BUTTON
PC3 (analog input) --> potentiometer
 */

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "pitches.h"

#define A PD2
#define B PD3
#define C PD4
#define D PD5
#define E PD6
#define F PB0
#define G PC4
#define D1 PB2
#define D2 PB3
#define D3 PB4
#define D4 PB5

//MORSE CODE FOR SOS (emergency)
#define DOT_DURATION 6    // Duration for a dot
#define DASH_DURATION 3 // Duration for a dash
#define PAUSE_DURATION 6  // Duration for pauses between elements

volatile uint16_t state_timer = 0;//track time when in set state
volatile uint16_t start_stop_timer = 0;//track time when start/stop state
volatile uint16_t button_press_duration = 0;//used to track time when holding set button
volatile int start_stop_state = 0;//tracks start or stop state
volatile int set_state = 0;//tracks set state (for example set_state = 1 sets MINUTES and set_state = 2 sets SECONDS)

void SevenSegment(int x){
    PORTD = 0;//clear LEDS
    PORTB = 0;//clear LEDS
    PORTC &= ~(1<<G);//clear LED
    switch (x){
        case 0: //to display 0, LED segments A,B,C,D,E,F MUST be ON
            PORTB = (1<<F);//LED Segment F ON
            PORTD = 0xFF; //LED Segment A,B,C,D,E ON
            break;

        case 1: //to display 1, LED segments B,C MUST be ON
            PORTD = (1<<B) | (1<<C) ;//LED Segment B,C ON
            break;

        case 2://to display 2, LED segments A,B,D,E,G MUST be ON
            PORTC |= (1<<G);//LED Segment G ON
            PORTD |= ~(1<<C);//turn ON all LED segments connected to portD exacpt segment C
            break;

        case 3://to display 3, LED segments A,B,C,D,G MUST be ON
            PORTC |= (1<<G);//LED Segment G ON
            PORTD |= ~(1<<E);//turn ON all LED segments connected to portD exacpt segment E
            break;

        case 4://to display 4, LED segments B,C,G,F MUST be ON
            PORTB = (1<<F);//LED Segment G,F ON
            PORTC |= (1<<G);
            PORTD = (1<<B) | (1<<C);//LED Segment B,C ON
            break;

        case 5://to display 5, LED segments A,C,D,G,F MUST be ON
            PORTB = (1<<F);//LED Segment G,F ON
            PORTC |= (1<<G);
            PORTD |= ~((1<<B) | (1<<E));//turn ON all LED segments connected to portD exacpt segment B,E
            break;

        case 6://to display 6, LED segments A,C,D,E,G,F MUST be ON
            PORTB = (1<<F);//LED Segment G,F ON
            PORTC |= (1<<G);
            PORTD |= ~(1<<B);//turn ON all LED segments connected to portD exacpt segment B
            break;

        case 7://to display 7, LED segments A,B,C MUST be ON
            PORTD |= ~((1<<E) | (1<<D));//turn ON all LED segments connected to portD exacpt segment D,E
            break;

        case 8://to display 8, LED segments ALL segments MUST be ON
            PORTB = (1<<F);//LED Segment G,F ON
            PORTC |= (1<<G);
            PORTD =0xFF;//LED Segment A,B,C,D,E ON
            break;

        case 9://to display 9, LED segments A,B,C,G,F MUST be ON
            PORTB = (1<<F);//LED Segment G,F ON
            PORTC |= (1<<G);
            PORTD |= ~((1<<E) | (1<<D));//turn ON all LED segments connected to portD exacpt segment D,E
            break;
        default:
            break;
    }

}

void init_Timer0() {
    // Set Timer0 to CTC mode
    TCCR0A |= (1 << WGM01);
    // Set the value for the compare match register for 1 ms interval
    OCR0A = 249;
    // Enable the Timer0 compare interrupt
    TIMSK0 |= (1 << OCIE0A);
    // Start Timer0 with a prescaler of 64
    TCCR0B |= (1 << CS01) | (1 << CS00);
    sei();
}

ISR(TIMER0_COMPA_vect) {
    
    //if set button is pressed, increment time or track time
    if (!(PINC & (1 << PC1))) 
        button_press_duration++;
    else 
        button_press_duration = 0; // Reset duration if button is not pressed
    
    //track time when setting minutes and seconds
    if (set_state == 1 || set_state == 2){
        state_timer++;
        }
    else 
        state_timer = 0;
    
    //track time when starting or stopping timer countdwn
    if (start_stop_state == 1)
        start_stop_timer++;
    else
        start_stop_timer = 0;

}

//Timer 1 initialization
void initTimer() {
    // Set Timer1 to Fast PWM mode with ICR1 as TOP
    TCCR1A = (1 << COM1A1) | (1 << WGM11); // Clear OC1A on match, Fast PWM mode
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11); // Fast PWM, ICR1 as TOP, 1/8 prescaler
    DDRB |= (1 << PB1);
}

void tone(uint16_t frequency) {
    DDRB |= (1 << PB1);
    // Calculate TOP value for the desired frequency
    // Frequency = (Clock Speed / (2 * Prescaler * TOP))
    // TOP = (Clock Speed / (2 * Prescaler * Frequency))
    // Assuming 16 MHz clock and prescaler of 8
    uint16_t top_value = (uint16_t)((16000000UL / (2 * 8 * frequency)) - 1);
    
    // Set ICR1 as TOP value
    ICR1 = top_value;
    
    // Set OCR1A to 50% duty cycle
    OCR1A = ICR1 / 2;
}

void mute(void) {
    DDRB &= ~(1 << PB1); // Set OC1A (PB1) as input
    PORTB &= ~(1 << PB1); // Ensure the pin is low 
}

//maps ADC values into minutes (0-99)
int map_minutes(uint16_t value){
    return (uint32_t)value * 99 / 1023;
    }

//maps ADC values into seconds (0-59)
int map_seconds(uint16_t value){
    return (uint32_t)value * 59 / 1023;
    }


void parseCount(int count,int digits[]){
   /*
   This for loop iterates over each digit position from least significant
   to most significant (units, tens, hundreds, thousands). Numbers with
   less than two digits have leading zeros stored in their higher-order
   positions.
    */
      for (int i = 1; i >= 0; i--) {
        digits[i] = count % 10;
        count /= 10;
    }
}

void initADC(void) {
    ADCSRA |= (1 << ADEN); //enable ADC
    ADMUX |= (1 << REFS0); // Vref connect internally to AVcc
    ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);//Set prescale to 128 (16000000/128 = 125Khz) 
    ADCSRA |= (1 << ADSC);// start conversion 1st time will take 25 cycles
}

int analog( uint8_t channel ) {
    ADMUX  &=  0xF0; //Clearing the last 4 bits of ADMUX
    ADMUX  |=  channel; //Selecting channel
    ADCSRA |= (1 << ADSC); //Start conversion process
    while ( ADCSRA & (1 << ADSC ) ); //wait for complete conversion and return the result
    return ADC;
}

//displays minutes and seconds on 4-digit seven segment
void display(int digits_minutes[],int digits_seconds[]){
    SevenSegment(digits_seconds[1]);
    PORTB = (PORTB & ~(1<<D4)) | (1<<D1) | (1<<D2) | (1<<D3);//this ensures D4 is LOW and D1,D2,D3 is high
    PORTC &= ~(1<<PC0);
    _delay_ms(5);
    SevenSegment(digits_seconds[0]);
    PORTB = (PORTB & ~(1<<D3)) | (1<<D1) | (1<<D2) | (1<<D4);//this ensures D3 is LOW and D1,D2,D4 is high
    PORTC &= ~(1<<PC0);
    _delay_ms(5);
    SevenSegment(digits_minutes[1]);
    PORTB = (PORTB & ~(1<<D2)) | (1<<D1) | (1<<D3) | (1<<D4);//this ensures D2 is LOW and D1,D3,D4 is high
    PORTC |= (1<<PC0);
    _delay_ms(5);
    SevenSegment(digits_minutes[0]);
    PORTB = (PORTB & ~(1<<D1)) | (1<<D2) | (1<<D3) | (1<<D4);//this ensures D1 is LOW and D2,D3,D4 is high
    PORTC &= ~(1<<PC0);
    _delay_ms(5);
}

//every time this function is called it reduces timer by 1 
void countdown(int digits_minutes[],int digits_seconds[]) {
    if (digits_seconds[0] == 0 && digits_seconds[1] == 0) {
        if (digits_minutes[0] == 0 && digits_minutes[1] == 0) {
            // Countdown complete
            return;
        } else {
            if (digits_minutes[1] == 0) {
                digits_minutes[1] = 9;
                digits_minutes[0]--;
            } else {
                digits_minutes[1]--;
            }
            digits_seconds[0] = 5;
            digits_seconds[1] = 9;
        }
    } else {
        if (digits_seconds[1] == 0) {
            digits_seconds[1] = 9;
            digits_seconds[0]--;
        } else {
            digits_seconds[1]--;
        }
    }
}

void delay_ms (uint16_t ms) {
	uint16_t i;
	for (i = 0; i < ms; i++)
		_delay_ms(1);		
}


int main(void)
{
    DDRD |= ~((1<<DDD0) | (1<<DDD1));//assign pins 2 to 8 in port D as output
    DDRB |= ~((1<<DDB6) | (1<<DDB7) );//assign pins 0 to 5 in port B as output
    DDRC |= (1 << DDC0) | (1<<DDC4);//assign pins 1 and 4 in port C as output
    PORTC |= (1<<PC1) | (1<<PC2);//PULL-UP
    initADC();//ADC initialization
    init_Timer0();//TIMER0 initialization
    initTimer();//TIMER 1 initialization
    int ADC_value;//store adc values
    int count = 0;//used for debounce
    int digits_minutes[2];//store timer settings for minutes
    int digits_seconds[2];//store  timer settings for seconds

    //Notes for SOS in morse code
    uint16_t morse_melody[] = { 
    NOTE_E4, NOTE_E4, NOTE_E4, // S: "..."
    0, // Short pause between characters
    NOTE_G4, NOTE_G4, NOTE_G4, // O: "---"
    0, // Short pause between characters
    NOTE_E4, NOTE_E4, NOTE_E4  // S: "..."
    };

	uint16_t morse_durations[] = { 
    DOT_DURATION, DOT_DURATION, DOT_DURATION, // S
    PAUSE_DURATION, // Short pause between characters
    DASH_DURATION, DASH_DURATION, DASH_DURATION, // O
    PAUSE_DURATION, // Short pause between characters
    DOT_DURATION, DOT_DURATION, DOT_DURATION// S
    };

    int i = 0 ;
    int initial_digits_minutes[2];//store initial timer settings for minutes
    int initial_digits_seconds[2];//store initial timer settings for seconds
    
    while(1){

        //if set button held for longer than 3 seconds, go to set_state = 1 (setting minutes)
        if (button_press_duration >= 3000){
             set_state = 1;
         }

        //This while loop sets minutes
         while (set_state == 1){
            ADC_value = analog(3);//read from potentiometer
            ADC_value = map_minutes(ADC_value);//map minutes from 0-1023 to 0-99
            parseCount(ADC_value,digits_minutes);
            SevenSegment(digits_minutes[1]);
            PORTB = (PORTB &  ~(1<<D2)) | (1<<D1) | (1<<D3) | (1<<D4);//this ensures D2 is LOW and D1,D3,D4 is high   
            _delay_ms(5);
            SevenSegment(digits_minutes[0]);
            PORTB = (PORTB &  ~(1<<D1)) | (1<<D2) | (1<<D3) | (1<<D4);//this ensures D1 is LOW and D1,D3,D4 is high
            _delay_ms(5);
            //every half a second it turns off for a 100ms, making it appear to flash
            if (state_timer > 500){
                PORTB = PORTB | (1<<D2) | (1<<D1) | (1<<D3) | (1<<D4);
                state_timer = 0;//reset timer
                _delay_ms(100);
            }
            //this ensures that if button is held for longer than 3 seconds it does NOT go to next state, should be released first
            if (PINC & (1 << PC1))
                count++;
            //if button is pressed again after inital hold and release, Go to next state
            if (!(PINC & (1 << PC1)) && count > 0){
                set_state = 2;//move to next state -- setting seconds
                count = 0;//reset
            }
        }

        //This while loop sets minutes
        while(set_state == 2){
            ADC_value = analog(3);//read from potentiometer
            ADC_value = map_seconds(ADC_value);//map seconds from 0-1023 to 0-59
            parseCount(ADC_value,digits_seconds);
            SevenSegment(digits_seconds[1]);
            PORTB = (PORTB &  ~(1<<D4)) | (1<<D1) | (1<<D3) | (1<<D2);//this ensures D4 is LOW and D1,D3,D4 is high   
            _delay_ms(5);
            SevenSegment(digits_seconds[0]);
            PORTB = (PORTB &  ~(1<<D3)) | (1<<D2) | (1<<D4) | (1<<D1);//this ensures D3 is LOW and D1,D3,D4 is high
            _delay_ms(5);
            //every half a second it turns off for a 100ms, making it appear to flash
            if (state_timer > 500){
                PORTB = PORTB | (1<<D2) | (1<<D1) | (1<<D3) | (1<<D4);
                state_timer = 0;
                _delay_ms(100);
            }
            //this ensures that if button is held for longer than 3 seconds it does NOT go to next state, should be released first
            if (PINC & (1 << PC1))
                count++;

            //if button is pressed again after inital hold and release, Go to next state
            if (!(PINC & (1 << PC1)) && count > 0){
                set_state = 3;//set_state 3 is used to store orginial TIMER values
                count = 0;
                PORTB = PORTB | (1<<D2) | (1<<D1) | (1<<D3) | (1<<D4);//turn off all leds
                
            }
        }  

        //store orginial TIMER values
        if (set_state == 3){
            initial_digits_minutes[0] = digits_minutes[0];
            initial_digits_minutes[1] = digits_minutes[1];
            initial_digits_seconds[0] = digits_seconds[0];
            initial_digits_seconds[1] = digits_seconds[1];
            set_state = 4;//random state, not used. could any number other than 0,1,2,3
        }

        //if start/stop pressed, goes to next state, where timer countdown begins
        if (!(PINC & (1 << PC2))){ // Button is pressed
        PORTB = PORTB | (1<<D2) | (1<<D1) | (1<<D3) | (1<<D4);//turn off all leds
        _delay_ms(200);//for debounce
        start_stop_state = 1;
        }

        //this while loop starts timer countdown
        while (start_stop_state == 1){
            
            //every second it countsdown a second
            if (start_stop_timer >= 1000){
               start_stop_timer = 0;//reset timer every second to avoid overflow
               countdown(digits_minutes,digits_seconds);  
            } 
            display(digits_minutes,digits_seconds);//display seven segment wit updated values
            
            //if start/stop is pressed, go to next state, where the timer is paused
            if (!(PINC & (1 << PC2))){
                PORTB = PORTB | (1<<D2) | (1<<D1) | (1<<D3) | (1<<D4);//turn off all leds
                _delay_ms(200);//for debounce
                start_stop_state = 2;
                }
            
            //when timer is reaches 0 minutes 0 seconds (00.00) , it plays the audio forever unless timer is reset
            while(digits_seconds[0] == 0 && digits_seconds[1] == 0 && digits_minutes[0] == 0 && digits_minutes[1] == 0) {
                
                PORTB = PORTB | (1<<D2) | (1<<D1) | (1<<D3) | (1<<D4);//turn off all leds
                for (int thisNote = 0; thisNote < 11; thisNote++) {
		        // to calculate the note duration, take one second 
		        // divided by the note type.
		        //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
		        uint16_t noteDuration = 1000/morse_durations[thisNote];
		        tone(morse_melody[thisNote]);
		        delay_ms(noteDuration);
		        // to distinguish the notes, set a minimum time between them.
		        // the note's duration + 30% seems to work well:
                mute();
		        uint16_t pauseBetweenNotes = noteDuration * 3/10;
		        delay_ms(pauseBetweenNotes);
		        // stop the tone playing:


                //if start/stop button is pressed, goes to next state where it resets timer and exits the loop, thus audio is no longer playing
                if (!(PINC & (1 << PC2))){
                    _delay_ms(300);
                    //restore orginial timer settings
                    digits_minutes[0] = initial_digits_minutes[0]; 
                    digits_minutes[1] = initial_digits_minutes[1];
                    digits_seconds[0] = initial_digits_seconds[0];
                    digits_seconds[1] = initial_digits_seconds[1];
                    start_stop_timer = 0;
                    start_stop_state = 2;
                    break;
	            }
                }

                
            }
        }
        
        //dislpays orginial timer settings and waits until start/stop button is pressed to begin timer countdown
        while (start_stop_state == 2){
            display(digits_minutes,digits_seconds);
            if (!(PINC & (1 << PC2))){
                PORTB = PORTB | (1<<D2) | (1<<D1) | (1<<D3) | (1<<D4);//turn off all leds
                _delay_ms(200);
                start_stop_state = 1;
                
                }
                
        }     
        
}
return 0;
}