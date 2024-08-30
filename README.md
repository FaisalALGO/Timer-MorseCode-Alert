# Timer-MorseCode-Alert
This project implements a countdown timer using an ATmega328P microcontroller (Arduino Uno) and a 4-digit 7-segment LED display. The timer can be configured to set minutes and seconds through a potentiometer, with the ability to start, stop, and reset the countdown via buttons. Additionally, it features an emergency "SOS" signal using Morse code tones.

## Features

- **Countdown Timer**: Set minutes (0-99) and seconds (0-59) via potentiometer.
- **Start/Stop Functionality**: Start or stop the countdown with the push of a button.
- **Reset Functionality**: Reset the countdown timer to initial settings.
- **7-Segment Display Control**: Timer values are displayed on a 4-digit 7-segment LED.
- **Morse Code SOS Alert**: Plays SOS in Morse code when the countdown reaches zero.
- **Configurable Buttons**:
  - Set Button for entering the minutes and seconds.
  - Start/Stop Button for controlling the countdown.

## Components

- ATmega328P (Arduino Uno)
- 4-Digit 7-Segment LED Display (common cathode)
- Potentiometer (for setting timer values)
- Buttons for Start/Stop and Set
- Buzzer for SOS Morse Code signal

## Pin Configuration

| ATmega328P Pin | Arduino Pin | 7-Segment LED Segment |
| -------------- | ----------- | --------------------- |
| PD2            | D2          | A                     |
| PD3            | D3          | B                     |
| PD4            | D4          | C                     |
| PD5            | D5          | D                     |
| PD6            | D6          | E                     |
| PB0            | D8          | F                     |
| PC4            | A4          | G                     |
| PB2            | D10         | D1 (Digit 1)          |
| PB3            | D11         | D2 (Digit 2)          |
| PB4            | D12         | D3 (Digit 3)          |
| PB5            | D13         | D4 (Digit 4)          |
| PC0            | A0          | Decimal Point         |

Additional connections:

- PC1 (A1) - Set Button (internal pull-up)
- PC2 (A2) - Start/Stop Button (internal pull-up)
- PC3 (A3) - Potentiometer (Analog input)

## Timer Modes

- **Set Mode**: Hold the Set button for 3 seconds to enter set mode. Use the potentiometer to adjust minutes, then seconds.
- **Start/Stop Mode**: Use the Start/Stop button to begin or pause the countdown.

## How to Run

1. Connect the hardware according to the pin configuration.
2. Upload the code to the ATmega328P/Arduino Uno.
3. Adjust the timer using the potentiometer, and control it with the buttons.

## Morse Code (SOS)

The timer plays an SOS signal in Morse code using a buzzer when the countdown finishes:

- **S**: "..." (Three short tones)
- **O**: "---" (Three long tones)
- **S**: "..." (Three short tones)

## Dependencies

- AVR libraries (`avr/io.h`, `util/delay.h`, `avr/interrupt.h`)
- Custom `pitches.h` file for generating tones.

## Code Overview

- **`SevenSegment()`**: Handles displaying numbers on the 7-segment LED.
- **`init_Timer0()`**: Initializes Timer0 for managing time intervals.
- **`ISR(TIMER0_COMPA_vect)`**: Interrupt handler to manage button states and timer updates.
- **`countdown()`**: Decrements the timer every second.
- **`initTimer()`**: Initializes Timer1 for generating PWM signals for the SOS buzzer.
- **`tone()`**: Generates sound frequencies for Morse code using Timer1.
- **`mute()`**: Stops the tone output.
- **`display()`**: Refreshes the 7-segment display with the current timer values.
- **`parseCount()`**: Extracts individual digits for minutes and seconds.
- **`initADC()`**: Initializes the Analog-to-Digital Converter (ADC) to read potentiometer values.

## Future Enhancements

- Debouncing the Buttons:Although there is a small delay to account for debounce, using hardware debouncing or more robust software debouncing could improve reliability.

## Demo 

https://github.com/user-attachments/assets/8c52758e-d985-4fc2-a4e5-7c50e0adf78a
