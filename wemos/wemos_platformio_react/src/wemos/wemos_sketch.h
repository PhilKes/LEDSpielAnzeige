#include "modes.h"
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include "wifi_settings.h"
#include <WiFiUdp.h>

//DEBUG true, for USB debugging
#define DEBUG true
//OTA_ENABLED true, for Over The Air Updates (WiFi sketch upload)
#define OTA_ENABLED true

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"europe.pool.ntp.org", 3600, 60000);
const int port = 8181;

String readString = String(100); //string to get commands from cmd_client

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


inline int isDaylightSaving(){
    /**Summer and winter time settings for time zone Berlin:
   * 25.10.20: 1603591221 summer to winter: 7200 -> 3600
   * 
   * 28.03.21: 1616889601 winter to summer: 3600 -> 7200
   * 31.10.21: 1635645601 summe to winter 
   * 
   * 27.03.22: 1648339201
   * 30.10.22: 1667084401
   * 
   * 26.03.23: 1679788801
   * 29.10.23: 1698534001
   * 
   * 31.03.24: 1711843201
   * 27.10.24: 1729983601
   */
  int now = timeClient.getEpochTime();
  if (now< 1603591221){
    return 7200;
  } if (now>=1603591221 && now <  1616889601) {
    return 3600;
  } if (now >=1616889601 && now < 1635645601) {
    return 7200;
  } if (now >= 1635645601 && now < 1648339201) {
    return 3600;
  } if (now >= 1648339201 && now < 1667084401) {
    return 7200;
  } if (now >= 1667084401 && now < 1679788801) {
    return 3600;
  } if (now >= 1679788801 && now < 1698534001) {
    return 7200;
  } if (now >= 1698534001 && now < 1711843201) {
    return 3600;
  } if (now >= 1711843201 && now < 1729983601) {
    return 7200;
  } return 3600;
}

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

// Set pinModes + init Digits&Segments
 void wemosSetup() 
 {

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(shiftDigitDataPin, OUTPUT);
  pinMode(shiftDigitClkPin, OUTPUT);
  pinMode(shiftDigitLatchPin, OUTPUT);

  pinMode(shiftSegDataPin, OUTPUT);
  pinMode(shiftSegClkPin, OUTPUT);
  pinMode(shiftSegLatchPin, OUTPUT);

  initDigits();
  initSegs();

  /*#if DEBUG
  Serial.begin(115200);
  #endif*/

  //Serial.begin(9600);
  flashLEDBuiltIn();

  
  timeClient.begin();
  timeClient.setTimeOffset(isDaylightSaving());
  timeClient.setUpdateInterval(updateIntervallTime * 60000);

}


void wemosLoop() {

    multiplexLoop(multiplexDelay);  //multiplex Digits, load current Segment values
    if (multiplexCount% 30 == 0) {
      timeClient.update();

      minutes = timeClient.getMinutes();
      hours = timeClient.getHours();
      seconds = timeClient.getSeconds();
      
      mainLoop(hours, minutes, seconds); 
      #if DEBUG
      flashLEDBuiltIn();
      #endif
    } 
}