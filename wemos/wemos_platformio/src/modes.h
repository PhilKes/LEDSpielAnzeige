#include <Arduino.h>

//Multiplex time per Digits in microseconds
const int multiplexDelay=1700;

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
  numbers[0],
  numbers[1],
  numbers[2],
  numbers[3],
  numbers[4],
  numbers[5],
  numbers[6],
  numbers[7],
};

//current Display data of all 8 right Displays
volatile byte digitsRight[]= 
{
  numbers[0],
  numbers[1],
  numbers[2],
  numbers[3],
  numbers[4],
  numbers[5],
  numbers[6],
  numbers[7],
};

/** MODES FOR SETTING digitsLeft & digitRight VALUES
 *  CALL ANY METHOD AFTER multiplexLoop() IN loop() FUNCTION 
*/

volatile int multiplexCount=0;
volatile int num=0;

//All Digits loop numbers 0-9
// speedInMs: time for each number in milliseconds
inline void numbersLoop(int speedInMs){
    multiplexCount++;
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