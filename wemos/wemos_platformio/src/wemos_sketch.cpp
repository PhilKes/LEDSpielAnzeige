#include <modes.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <wifi_settings.h>
#include <ESP8266WiFiMulti.h>
#include <NTPClient.h>


//DEBUG true, for USB debugging
#define DEBUG false
//OTA_ENABLED true, for Over The Air Updates (WiFi sketch upload)
#define OTA_ENABLED true

ESP8266WiFiMulti WiFiMulti;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"europe.pool.ntp.org", 7200, 60000);
const int port = 8181;
WiFiServer cmd_server(8181);

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

void segsMapping()
{
  while(true){
    setDigits(255);
    byte val=1;
    while(val>0){
       setSegs(val,val);
       val=val<<1;
       delay(1000);
    }
    delay(3000);
  }  
}

void digitsMap()
{
  while(true){
    setSegs(255,255);
    byte val=1;
    while(val>0){
       setDigits(val);
       val=val<<1;
       delay(1000);
    }
    delay(2500);
  }  
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
      setSegs(digitsLeft[displayIdx],digitsRight[displayIdx]); //Load Segments for new Digits
      delayMicroseconds(multiDel);
   }while(digitData!=B10000000);
   multiplexCount++;
 }

inline char *handleCmd(uint8_t cmd) {
  
  switch(cmd) {

    case 50:
    scoreGuest = scoreGuest + 1;
    return "up guest \n";

    case 49:
    scoreGuest = scoreGuest-1;
    return "down guest \n";

    case 51:
    scoreHome = scoreHome +1;
    return "up home \n";
    
    case 52:
    scoreHome = scoreHome -1;
    return "down home \n";

    case 97:
    scoreOnOff = 1;
    scoreHome = 0;
    scoreGuest =0;
    return "on \n";

    case 100:
    scoreOnOff = 0;
    return "off \n";

    default:
    return "";
  }

}



// Set pinModes + init Digits&Segments
 void setup() 
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

  #if DEBUG
  Serial.begin(115200);
  #endif

  //Serial.begin(9600);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFiMulti.addAP(ssid, password);
    WiFi.begin(ssid, password);
    #if DEBUG
    Serial.println("Retrying connection...");
    #endif
  }
  #if OTA_ENABLED
  ArduinoOTA.setHostname(host);
  ArduinoOTA.onStart([]() { // switch off all Digits&Segments during upgrade
    initDigits();
    initSegs();
    digitData=B00000001;
    displayIdx=1;
  });
  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
    flashLEDBuiltIn();
  });

  ArduinoOTA.onError([](ota_error_t error) {
    (void)error;
    ESP.restart();
  });

  /* setup the OTA server */
  ArduinoOTA.begin();
  #endif
  #if DEBUG
  Serial.println("Ready");
  #endif

  flashLEDBuiltIn();
  timeClient.begin();
  timeClient.setUpdateInterval(300000);
  cmd_server.begin();

}


void loop() {

  #if OTA_ENABLED
  ArduinoOTA.handle();
  #endif
  WiFiClient cmd_client = cmd_server.available();
  
 

  
  multiplexLoop(multiplexDelay);  //multiplex Digits, load current Segment values

  while(cmd_client.connected()){
    multiplexLoop(multiplexDelay);
    if(cmd_client.available()) {
      
      uint8_t buf;
      size_t length =1;
      cmd_client.read(&buf, length);
      cmd_client.write(handleCmd(buf));
      cmd_client.stop();
    } 
    multiplexLoop(multiplexDelay);
  }

  if (multiplexCount% 50 == 0) {
    /**TODO
    //save_current_disp
    then do update
    refresh
    */
    //setScore();
    timeClient.update();

    minutes = timeClient.getMinutes();
    hours = timeClient.getHours();
    seconds = timeClient.getSeconds();
    
    show_time(hours, minutes, seconds); 
    
  } 

  
}