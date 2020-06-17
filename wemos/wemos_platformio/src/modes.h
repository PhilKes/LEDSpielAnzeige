#include <Arduino.h>
#include <NTPClient.h>
#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
WiFiClient client;

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

//For the weather API

const String url ="http://api.openweathermap.org/data/2.5/weather?q=Konstanz,DE&units=metric&appid=28f8833d6494314b297aba01d98a6f72";
String apiKey= "28f8833d6494314b297aba01d98a6f72"; 
char weather_server[] = "api.openweathermap.org";   
String location = "Konstanz, DE";

//Count amount of multiplexLoop() calls for timings
volatile int multiplexCount=0;
//Multiplex time per Digits in microseconds
const int multiplexDelay=1000;

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


volatile int temp = -99;

volatile int second_min;
volatile int first_min;

volatile int second_h;
volatile int first_h;

volatile bool dispSeconds = true;
volatile bool dispTemp = true;

volatile bool kabuMode = false;
volatile bool setNames = false;
volatile int playerNamesSet = 0;
String player1 = "Player 1";
String player2 = "Player 2";
String player3 = "Player 3";
String player4 = "Player 4";
volatile int kabuScore1 = 0;
volatile int kabuScore2 = 0;
volatile int kabuScore3 = 0;
volatile int kabuScore4 = 0;

volatile int zeroDigit;
volatile int firstDigit;
volatile int secondDigit;

volatile int scoreOnOff = 0;
volatile int scoreGuest = 0;
volatile int scoreHome = 0;

inline void setNumberToTarget(int number, int TargetDisplay) { 
  /**
   * Target display is either of the 4 sets of 3 panels which can display numbers between -10 and 199 counting top to bottom left to right 
   * 1 is Home score
   * 2 is Guest score
   * 3 is Middle Clock
   * 4 is Bottom left
   * 5 is Bottom right
   */
  if (number >= 0){
    secondDigit = (number%100) %10;
    firstDigit = ((number%100)-secondDigit)/10;
  } else {
    number = -number;
    secondDigit = (number%100) %10;
    firstDigit = ((number%100)-secondDigit)/10;
    number = -number;
  }
 

  if (TargetDisplay == 1) {
    if (number<0 && number > -10){
      digitsLeft[0] = B01000000;
      digitsLeft[4] = numbers[-number];
    } if (number<10 && number > -1){
      digitsLeft[4] = numbers[number];
      digitsLeft[0] = 0;
    } if (number>9 && number <100) {
      digitsLeft[4] = numbers[secondDigit];
      digitsLeft[0] = numbers[firstDigit];
    } if (number>99 && number <200) {
      digitsLeft[4] = numbers[secondDigit] + B00000100;
      digitsLeft[0] = numbers[firstDigit] + B00000100;
    } if (number >199) {
      digitsLeft[4] = numbers[9] + B00000100;
      digitsLeft[0] = numbers[9] + B00000100;
    }
  } if (TargetDisplay == 2) {
    if (number<0 && number > -10){
      digitsLeft[1] = B01000000;
      digitsLeft[5] = numbers[-number];
    } if (number<10 && number > -1){
      digitsLeft[5] = numbers[number];
      digitsLeft[1] = 0;
    } if (number>9 && number <100) {
      digitsLeft[5] = numbers[secondDigit];
      digitsLeft[1] = numbers[firstDigit];
    } if (number>99 && number <200) {
      digitsLeft[5] = numbers[secondDigit] + B00000100;
      digitsLeft[1] = numbers[firstDigit] + B00000100;
    } if (number > 199) {
      digitsLeft[1] = numbers[9] + B00000100;
      digitsLeft[5] = numbers[9] + B00000100;
    }
  } 
  
  
  
  if (TargetDisplay == 4) {
    if (number<0 && number > -100) {
      digitsRight[3] = B01000000;
      digitsRight[4] = numbers[secondDigit];
      digitsRight[0] = numbers[firstDigit];
    } if (number> 99) {
      zeroDigit = (number - (firstDigit *10 + secondDigit)) /100;
      digitsRight[3] = numbers[zeroDigit];
      digitsRight[4] = numbers[secondDigit];
      digitsRight[0] = numbers[firstDigit];
    } else {
      digitsRight[4] = numbers[secondDigit];
      digitsRight[0] = numbers[firstDigit];
    }
  } if (TargetDisplay ==5 ) {
      if (number<0 && number > -100) {
        digitsRight[7] = B01000000;
        digitsRight[5] = numbers[secondDigit];
        digitsRight[1] = numbers[firstDigit];
      } if (number> 99) {
        zeroDigit = (number - (firstDigit *10 + secondDigit)) /100;
        digitsRight[7] = numbers[zeroDigit];
        digitsRight[5] = numbers[secondDigit];
        digitsRight[1] = numbers[firstDigit];
      } else {
        digitsRight[5] = numbers[secondDigit];
        digitsRight[1] = numbers[firstDigit];
      }
  } 
}


inline void setScore(){

  if (scoreOnOff == 0) {
    digitsLeft[5] = 0;
    digitsLeft[1] = 0;

    digitsLeft[0] = 0;
    digitsLeft[4] = 0;

  } else {
    if(scoreHome <-9){
      scoreHome = -9;
    } if (scoreHome >199){
      scoreHome = 199;
    }  if(scoreGuest <-9){
      scoreGuest = -9;
    } if (scoreGuest >199){
      scoreGuest = 199;
    }   
    setNumberToTarget(scoreHome, 1);
    setNumberToTarget(scoreGuest, 2);
  }
  if (kabuMode){
    setNumberToTarget(kabuScore1, 1);
    setNumberToTarget(kabuScore2, 2);
    setNumberToTarget(kabuScore3, 4);
    setNumberToTarget(kabuScore4, 5);
  }
}

inline void get_weather(){
 
    HTTPClient http;  //Declare an object of class HTTPClient
 
    // specify request destination
    http.begin("http://api.openweathermap.org/data/2.5/weather?q=Konstanz,DE&units=metric&appid=28f8833d6494314b297aba01d98a6f72");
 
    int httpCode = http.GET();  // send the request
 
    if (httpCode > 0) { // check the returning code
 
      String payload = http.getString();   //Get the request response payload
      //Serial.println(payload);
      DynamicJsonDocument root(512);
      //StaticJsonDocument<5000> root; 
      deserializeJson(root, payload);
      temp = (int)(root["main"]["temp"]) ;        // get temperature
    }
 
    http.end();   //Close connection

}



inline void show_time(int hours, int minutes, int seconds) {
  
  second_min = minutes % 10;
  first_min = (minutes - second_min)/10;

  second_h = hours%10;
  first_h = (hours - second_h)/10;

  if (multiplexCount == 200 ) {
    if (dispTemp){
      if (WiFi.status() == WL_CONNECTED){
        get_weather();
      }
    }
  }

  if (multiplexCount>180000){
    multiplexCount = 0;
  }

  for (byte i = 0; i < 8; i++)
  {
    digitsLeft[i]=0;
    digitsRight[i]=0;
  }

    digitsLeft[2] = numbers[first_min] + B00000100;
    digitsLeft[6] = numbers[second_min] + B00000100;

    digitsRight[2] = numbers[first_h];
    digitsRight[6] = numbers[second_h];

    if (dispSeconds){
      setNumberToTarget(seconds, 5);
    } else {
      digitsRight[1] = 0;
      digitsRight[5] = 0;
    }
    if (dispTemp){
      setNumberToTarget(temp, 4);
    } 

    setScore();
}