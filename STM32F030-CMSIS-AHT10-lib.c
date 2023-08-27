//  ==========================================================================================
//  STM32F030-CMSIS-I2C-AHT10-lib.c
//  ------------------------------------------------------------------------------------------
//  Routines to get data from the AHT10 I2C temperature and humdidity sensor. Note that the
//  values returned indicate the temperature and humidity x 100. This is done to avoid the
//  overhead of using floating-point routines.
//  ------------------------------------------------------------------------------------------
//  https://github.com/EZdenki/STM32F030-CMSIS-I2C-AHT10-lib
//  Released under the MIT License
//  Copyright (c) 2023
//  Mike Shegedin, EZdenki.com
//  Version 1.0   20 Jul 2023   Updated init procedure
//  Version 0.9      May 2023   Start
//  ------------------------------------------------------------------------------------------
//  Target Devices:
//    STM32F030Fxxx running at 8 MHz internal clock
//    AHT10 Temperature and Humidity Module
//  ------------------------------------------------------------------------------------------
//  HARDWARE SETUP:
//
//                  STM32F030Fxxx
//                   ,---. .---,
//             BOOT0 |1o  V  20| PA14 / SWCLK               AHT10 Module
//            OSC_IN |2      19| PA13 / SWDIO              ,-------------, 
//           OSC_OUT |3      18| PA10 / I2C1_SDA --------- | SDA  AHT10  |
//              NRST |4      17| PA9  / I2C1_SCL --------- | SCL  Module |
//              VDDA |5      16| VCC               GND --- | GND         |
//               PA0 |6      15| GND               VCC --- | VIN         |
//               PA1 |7      14| PB1                       '-------------'
//               PA2 |8      13| PA7
//               PA3 |9      12| PA6   ** Pullups on SDA/SCL not required as they are
//               PA4 |10     11| PA5      built into the AHT10 module
//                   '---------'
//
//  ------------------------------------------------------------------------------------------
//  Routines in this Library
//
//  void
//  AHT10_init( I2C_TypeDef *this I2C, uint32_t I2CSpeed )
//    Initialize the specified I2C interface (I2C1) at the specified I2C speed. Then
//    initialize the AHT10 unit to its default calibrated values.
//
//  void
//  AHT10_readSensorData( uint8_t *data )
//    Called with a pointer to an array of at least 6 uint8_t ints.
//    Sends command to trigger a measurement. Then reads in the measured data after 75 ms.
//    The status register is contained in the first byte in the array. The subsequent 5 bytes
//    contain the raw humidity and temperature values.
//    The status register should have a value of 0x19 if a normal temperature/humidity
//    conversion occurred. A status value of 0x99 indicates that there was not sufficient time
//    to complete the measurement.
//    Note that, after powering up the sensor, the AHT10_init routine must be called one time
//    before calling this routine for the first time. Subsequent calls to this routine do not
//    require additional calls to AHT10_init();
//
//  uint8_t
//  AHT10_getTempHumid100( int16_t *temp100, int16_t *humid100 )
//    Gets temperature and humidity data from the AHT10 I2C temperature and humidity sensor.
//    Data is passed via reference to temp100 and humid100 integer values. temp100 is 100
//    times the value of the temperature in Celsius, and humid100 is 100 times the relative
//    humidity.
//    For example, temp100 = 2753 indicates an actual temperature of 27.53 degrees Celsius.
//    Likewise, humid100 = 67 indicates an actual humidity of 0.67 (67%).
//    The return value is the sensor status byte.
//
//  void
//  i100toa( int16_t realV, char *thisString )
//    i100toa takes a number with 2 decimal places multiplied by 100, and returns a string
//    of the original decimal number rounded to 1 decimal place. For example, if the number
//    in question is 12.36, then 1236 is passed via realV. The resulting string is 12.4,
//    because 12.36 rounds up to 12.4. Negative numbers and more complex rounding work as
//    expected. For example, -2.35, passed as -235, returns "-2.4".
//  ============================================================================================

#ifndef __STM32F103_CMSIS_AHT10_LIB_C
#define __STM32F103_CMSIS_AHT10_LIB_C

#include <string.h>
#include <stdlib.h>

#include "stm32f030x6.h"              // Primary CMSIS header file
#include "STM32F030-CMSIS-I2C-lib.c"  // I2C library
#include "STM32F030-Delay-lib.c"      // pause and delay_us library

I2C_TypeDef *AHT10_I2C;               // Global variable to point to the I2C interface used for
                                      // the I2C AHT10 routines. 

//  Useful constants used with AHT10 sensor routines
#define AHT10_ADD       0x38  // I2C address of AHT10 sensor
#define AHT10_INIT      0xE1  // Initialization command byte
#define AHT10_INIT_D0   0x08  // Initialization 2nd byte to turn on calibration
#define AHT10_INIT_D1   0x00  // Initialization 3rd byte
#define AHT10_TRIG_MEAS 0xAC  // 1st byte to trigger measurement
#define AHT10_TRIG_D0   0x33  // 2nd byte to trigger measurement
#define AHT10_TRIG_D1   0x00  // 3rd byte to trigger measurement
#define AHT10_CHAR_DEG  0xDF  // Degree symbol character
#define AHT10_CHAR_DOT  0xA5  // Center dot Character


//  void
//  AHT10_init( I2C_TypeDef *this I2C )
//    Initialize the specified I2C interface (I2C1) at the specified I2C speed. Then
//    initialize the AHT10 unit to its default calibrated values.
void
AHT10_init( I2C_TypeDef *thisI2C, uint32_t I2CSpeed )
{
  AHT10_I2C = thisI2C;                     // Associate AHT10_ routines with this I2C interface
  I2C_init( AHT10_I2C, I2CSpeed );         // Initialize this I2C2 interface
  I2C_setAddress( AHT10_I2C, AHT10_ADD );
  I2C_setNBytes(  AHT10_I2C, 3 );
  I2C_start( AHT10_I2C );                  // I2C start
  I2C_write( AHT10_I2C, AHT10_INIT    );   // 0xE1: Init command
  I2C_write( AHT10_I2C, AHT10_INIT_D0 );   // 0x08: 2nd init byte to set CAL bit
  I2C_write( AHT10_I2C, AHT10_INIT_D1 );   // 0x00: Finish command with 0-byte
  I2C_stop(  AHT10_I2C );                  // I2C stop
delay_us(40);
}


//  void
//  AHT10_readSensorData( uint8_t *data )
//    Called with a pointer to an array of at least 6 uint8_t ints.
//    Sends command to trigger a measurement. Then reads in the measured data after 75 ms.
//    The status register is contained in the first byte in the array. The subsequent 5 bytes
//    contain the raw humidity and temperature values.
//    The status register should have a value of 0x19 if a normal temperature/humidity
//    conversion occurred. A status value of 0x99 indicates that there was not sufficient time
//    to complete the measurement.
//    Note that, after powering up the sensor, the AHT10_init routine must be called one time
//    before calling this routine for the first time. Subsequent calls to this routine do not
//    require additional calls to AHT10_init();
void
AHT10_readSensorData( uint8_t *data )
{
  I2C_setAddress( AHT10_I2C, AHT10_ADD );
  I2C_setNBytes( AHT10_I2C, 3 );
  I2C_start( AHT10_I2C );                   // Send command to trigger measurement
  I2C_write( AHT10_I2C, AHT10_TRIG_MEAS );  // 0xAC: 1st measurement trigger command byte
  I2C_write( AHT10_I2C, AHT10_TRIG_D0   );  // 0x33: 2nd measurement trigger command byte
  I2C_write( AHT10_I2C, AHT10_TRIG_D1   );  // 0x00: 3rd measurement trigger command byte 
  I2C_stop( AHT10_I2C );                    // Stop this transaction
  
  delay_us( 75e3 );                         // Wait for measurement to complete

  I2C_setNBytes( AHT10_I2C, 6 );
  I2C_setReadMode( AHT10_I2C );         // Set sensor to read mode
  I2C_start( AHT10_I2C );               // Send command to read data registers
  data[0] = I2C_read( AHT10_I2C );      // Read Status register, send ACK
  data[1] = I2C_read( AHT10_I2C );      // Read Humidity [19:12], send ACK
  data[2] = I2C_read( AHT10_I2C );      // Read Humidity [11:4], send ACK
  data[3] = I2C_read( AHT10_I2C );      // Read Humidity [3:0] / Temperature [19:16],
                                        // and send ACK.
  data[4] = I2C_read( AHT10_I2C );      // Read Temperature [15:8], send ACK
  data[5] = I2C_read( AHT10_I2C );      // Read Temperature [7:0], send NAK
  I2C_stop( AHT10_I2C );                // Stop this transaction
  I2C_setWriteMode( AHT10_I2C );        // Restore sensor to write mode
delay_us(420);
}


//  uint8_t
//  AHT10_getTempHumid100( int16_t *temp100, int16_t *humid100 )
//    Gets temperature and humidity data from the AHT10 I2C temperature and humidity sensor.
//    Data is passed via reference to temp100 and humid100 integer values. temp100 is 100
//    times the value of the temperature in Celsius, and humid100 is 100 times the relative
//    humidity.
//    For example, temp100 = 2753 indicates an actual temperature of 27.53 degrees Celsius.
//    Likewise, humid100 = 67 indicates an actual humidity of 0.67 (67%).
//    The return value is the sensor status byte.
uint8_t
AHT10_getTempHumid100( int16_t *temp100, int16_t *humid100 )
{
  uint8_t ahtData[6];             // Contains the 6 bytes data sent from the sensor
  uint32_t tempData, humidData;   // Contains the separated raw temperature and humidity
                                  // data collected from the sensor

  AHT10_readSensorData( ahtData );          // Read raw data from the sensor

                                            // Separate out humidity and temperature data
  humidData = ( ahtData[1]<<16           | ahtData[2]<<8 | ahtData[3] ) >> 4;
  tempData  = ( ahtData[3] & 0x0F ) <<16 | ahtData[4]<<8 | ahtData[5] ;

  *temp100 = (tempData*625) / 32768 - 5000; // Calculate temperature x 100.
                                            // This is done to avoid the overhead of floating-
                                            // point math routines.
                                            // tempC = (( tempV * 200 ) / 2^20 ) - 50
                                            // 100 x tempC = tempV  * 20000 ) / 2^20 ) - 5000
                                            // Reduces to: (( tempV * 625 ) / 32768 ) - 5000
  
  *humid100 = humidData/10486;              // Calculate humidity x 100 (i.e. 65% = 65)
  return ahtData[0];                        // Return device status byte Should be 0x19. See
                                            // datasheet for details.
}


//  void
//  i100toa( int16_t realV, char *thisString )
//    i100toa takes a number with 2 decimal places multiplied by 100, and returns a string
//    of the original decimal number rounded to 1 decimal place. For example, if the number
//    in question is 12.36, then 1236 is passed via realV. The resulting string is 12.4,
//    because 12.36 rounds up to 12.4. Negative numbers and more complex rounding work as
//    expected. For example, -2.35, passed as -235, returns "-2.4", and 19.96, passed as 1996,
//    "w" = -2 and "d" = 4. Also, 19.96 would return "w" = 20 and "d" = 0.
void
i100toa( int16_t realV, char *thisString )
{
  char tmpString[6];              // Used to build output string. Might need to hold "-100\0"

  int16_t  x = abs( realV );      // realV = 2596 (target: 26.0C) -> x = 2596
  int16_t  w = x / 100;           // w (whole part) = 25
  int16_t  f = x - ( w * 100 );   // f (fraction)   = 2596 - 2500 = 96
  int16_t  d = f / 10;            // d (decimal)    = 9
  int16_t  r = f - ( d * 10 );    // r (remainder)  = 96 - 90 = 6

  if( r>=5 )                      // Round up if needed
  {
    d = d + 1;                    // d gets rounded up to 10
    if( d >= 10 )                 // If d gets rounded up,
    {
      d = 0;                      // d = 0    zero out d and
      w = w + 1;                  // w = 26   bump up w. 
    }
  }   
  if( realV < 0 )                 // Start with negative sign if original value was negative
    strcpy( thisString, "-" );
  else                            // otherwise start with blank string.
    strcpy( thisString, "" );

  itoa( w, tmpString, 10 );         // Make whole part into string
  strcat( thisString, tmpString );  // Tack onto thisString
  strcat( thisString, "." );        // Add the decimal point
  itoa( d, tmpString, 10 );         // Make a string for the single decimal place
  strcat( thisString, tmpString );  // Tack on the decimal digit to the rest of the string
}  

#endif /* __STM32F103_CMSIS_AHT10-LIB_C */
