#include "modes.h"
#include <ArduinoOTA.h>
#include "wifi_settings.h"

//DEBUG true, for USB debugging
#define DEBUG false
//OTA_ENABLED true, for Over The Air Updates (WiFi sketch upload)
#define OTA_ENABLED true

int shiftDigitDataPin=D1;
int shiftDigitClkPin=D3;
int shiftDigitLatchPin=D2;

int shiftSegDataPin=D0;
int shiftSegClkPin=D6;
int shiftSegLatchPin=D5;

int mode=0;
volatile int hours = 99;
volatile int minutes = 99;
volatile int seconds = 99;

inline void flashLEDBuiltIn(){
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
  digitalWrite(LED_BUILTIN, HIGH);
} 

// ShiftOut Segment values
inline void setSegs(byte left,byte right)
{
    digitalWrite(shiftSegLatchPin, LOW);
    // shift out the 16 bits:
    shiftOut(shiftSegDataPin, shiftSegClkPin, LSBFIRST, right);  
    shiftOut(shiftSegDataPin, shiftSegClkPin, LSBFIRST, left);  
  
    digitalWrite(shiftSegLatchPin, HIGH);
}

inline void setDigits(byte value)
{
    digitalWrite(shiftDigitLatchPin, LOW);
    shiftOut(shiftDigitDataPin, shiftDigitClkPin, LSBFIRST, value);
    digitalWrite(shiftDigitLatchPin, HIGH);
}

//Init all Digits to 1st On, all other off
inline void initDigits()
{
  setDigits(0);
}

//Init all Segments to on
inline void initSegs()
{
  setSegs(0,0);
}

volatile byte digitData=B10000000;

//Index of current active Display (starting with 1)
volatile byte displayIdx=0;

//Turns all Segs off, shift the Digits "1" to the left, if last Digit reached, reset to first Digit
//returns true if last Digit was displayed
inline void nextDigit()
{
    if(digitData==B10000000)
    {
      digitData=B00000001;
      displayIdx=0;
    }
    else
    {
      digitData=digitData<<1;
      displayIdx++;
    }

    setSegs(0,0); //Turn all Segments off
    setDigits(digitData);
}

/* main multiplex Loop */
inline void multiplexLoop(int multiDel)
 {
   do{
      nextDigit();
      if (dimBrightness != 10) {
        setSegs(0,0); //Load Segments for new Digits
        delayMicroseconds(dimDelay);
      }
      setSegs(digitsLeft[displayIdx],digitsRight[displayIdx]);
      delayMicroseconds(multiDel - dimDelay);
   }while(digitData!=B10000000);
   multiplexCount++;
   if (multiplexCount >180000){
     multiplexCount = 0;
   }
 }

inline void setTime(time_t time){
  struct tm *tmp = localtime(&time);
  hours=tmp->tm_hour;
  minutes=tmp->tm_min;
  seconds=tmp->tm_sec;
}

// Set pinModes + init Digits&Segments
 void wemosSetup() {

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(shiftDigitDataPin, OUTPUT);
  pinMode(shiftDigitClkPin, OUTPUT);
  pinMode(shiftDigitLatchPin, OUTPUT);

  pinMode(shiftSegDataPin, OUTPUT);
  pinMode(shiftSegClkPin, OUTPUT);
  pinMode(shiftSegLatchPin, OUTPUT);

  initDigits();
  initSegs();

  flashLEDBuiltIn();
}

//TEST LOOP to display current Time from NTP
void wemosLoop(volatile char digitsLeft[],volatile char digitsRight[], NTPStatus* ntpStatus) {

    multiplexLoop(multiplexDelay);  //multiplex Digits, load current Segment values
    if (multiplexCount% 30 == 0) {
      setTime(ntpStatus->getTime());
      #if DEBUG
      Serial.printf("%d:%d:%d\n", hours,minutes,seconds);
      #endif
      mainLoop(hours, minutes, seconds); 
    } 
}