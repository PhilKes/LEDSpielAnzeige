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

String readString = String(100); 

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

  if (cmd_client){
  while(cmd_client.connected())
  {
    //multiplexLoop(multiplexDelay);

    char c = cmd_client.read();
    if (readString.length() < 100) {
      readString = readString + c; 
      }
    if (c == '\n') {  
      if(readString.indexOf("3=Up") > -1) {
        scoreHome = scoreHome +1;
      }
      if(readString.indexOf("3=Down") > -1){
        scoreHome = scoreHome -1;
      }
      if(readString.indexOf("4=Up") > -1) {
        scoreGuest = scoreGuest +1;
      }
      if(readString.indexOf("4=Down") > -1){
        scoreGuest = scoreGuest -1;
      }
      if(readString.indexOf("all=Scores+off") > -1){
        scoreOnOff = 0;
      }
      if(readString.indexOf("all=Scores+on") > -1){
        scoreOnOff = 1;
        scoreHome = 0;
        scoreGuest = 0;
      }
      if(readString.indexOf("all=Display+Seconds+On+or+Off") > -1){
        if (dispSeconds){
          dispSeconds = false;
        } else {
          dispSeconds = true;
        }
      }
      if(readString.indexOf("all=Display+Temperature+On+or+Off") > -1){
        if (dispTemp){
          dispTemp = false;
        } else {
          dispTemp = true;
        }
      }      
      if (readString.indexOf("quantityHome") > -1){
        int index = readString.indexOf("quantityHome");
        String num = readString.substring(index+13);
        scoreHome = scoreHome + num.toInt();
      }
      if (readString.indexOf("quantityGuest") > -1){
        int index = readString.indexOf("quantityGuest");
        String num = readString.substring(index+14);
        scoreGuest = scoreGuest + num.toInt();
      }

      cmd_client.println("HTTP/1.1 200 OK");
      cmd_client.println("Content-Type: text/html");
      cmd_client.println();
      cmd_client.print("<html><head>");
      cmd_client.print("<title>Arduino Webserver Anzeige</title>");
      cmd_client.println("</head>");
      cmd_client.print("<body bgcolor='#444444'>");
      //---Überschrift---
      cmd_client.println("<br><hr />");
      cmd_client.println("<h1><div align='center'><font color='#2076CD'>Arduino Webserver for Score</font color></div></h1>");
      cmd_client.println("<hr /><br>");
      cmd_client.println("<div align='left'><font face='Verdana' color='#FFFFFF'>Scores:</font></div>");
      cmd_client.println("<form method=get><input type=submit name=all value='Scores on'></form>");
      cmd_client.println("<form method=get><input type=submit name=all value='Scores off'></form>");
      cmd_client.println("<br>");
      cmd_client.println("<table border='1' width='500' cellpadding='5'>");
      cmd_client.println("<tr bgColor='#222222'>");
      cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>Home<br></font></td>");
      cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=3 value='Up'></form></td>");
      cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=3 value='Down'></form></td>");
      cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=number name=quantityHome min='-10' max ='100'></form></td>");      
      cmd_client.println("</tr>");
      cmd_client.println("<tr bgColor='#222222'>");
      cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>Guest<br></font></td>");
      cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=4 value='Up'></form></td>");
      cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=4 value='Down'></form></td>");
      cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=number name=quantityGuest min='-10' max ='100'></form></td>"); 
      cmd_client.println("</tr>");
      cmd_client.println("</tr>");
      cmd_client.println("</table>");
      cmd_client.println("<br>");
      cmd_client.println("<form method=get><input type=submit name=all value='Display Seconds On or Off'></form>");
      cmd_client.println("<form method=get><input type=submit name=all value='Display Temperature On or Off'></form>");
      cmd_client.println("</body></html>");
      readString="";

      cmd_client.stop();
    } 
    multiplexLoop(multiplexDelay);
  }}

  if (multiplexCount% 50 == 0) {
    timeClient.update();

    minutes = timeClient.getMinutes();
    hours = timeClient.getHours();
    seconds = timeClient.getSeconds();
    
    show_time(hours, minutes, seconds); 
    
  } 

  
}