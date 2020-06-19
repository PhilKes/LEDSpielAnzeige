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

//Count amount of multiplexLoop() calls for timings
volatile int multiplexCount=0;
//Multiplex time per Digits in microseconds
const int multiplexDelay=800;


/**
 * Hilfsvariablen 
 */

volatile int temp = -99;
unsigned int updateIntervallWeather = 30; // in minutes 
int updateIntervallTime = 15; //in minutes 
unsigned long timeWeather = millis();
const unsigned long oneMinuteinMillis = 60000;

//static variables for easier handling of displaying numbers
const int turnDisplayOff = -200;
const int minutesDisplay = 6;
const  int hoursDisplay = 5;
const int topLeftDisplay = 1;
const int topRightDisplay = 2;
const int bottomLeftDisplay = 3;
const int bottomRightDisplay = 4;
const int topMiddleDisplay = 7;
const int middleMiddlePoints = 1;
const int middleLeftPoints = 6;
const int middleRightPoints = 7;
const int bottomLeftPoints = 2;
const int bottomRightPoints = 3;
const int topLeftArrow = 4;
const int topRightArrow = 5;

volatile bool dispSeconds = false;
volatile bool dispTemp = false;

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

volatile int dimBrightness = 10; 
volatile int dimDelay = 0;

//All Digits loop numbers 0-9
// speedInMs: time for each number in milliseconds
volatile int num=0;
int digit=0;
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

inline void setNumberToTarget(int number, int TargetDisplay) { 
  /**
   * Target display is either of the 4 sets of 3 panels which can display numbers between -10 and 199 counting top to bottom left to right 
   * if number == -200 the target display is off 
   * 1 is Home score
   * 2 is Guest score
   * 3 is Bottom left
   * 4 is Bottom right
   * 5 is middle left
   * 6 is middle right
   * 7 is top middle number 
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
    if (number == -200){
      digitsLeft[0] = 0;
      digitsLeft[4] = 0;
      return;
    } if (number < -9) {
      digitsLeft[0] = B01000000;
      digitsLeft[4] += numbers[9];
      } if (number<0 && number > -10){
      digitsLeft[0] = B01000000;
      digitsLeft[4] += numbers[-number];
      return;
    } if (number<10 && number > -1){
      digitsLeft[4] += numbers[number];
      digitsLeft[0] = 0;
      return;
    } if (number>9 && number <100) {
      digitsLeft[4] += numbers[secondDigit];
      digitsLeft[0] += numbers[firstDigit];
      return;
    } if (number>99 && number <200) {
      digitsLeft[4] = numbers[secondDigit] + B00000100;
      digitsLeft[0] = numbers[firstDigit] + B00000100;
      return;
    } if (number >199) {
      digitsLeft[4] = numbers[9] + B00000100;
      digitsLeft[0] = numbers[9] + B00000100;
      return;
    }
  } if (TargetDisplay == 2) {
    if (number == -200){
      digitsLeft[1] = 0;
      digitsLeft[5] = 0;
      return;
    } if (number < -9) {
      digitsLeft[1] = B01000000;
      digitsLeft[5] += numbers[9];
      } if (number<0 && number > -10){
      digitsLeft[1] = B01000000;
      digitsLeft[5] += numbers[-number];
      return;
    } if (number<10 && number > -1){
      digitsLeft[5] += numbers[number];
      digitsLeft[1] = 0;
      return;
    } if (number>9 && number <100) {
      digitsLeft[5] += numbers[secondDigit];
      digitsLeft[1] += numbers[firstDigit];
      return;
    } if (number>99 && number <200) {
      digitsLeft[5] = numbers[secondDigit] + B00000100;
      digitsLeft[1] = numbers[firstDigit] + B00000100;
      return;
    } if (number > 199) {
      digitsLeft[1] = numbers[9] + B00000100;
      digitsLeft[5] = numbers[9] + B00000100;
      return;
    }
  } if (TargetDisplay == 3) {
    if (number ==-200) {
      digitsRight[3] = 0;
      digitsRight[4] = 0;
      digitsRight[0] = 0;
      return;
    }
    if (number<0 && number > -100) {
      digitsRight[3] = B01000000;
      digitsRight[4] += numbers[secondDigit];
      digitsRight[0] += numbers[firstDigit];
      return;
    } if (number> 99) {
      zeroDigit = (number - (firstDigit *10 + secondDigit)) /100;
      digitsRight[3] = numbers[zeroDigit];
      digitsRight[4] += numbers[secondDigit];
      digitsRight[0] += numbers[firstDigit];
      return;
    } else {
      digitsRight[4] += numbers[secondDigit];
      digitsRight[0] += numbers[firstDigit];
      return;
    }
  } if (TargetDisplay ==4) {
      if (number ==-200) {
        digitsRight[7] = 0;
        digitsRight[5] = 0;
        digitsRight[1] = 0;
        return;
      } if (number<0 && number > -100) {
        digitsRight[7] = B01000000;
        digitsRight[5] += numbers[secondDigit];
        digitsRight[1] += numbers[firstDigit];
        return;
      } if (number> 99) {
        zeroDigit = (number - (firstDigit *10 + secondDigit)) /100;
        digitsRight[7] = numbers[zeroDigit];
        digitsRight[5] += numbers[secondDigit];
        digitsRight[1] += numbers[firstDigit];
        return;
      } else {
        digitsRight[5] += numbers[secondDigit];
        digitsRight[1] += numbers[firstDigit];
        return;
      }
  } if (TargetDisplay == 5 ) {
      if (number == -200) {
        digitsRight[2] = 0;
        digitsRight[6] = 0;
        return;
      }
      if (number< 0){
        digitsRight[2] = numbers[0];
        digitsRight[6] = numbers[0];
        return;
      } if (number>-1 && number < 100) {
        digitsRight[2] = numbers[firstDigit];
        digitsRight[6] = numbers[secondDigit];
        return;
      } if (number >99){
        digitsRight[2] = numbers[9];
        digitsRight[6] = numbers[9];
        return;
      }
  } if (TargetDisplay == 6 ) {
      if (number == -200){
        digitsLeft[2] = 0;
        digitsLeft[6] = 0;
        return;
      } if (number< 0){
        digitsLeft[2] += numbers[0];
        digitsLeft[6] += numbers[0];
        return;
      } if (number>-1 && number < 100) {
        digitsLeft[2] += numbers[firstDigit] ;
        digitsLeft[6] += numbers[secondDigit];
        return;
      } if (number >99){
        digitsLeft[2] += numbers[9] ;
        digitsLeft[6] += numbers[9] ;
        return;
      }
  } if (TargetDisplay == 7 ){
    if (number >-1 && number <10){
      digitsLeft[3] = numbers[secondDigit];
      return;
    } else {
      digitsLeft[3] = 0;
    }
  }
}

inline  void setPoints(int points) {
  /**
   * 1 is middle double points 
   * 2 is bottom left double points 
   * 3 is bottom right double points 
   * 4 is top left arrow
   * 5 is top right arrow 
   * 6 is middle left three points
   * 7 is middle right three points 
   */
  if (points == 1){
    digitsLeft[2] += B00000100;
    digitsLeft[6] += B00000100;
    return;
  } if (points == 2) {
    digitsRight[0] += B00000100;
    digitsRight[4] += B00000100;
    return;
  } if (points == 3) {
    digitsRight[1] += B00000100;
    digitsRight[5] += B00000100;
    return;
  } if (points == 4) {
    digitsLeft[7] += B00000100;
    return;
  } if (points == 5) {
    digitsLeft[7] +=B00100000;
    return;
  } if (points == 6) {
    digitsLeft[7] += B01001001;
    return;
  } if (points ==7) {
    digitsLeft[7] += B10010010;
    return;
  }

}

inline void setScore(){

  if (scoreOnOff == 0) {
    setNumberToTarget(turnDisplayOff, topRightDisplay);
    setNumberToTarget(turnDisplayOff, topRightDisplay);

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
    setNumberToTarget(scoreHome, topLeftDisplay);
    setNumberToTarget(scoreGuest, topRightDisplay);
  }
  if (kabuMode){
    setNumberToTarget(kabuScore1, topLeftDisplay);
    setNumberToTarget(kabuScore2, topRightDisplay);
    setNumberToTarget(kabuScore3,bottomLeftDisplay);
    setNumberToTarget(kabuScore4, bottomRightDisplay);
  }
}

inline void get_weather(){

    if (millis() - timeWeather > updateIntervallWeather * oneMinuteinMillis ) {
      if (dispTemp){
        if (WiFi.status() == WL_CONNECTED){
 
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
          timeWeather = millis();
        }
      }
    }
}

inline void resetAll() {
  for (byte i = 0; i < 8; i++)
    {
      digitsLeft[i]=0;
      digitsRight[i]=0;
    }
}

inline void mainLoop(int hours, int minutes, int seconds) {

  get_weather();

  resetAll();

  setNumberToTarget(hours, hoursDisplay);
  setNumberToTarget(minutes, minutesDisplay);
  setPoints(middleMiddlePoints);

  if (dispSeconds){
    setNumberToTarget(seconds, bottomRightDisplay);
  }
  if (dispTemp){
    setNumberToTarget(temp, bottomLeftDisplay);
  } 

  setScore();
}