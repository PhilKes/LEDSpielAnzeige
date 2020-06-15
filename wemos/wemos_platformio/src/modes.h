#include <Arduino.h>
#include <NTPClient.h>
#include <SPI.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
WiFiClient client;

volatile byte temp = 99;

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


//int temp = 0;
int second_min;
int first_min;

int second_h;
int first_h;
int second_sec;
int first_sec;
  
int second_temp;
int first_temp;

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

  second_temp = temp %10;
  first_temp = (temp-second_temp)/10; 


  if (multiplexCount == 10 ) {
    get_weather();
  }

  if (multiplexCount>30000){
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

    digitsRight[1] = numbers[first_sec];
    digitsRight[5] = numbers[second_sec];


    digitsRight[4] = numbers[second_temp];
    digitsRight[0] = numbers[first_temp];

}
