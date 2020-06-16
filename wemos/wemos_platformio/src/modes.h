#include <Arduino.h>
#include <NTPClient.h>
#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
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
volatile int second_sec;
volatile int first_sec;
  
volatile bool negativeTemp;
volatile int second_temp;
volatile int first_temp;

volatile int second_score_home;
volatile int first_score_home;
volatile int second_score_guest;
volatile int first_score_guest;

volatile int scoreOnOff = 0;
volatile int scoreGuest = 0;
volatile int scoreHome = 0;

inline void setScore(){

  if (scoreOnOff == 0) {
    digitsLeft[5] = 0;
    digitsLeft[1] = 0;

    digitsLeft[0] = 0;
    digitsLeft[4] = 0;

  } else {
    
    if (scoreHome<0 && scoreHome > -10){
    digitsLeft[0] = B01000000;
    digitsLeft[4] = numbers[scoreHome * -1];
  } if (scoreHome<10 && scoreHome > -1){
    digitsLeft[4] = numbers[scoreHome];
    digitsLeft[0] = 0;
  } if (scoreHome>9 && scoreHome <100) {
    second_score_home = scoreHome %10;
    first_score_home = (scoreHome-second_score_home)/10; 
    digitsLeft[4] = numbers[second_score_home];
    digitsLeft[0] = numbers[first_score_home];
  } if (scoreHome>99 && scoreHome <200) {
    scoreHome = scoreHome -100;
    second_score_home = scoreHome %10;
    first_score_home = (scoreHome-second_score_home)/10; 
    scoreHome = scoreHome +100;
    digitsLeft[4] = numbers[second_score_home] + B00000100;
    digitsLeft[0] = numbers[first_score_home] + B00000100;

  } if (scoreHome>199) {
    digitsLeft[4] = numbers[9] + B00000100;
    digitsLeft[0] = numbers[9] + B00000100;
    scoreHome = 199;

  } if (scoreHome < -11) {
    scoreHome = 0;
    digitsLeft[4] = numbers[scoreHome];
    digitsLeft[0] = numbers[scoreHome];
  }

  if (scoreGuest<0 && scoreGuest > -10){
    digitsLeft[1] = B01000000;
    digitsLeft[5] = numbers[scoreGuest * -1];
  } if (scoreGuest<10 && scoreGuest > -1){
    digitsLeft[5] = numbers[scoreGuest];
    digitsLeft[1] = 0;
  } if (scoreGuest>9 && scoreGuest <100) {
    second_score_guest = scoreGuest %10;
    first_score_guest = (scoreGuest-second_score_guest)/10; 
    digitsLeft[5] = numbers[second_score_guest];
    digitsLeft[1] = numbers[first_score_guest];
  } if (scoreGuest>99 && scoreGuest <200) {
    scoreGuest = scoreGuest -100;
    second_score_guest = scoreGuest %10;
    first_score_guest = (scoreGuest-second_score_guest)/10; 
    scoreGuest = scoreGuest +100;
    digitsLeft[5] = numbers[second_score_guest] + B00000100;
    digitsLeft[1] = numbers[first_score_guest] + B00000100;

  } if (scoreGuest>199) {
    digitsLeft[5] = numbers[9] + B00000100;
    digitsLeft[1] = numbers[9] + B00000100;
    scoreGuest = 199;
  } if (scoreGuest < -11) {
    scoreGuest = 0;
    digitsLeft[5] = numbers[scoreGuest];
    digitsLeft[1] = numbers[scoreGuest];
  }
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
      // Parse JSON object
      //JsonObject& root = jsonBuffer.parseObject(payload);

      //if (!root.success()) {
        //Serial.println(F("Parsing failed!"));
        //return;
      //}
 
      temp = (int)(root["main"]["temp"]) ;        // get temperature
    }
 
    http.end();   //Close connection
 


}



inline void show_time(int hours, int minutes, int seconds) {
  
  second_min = minutes % 10;
  first_min = (minutes - second_min)/10;

  second_h = hours%10;
  first_h = (hours - second_h)/10;

  second_sec = seconds %10;
  first_sec = (seconds-second_sec)/10;

  if (temp<0) {
    temp = -temp;
    second_temp = temp %10;
    first_temp = (temp-second_temp)/10;
    temp = -temp;
    negativeTemp = true;
  } else {
    second_temp = temp %10;
    first_temp = (temp-second_temp)/10; 
  }
  

  if (multiplexCount == 200 ) {
    if (dispTemp){
      get_weather();
    }
  }

  if (multiplexCount>180000){
    multiplexCount =0;
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
      digitsRight[1] = numbers[first_sec];
      digitsRight[5] = numbers[second_sec];
    } else {
      digitsRight[1] = 0;
      digitsRight[5] = 0;
    }

    if (dispTemp){
      if (negativeTemp){
        digitsRight[3] = B01000000;
        negativeTemp = false;
      } else {
        digitsRight[3] = 0;
      }
      digitsRight[4] = numbers[second_temp];
      digitsRight[0] = numbers[first_temp];
    } else {
      digitsRight[4] = 0;
      digitsRight[0] = 0;      
    }

    setScore();
}