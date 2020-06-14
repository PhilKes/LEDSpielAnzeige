#include <modes.h>

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// WiFi Settings
const char* ssid = "Network Name";
const char* password = "WiFi Password";
const char* host = "Wemos_OTA";

//DEBUG true, for USB debugging
#define DEBUG true
//OTA_ENABLED true, for Over The Air Updates (WiFi sketch upload)
#define OTA_ENABLED false

int shiftDigitDataPin=D1;
int shiftDigitClkPin=D3;
int shiftDigitLatchPin=D2;

int shiftSegDataPin=D0;
int shiftSegClkPin=D6;
int shiftSegLatchPin=D5;

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

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
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
}

int mode=0;

void loop() {
  #if OTA_ENABLED
  ArduinoOTA.handle();
  #endif

  multiplexLoop(multiplexDelay);  //multiplex Digits, load current Segment values
  numbersLoop(500);               //set current Segment values in selected mode (digitsLeft,digitsRight)
}
