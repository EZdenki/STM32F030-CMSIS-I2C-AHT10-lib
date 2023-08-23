# STM32F030-CMSIS-I2C-AHT10-lib
### Library to Initialize and Read the AHT10 I2C Temperature and Humidity Sensor when attached to the STM32F030 Microcontroller
### The STM32F030-CMSIS-I2C-AHT10-lib.c library supports the following routines:

+ **```void  AHT10_init( I2C_TypeDef *this I2C, uint32_t I2CSpeed )```**<br>
  Initialize the specified I2C interface (I2C1) at the specified I2C speed. Then
  initialize the AHT10 unit to its default calibrated values.
+ **```void  AHT10_readSensorData( uint8_t *data )```**<br>
  Called with a pointer to an array of at least 6 uint8_t ints.
  Sends command to trigger a measurement. Then reads in the measured data after 75 ms.
  The status register is contained in the first byte in the array. The subsequent 5 bytes
  contain the raw humidity and temperature values.
  The status register should have a value of 0x19 if a normal temperature/humidity
  conversion occurred. A status value of 0x99 indicates that there was not sufficient time
  to complete the measurement.<br>
  Note that, after powering up the sensor, the AHT10_init routine must be called one time
  before calling this routine for the first time. Subsequent calls to this routine do not
  require additional calls to AHT10_init();
+ **```uint8_t  AHT10_getTempHumid100( int16_t *temp100, int16_t *humid100 )```**<br>
  Gets temperature and humidity data from the AHT10 I2C temperature and humidity sensor.
  Data is passed via reference to temp100 and humid100 integer values. temp100 is 100
  times the value of the temperature in Celsius, and humid100 is 100 times the relative
  humidity.<br>
  For example, temp100 = 2753 indicates an actual temperature of 27.53 degrees Celsius.
  Likewise, humid100 = 67 indicates an actual humidity of 0.67 (67%).
  The return value is the sensor status byte.
+ **```void  i100toa( int16_t realV, char *thisString )```**<br>
  i100toa takes a number with 2 decimal places multiplied by 100, and returns a string
  of the original decimal number rounded to 1 decimal place. For example, if the number
  in question is 12.36, then 1236 is passed via realV. The resulting string is 12.4,
  because 12.36 rounds up to 12.4. Negative numbers and more complex rounding work as
  expected. For example, -2.35, passed as -235, returns "-2.4", and 19.96, passed as 1996,
  "w" = -2 and "d" = 4. Also, 19.96 would return "w" = 20 and "d" = 0.<br>
  <br>
**See STM32F030-CMSIS-AHT10-lib.c for further details.**
### The STMF030-CMSIS-AHT10-lib.c library requires the following libraries to operate:
- STM32F030-CMSIS-I2C-lib
- STM32F030-Delay-lib.c
### Sample Application to Display Temperature and Humidity on an 8x2 LCD
- Includes a sample project that reads the sensor and displays the temperature, humidity, temperature index,
  and English phrases. to a NON-I2C-driven 8x2 LCD module.
- See STM32F030-CMSIS-LCD-lib.c for details on how to connect the LCD module to the STM32F030.
- To run the sample sample 16x2 LCD project, clone this repo and then simply type<br>
  ```make clean && make```<br>
  to compile and upload.
- See main.c for further details.
