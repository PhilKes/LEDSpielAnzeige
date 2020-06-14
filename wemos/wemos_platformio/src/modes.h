#include <Arduino.h>

volatile byte numbers[]= 
{
  B10111011,
  B00011000,
  B11101010,
  B01111010,
  B01011001,
  B01110011,
  B11110011,
  B00011010,
  B11111011,
  B01111011,
};

//current Display data of all 8 left Displays
volatile byte digitsLeft[]= 
{
  numbers[0], //C1
  numbers[1], //C2
  numbers[2], //C3
  numbers[3], //C4
  numbers[4], //C5
  numbers[5], //C6
  numbers[6], //C7
  numbers[7], //C8
};

//current Display data of all 8 right Displays
volatile byte digitsRight[]= 
{
  numbers[0], //B1
  numbers[1], //B2
  numbers[2], //B3
  numbers[3], //B4
  numbers[4], //B5
  numbers[5], //B6
  numbers[6], //B7
  numbers[7], //B8
};

/** MODES FOR SETTING digitsLeft & digitRight VALUES
 *  CALL ANY METHOD AFTER multiplexLoop() IN loop() FUNCTION 
*/

//Count amount of multiplexLoop() calls for timings
volatile int multiplexCount=0;
//Multiplex time per Digits in microseconds
const int multiplexDelay=1700;

//All Digits loop numbers 0-9
// speedInMs: time for each number in milliseconds
volatile int num=0;
inline void numbersLoop(int speedInMs){
    if(multiplexCount > (speedInMs*1000)/(multiplexDelay*8))
    {
        multiplexCount=0;
        num++; 
        if(num> 9)
        {
            num=0;
        }
        for (byte i = 0; i < 8; i++)
        {
            digitsLeft[i]=numbers[num];
            digitsRight[i]=numbers[num];
        }

    }
}

int digit=0;
inline void digitMappings(int speedInMs)
{
  for (byte i = 0; i < 8; i++)
  {
    digitsLeft[i]=0;
    digitsRight[i]=0;
  }
  if(multiplexCount > (speedInMs*1000)/(multiplexDelay*8))
  {
    multiplexCount=0;
    digit++;
    if(digit >7)
    {
      digit=0;
      delay(1000); //to highlight last digit reached
    }
  }
  digitsLeft[digit]=B11111111;
  digitsRight[digit]=B11111111;
}