//  ==========================================================================================
//  main.c for STM32F030-CMSIS-I2C-AHT10-lib.c
//  ------------------------------------------------------------------------------------------
//  Reads in temperature and humidity data from an AHT10 I2C temperature and humidity
//  sensor, and displays this data along with comfort level phrases to an LCD module.
//  ------------------------------------------------------------------------------------------
//  https://github.com/EZdenki/STM32F030-CMSIS-I2C-AHT10-lib
//  Released under the MIT License
//  Copyright (c) 2023
//  Mike Shegedin, EZdenki.com
//  Version 1.0   16 Aug 2023   Cleanup code and comments
//  Version 0.1   27 Jul 2023   Started port from STM32F103-CMSIS-I2C-LCD-AHT10 project
//  ------------------------------------------------------------------------------------------
//  Target Devices:
//    STM32F030Fxxx running at 8 MHz internal clock
//    AHT10 Temperature and Humidity Module
//    8x2 LCD Module (not I2C-driven module)
//  ------------------------------------------------------------------------------------------
//  HARDWARE SETUP:
//
//     AHT10 Module
//     ------------
//      LCD Module        STM32F030       POWER
//     ============    ===============    =====
//                          GND ---------- GND
//                          VCC ---------- 3.3V
//
//         |  VIN ------------------------ 5V
//   AHT10 |  GND ------------------------ GND
//  Module |  SCL -- A9 /Pin17 SCK |I2C1
//         |  SDA -- A10/Pin18 SDA |
//
//
//         | VSS (1)  -------------------- GND
//         | VDD (2)  -------------------- 5V
//         |                         ,---- 5V      
//         | VO  (3)  --------------< --------- Center [10 KΩ Contrast Pot]
//         |                         `---- GND
//     LCD | RS  (4) --- A5 -- [10 KΩ] --- GND
//  Module | RW  (5) --------------------- GND
//         | EN  (6) --- A4 -- [10 KΩ] --- GND 
//         | 
//         | D4  (11) -- A0 -- [10 KΩ] --- GND
//         | D5  (12) -- A1 -- [10 KΩ] --- GND
//         | D6  (13) -- A2 -- [10 KΩ] --- GND
//         | D7  (14) -- A3 -- [10 KΩ] --- GND
//
// ===========================================================================================

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "stm32f030x6.h"                  // Primary CMSIS header file

#include "STM32F030-CMSIS-LCD-lib.c"      // LCD driver library
#include "STM32F030-CMSIS-AHT10-lib.c"    // AHT10 sensor library



//  float
//  heatIndex( float tempC, float humid )
//  Returns the "heat index" given the temperature in Celsius and relative humidity as a
//  whole number, i.e. 67.3% humidity is represented as 67 (not 0.673). Note that the values
//  returned by this function are not likely to be particularly accurate for temperatures
//  below 20 degrees Celsius or humidity values below 40%.
//  Constants and formula for this routine taken from:
//    https://en.wikipedia.org/wiki/Heat_index
float
heatIndex( float t, float r )
{
  float htIdx;  // Holds calculated heat index value
/*
//  These constants are for Fahrenheit calculations:
  const float c1 = -42.379;
  const float c2 =   2.04901523;
  const float c3 =  10.14333127;
  const float c4 =  -0.22475541;
  const float c5 =  -6.83783e-3;
  const float c6 =  -5.481717e-2;
  const float c7 =   1.22874-3;
  const float c8 =   8.5282e-4;
  const float c9 =  -1.99e-6;
*/

//  These following constants are for Celsius calculations:
  const float c1 = -8.78470    ;
  const float c2 =  1.61139    ;
  const float c3 =  2.33855    ;
  const float c4 = -1.46116e-1 ;
  const float c5 = -1.23081e-2 ;
  const float c6 = -1.64248e-2 ;
  const float c7 =  2.21173e-3 ;
  const float c8 =  7.25460e-4 ;
  const float c9 = -3.58200e-6 ;
  
  htIdx = c1 + c2*t + c3*r + c4*t*r + c5*t*t + c6*r*r + c7*t*t*r + c8*t*r*r + c9*t*t*r*r;

  return htIdx;
}  


//  void
//  outFuzzyHeatIndex( int heatIndex )
//  Output a 10-character string to the LCD describing the current heat index in English.
void
outFuzzyHeatIndex( int temp )
{
  if( temp < 1 )
    LCD_puts("freezing");
  else if( temp < 11 )
    LCD_puts("too cold");
  else if( temp < 18 )
    LCD_puts(" cold   ");
  else if( temp < 27 )
    LCD_puts(" nice!  ");
  else if( temp < 30 )
    LCD_puts("  warm  ");
  else if( temp < 35 )
    LCD_puts("  hot   ");
  else
    LCD_puts("too hot!");
}


// ============================================================================================
// main
// ============================================================================================
int
main()
{
  char     myString[16];            // Will hold printable strings
  int16_t  temp100, humid100;       // Used in conversion from raw to real data
  float    rTemp, rHumid, heatIdx;  // Used to pass values to/from heat index routine

  LCD_init( );                  // Set the LCD interface to I2C1 and initialize it
  LCD_cmd( LCD_4B_58F_2L );
  // Get LCD into 4-bit mode
  LCD_cmd( LCD_ON_NO_CURSOR );  // LCD ON, Cursor OFF
  LCD_cmd( LCD_CLEAR );         // Clear the LCD screen
  LCD_cmd( LCD_HOME );          // Set the LCD to the home position
  
  AHT10_init( I2C1, 100e3 );               // Initialize AHT10 sensor, set sensor to I2C2

  while ( 1 )                           // Repeat this block forever
  {
  LCD_cmd( LCD_CLEAR );         // Clear the LCD screen
    //AHT10_readSensorData( gotData );  // Get data from sensor
    AHT10_getTempHumid100( &temp100, &humid100  );

    // Separate out humidity and temperature data
    LCD_cmd( LCD_1ST_LINE );    // Position LCD to display temperature and humidity
    i100toa( temp100, myString );       // Convert x100 value to readable string with one
                                        // decimal place.
    
    LCD_puts( myString );           // Display rounded single-decimal-place value
    LCD_putc( ' ' );
    LCD_putc( 0xDF );               // Display the degree character
    LCD_puts( "C" );

    LCD_cmd( LCD_2ND_LINE );
    itoa( humid100, myString, 10);      // humidity % = ( humidV / 2^20 ) * 100
    LCD_puts( myString );           // which is equiv. to ( humidV / 10486 ).
    LCD_puts( " % RH " );           // Display % character and spaces to ensure the old
                                        // display is cleared.
    delay_us( 4e6 );                  // Pause for a few seconds

    LCD_cmd( LCD_1ST_LINE );        // Display the temperature index value
    LCD_puts( " Feels  " );
    LCD_cmd( LCD_2ND_LINE );
    LCD_puts( "like " );
    rTemp   = temp100 / 100;            // Get "real" floating point temperature value
    rHumid  = humid100;                 // get "real" floating point humidity value
    heatIdx = heatIndex ( rTemp, rHumid );  // Get heat index

    // Round floating heatIdx to nearest int
    if( heatIdx < 0 )
      heatIdx -= 0.5;
    else
      heatIdx += 0.5;
    
    itoa( (int)heatIdx, myString, 10 );  // Convert integer part of 
    LCD_puts( myString );
    LCD_putc( 0xDF );

    delay_us( 5e6 );                    // Pause approx. 1:0 s between measurements. Excessive
                                        // measurements can lead to self-heating of the sensor.
    LCD_cmd( LCD_2ND_LINE );
    outFuzzyHeatIndex( heatIdx );

    delay_us( 4e6 );
  }
  return 1;
}
