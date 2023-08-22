//  ==========================================================================================
//  STM32F030-CMSIS-LCD-lib.c
//  ------------------------------------------------------------------------------------------
//  Simple functionality to control a 8x2 LCD via the STM32F030 microcontroller.
//  ------------------------------------------------------------------------------------------
//  https://github.com/EZdenki/STM32F030-CMSIS-LCD-lib
//  Released under the MIT License
//  Copyright (c) 2023
//  Mike Shegedin, EZdenki.com
//    Version 1.0   16 Aug 2023   Updated comments
//    Version 0.9      Jul 2023   Started
//  ------------------------------------------------------------------------------------------
//  Target Devices: STM32F030F4xx, 16x2 (or 8x2, etc.) LCD display
//  ------------------------------------------------------------------------------------------
//  HARDWARE SETUP:
//    The LCD module requires 4 to 5 Vdc to operate and therefore uses 5V signals. However,
//    the STM32F030 IO pins are not 5V tolerant. To mitigate this, all of the signal lines
//    between the LCD display and the microcontroller will be pulled down using a 10k
//    resistor. The LCD does respond to 3.3 V signals, however, this is a little lower than
//    the 70% of VCC required for a positive pulse according to the datasheet. This can be
//    further mitigated by powering the LCD module with 4.3 V by connecting the VCC pin on the
//    LCD through a diode. By doing so, a high signal would only require 70% of 4.3 V, or
//    3.0 V, making 3.3 signalling sufficient.
//
//          STM32F030        LCD    INLINE  5V/GND
//         ============    =======  ======  ======
//         GND (pin 15) ---- VSS ------------- GND
//         VDD (pin 16) --------------------- 3.3V
//                           VDD - [-Diode+] - 5V
//                                         /-- 5V
//                            VO -[10k Pot]**     (LCD contrast control)
//                                         \-- GND
//          A5 (pin 11) ----- RS --- [10K] --- GND
//                            RW ------------- GND
//          A4 (pin 10) ----- EN --- [10K] --- GND
//                            D0 ------------- GND
//                            D1 ------------- GND
//                            D2 ------------- GND
//                            D3 ------------- GND
//           A0 (pin 6) ----- D4 --- [10K] --- GND
//           A1 (pin 7) ----- D5 --- [10K] --- GND
//           A2 (pin 8) ----- D6 --- [10K] --- GND
//           A3 (pin 9) ----- D7 --- [10K] --- GND
//
//  ==========================================================================================

#ifndef __STM32F030_CMSIS_LCD_LIB_C
#define __STM32F030_CMSIS_LCD_LIB_C

#include "stm32f030x6.h"          // Primary CMSIS header file
#include "STM32F030-Delay-lib.c"  // Has the microsecond delay function

#define LCD_RS_BIT (1<<5)         // Define GPIO pin for RS
#define LCD_EN_BIT (1<<4)         // Define GPIO pin for EN

#define LCD_RS_ON()  GPIOA->ODR |=  LCD_RS_BIT  // Turn ON RS pin
#define LCD_EN_ON()  GPIOA->ODR |=  LCD_EN_BIT  // Turn ON EN pin
#define LCD_RS_OFF() GPIOA->ODR &= ~LCD_RS_BIT  // Turn OFF RS pin
#define LCD_EN_OFF() GPIOA->ODR &= ~LCD_EN_BIT  // Turn OFF EN pin

//  Defines for LCD commands
#define LCD_CLEAR                0x01
#define LCD_HOME                 0x02
#define LCD_OFF                  0x08
#define LCD_ON_NO_CURSOR         0x0C
#define LCD_ON_BLINK_CURSOR      0x0D
#define LCD_ON_LINE_CURSOR       0x0E 
#define LCD_ON_LINE_BLINK_CURSOR 0x0F
#define LCD_4B_58F_2L            0x2B
#define LCD_1ST_LINE             0x80
#define LCD_2ND_LINE             0xC0


//  writeLowerNibble
//  Puts the lower 4 bits of 'data' onto GPIO pins A[3:0]
void
LCD_writeLowerNibble( uint8_t data )
{
  GPIOA->ODR &= 0xFFF0;    // Clear GPIO A[3:0]
  // Mask off the lower nibble of data, and write those bits to the newly cleared nibble
  // at GPIO A[3:0].
  GPIOA->ODR |= (data & 0x0F) ;
}


//  writeUpperNibble
//  Writes the upper 4 bits of 'data' onto GPIO pins A[3:0]
void
LCD_writeUpperNibble( uint8_t data )
{
  GPIOA->ODR &= 0xFFF0;    // Clear GPIO A[3:0]
  // Mask off the upper nibble of data, and shift those bits down 4 bits and write them to
  // the newly cleared nibble at A[3:0]
  GPIOA->ODR |= (data & 0xF0) >> 4;
}


//  LCD_pulse_EN
//  Make EN high, wait for 1 ms, and then bring it down low.
void
LCD_pulse_EN(void)
{
  LCD_EN_ON();
  delay_us(1e3);
  LCD_EN_OFF();
}


//  LCD_cmd
//  Send a 1-byte command to the LCD
void
LCD_cmd( uint8_t data )
{
  // Make sure EN and RS are low
  LCD_EN_OFF();
  LCD_RS_OFF();
  // Delay
  delay_us(1);
  // Place upper nibble of command on GPIOA[3:0]
  LCD_writeUpperNibble( data );
  // Pulse EN pin to set this nibble
  LCD_pulse_EN();
  // Place lower nibble of command on GPIOA[3:0]
  LCD_writeLowerNibble( data );
  // Pulse EN pin to set this nibble
  LCD_pulse_EN();
  delay_us( 50 );
  if( ( data == LCD_CLEAR ) || ( data == LCD_HOME ))  // 2 ms delay if CLEAR or HOME commands
    delay_us( 1.5e3 );
}


//  LCD_putc
//  Send a character to the LCD
void
LCD_putc( char data )
{
  // Set EN low, and RS high
  LCD_EN_OFF();
  LCD_RS_ON();
  delay_us(1);
  // Place upper nibble of character on GPIOA[3:0]
  LCD_writeUpperNibble( data );
  // Pulse EN pin to set this nibble
  LCD_pulse_EN();
  // Place lower nibble of command on GPIOA[3:0]
  LCD_writeLowerNibble( data );
  // Pulse EN pin to set this nibble
  LCD_pulse_EN();
  delay_us(2);
}


//  LCD_puts
//  Takes a pointer to a null-terminated string and displays that string
//  from the current LCD cursor position. Does not check for LCD line/string
//  overflow.
void
LCD_puts( char *data )
{
  uint8_t j=0;

  while( data[j] != 0 )
  {
    LCD_putc( data[j] );
    j++;
  }
}


//  LCD_init
//  Initializes LCD by initializing required GPIO ports and pins used to talk
//  to the LCD. Also initializes the LCD screen itself to be in 4-bit mode.
//  This setup uses GPIO A3, A2, A1, and A0 for LCD data pins 4, 5, 6, 7 respectively.
//  GPIO A4 is LCD RS and GPIO A5 is the LCD EN pin.
//  Note that these GPIO pins are *not* 5 V tolerant. So they are installed with physical
//  pull-down resistors to ground to protect the GPIO pins on the STM32F030 from 5 V from
//  the LCD. The pins are configured as outputs in a push-pull configuration.
void
LCD_init( void )
{
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN ;     // Enable GPIO Port A
  // Set the MODERx[1:0] bits to 0b01 for push-pull output configuration
  GPIOA->MODER |= GPIO_MODER_MODER0_0 |
                  GPIO_MODER_MODER1_0 |
                  GPIO_MODER_MODER2_0 |
                  GPIO_MODER_MODER3_0 |
                  GPIO_MODER_MODER4_0 |
                  GPIO_MODER_MODER5_0;

  // Start with delay to make sure the LCD module is fully powered up.
  // 400 ms is overkill since 300 ms works. But decided to give some leeway.
  delay_us(300e3);
  // Set EN and RS low
  LCD_EN_OFF();
  LCD_RS_OFF();
  // Delay
  delay_us(1);
  // Write 0x02 to data pins to start 4-bit mode
  LCD_writeLowerNibble( 0x2 );
  // Pulse EN pin to set this nibble
  LCD_pulse_EN();

  LCD_cmd( LCD_4B_58F_2L );     // 4-bit, 5x8 font, 2 lines
  LCD_cmd( LCD_CLEAR );         // Clear display
  LCD_cmd( LCD_HOME );          // Home
  LCD_cmd( LCD_ON_NO_CURSOR );  // Display on, no cursor
}

#endif /* __STM32F030_CMSIS_LCD_LIB_C  */
