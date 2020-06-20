#include <modes.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
//#include <wifi_settings.h>
#include <ESP8266WiFiMulti.h>
#include <NTPClient.h>


//DEBUG true, for USB debugging
#define DEBUG false
//OTA_ENABLED true, for Over The Air Updates (WiFi sketch upload)
#define OTA_ENABLED true

ESP8266WiFiMulti WiFiMulti;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"europe.pool.ntp.org", 3600, 60000);
const int port = 8181;
WiFiServer cmd_server(8181);

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

inline void showWebInterface() {
    WiFiClient cmd_client = cmd_server.available();
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
        if(readString.indexOf("all=Scores+On+or+Off") > -1){
          if (scoreOnOff) {
            scoreOnOff = 0;
          } else {
            scoreOnOff = 1;
            scoreHome = 0;
            scoreGuest = 0;
          }
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
            get_weather();
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
        if (readString.indexOf("brightness") > -1){
          int index = readString.indexOf("brightness");
          String num = readString.substring(index+11);
          dimBrightness = num.toInt();
          dimDelay = multiplexDelay - ((dimBrightness) * multiplexDelay/10) - 1;
        } 
        if(readString.indexOf("all=Set") > -1){
          
        }
        if(readString.indexOf("all=Kabu+mode+On+or+Off") > -1){
          if (kabuMode){
            kabuMode = false;
            setNames = false;
            player1 = "Player 1";
            player2 = "Player 2";
            player3 = "Player 3";
            player4 = "Player 4";
          } else {
            kabuMode = true;
            dispSeconds = false;
            dispTemp = false;
            scoreOnOff = false;
            kabuScore1 = 0;
            kabuScore2 = 0;
            kabuScore3 = 0;
            kabuScore4 = 0;
          }
        } 
        if (kabuMode) {    
          if (readString.indexOf("p1Points") > -1){
            int index = readString.indexOf("p1Points");
            String num = readString.substring(index+9);
            kabuScore1 = kabuScore1 + num.toInt();
          }
          if (readString.indexOf("p2Points") > -1){
            int index = readString.indexOf("p2Points");
            String num = readString.substring(index+9);
            kabuScore2 = kabuScore2 + num.toInt();
          }
          if (readString.indexOf("p3Points") > -1){
            int index = readString.indexOf("p3Points");
            String num = readString.substring(index+9);
            kabuScore3 = kabuScore3 + num.toInt();
          }
          if (readString.indexOf("p4Points") > -1){
            int index = readString.indexOf("p4Points");
            String num = readString.substring(index+9);
            kabuScore4 = kabuScore4 + num.toInt();
          }

          if (readString.indexOf("p1Name") > -1){
            int index = readString.indexOf("p1Name");
            player1 = readString.substring(index+7, readString.indexOf("HTTP"));
            playerNamesSet++;
          }
          if (readString.indexOf("p2Name") > -1){
            int index = readString.indexOf("p2Name");
            player2 = readString.substring(index+7, readString.indexOf("HTTP"));
            playerNamesSet++;
          }
          if (readString.indexOf("p3Name") > -1){
            int index = readString.indexOf("p3Name");
            player3 = readString.substring(index+7, readString.indexOf("HTTP"));
            playerNamesSet++;
          }
          if (readString.indexOf("p4Name") > -1){
            int index = readString.indexOf("p4Name");
            player4= readString.substring(index+7, readString.indexOf("HTTP"));
            setNames=true;
            playerNamesSet = 0;
          }
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
        cmd_client.println("<h1><div align='center'><font color='#2076CD'>Anzeigetafel Arduino</font color></div></h1>");
        cmd_client.println("<hr /><br>");
        cmd_client.println("<div align='left'><font face='Verdana' color='#FFFFFF'>Set the level of Brightness </font></div>");
        cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=range name=brightness min='0' max='10' value='"+String(dimBrightness)+"' onchange='this.form.submit()' step='1'></form></td>"); 
        cmd_client.println("<br>");  
        cmd_client.println("<form method=get><input type=submit name=all value='Scores On or Off'></form>");
        cmd_client.println("<br>");   
        if (scoreOnOff){     
          cmd_client.println("<div align='left'><font face='Verdana' color='#FFFFFF'>Scores:</font></div>");
          cmd_client.println("<table border='1' width='500' cellpadding='5'>");
          cmd_client.println("<tr bgColor='#222222'>");
          cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>Home: "+String(scoreHome)+"<br></font></td>");
          cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=3 value='Up'></form></td>");
          cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=3 value='Down'></form></td>");
          cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=number name=quantityHome min='-10' max ='100'></form></td>");      
          cmd_client.println("</tr>");
          cmd_client.println("<tr bgColor='#222222'>");
          cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>Guest: "+String(scoreGuest)+"<br></font></td>");
          cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=4 value='Up'></form></td>");
          cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=submit name=4 value='Down'></form></td>");
          cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=number name=quantityGuest min='-10' max ='100'></form></td>"); 
          cmd_client.println("</tr>");
          cmd_client.println("</tr>");
          cmd_client.println("</table>");
        }
        cmd_client.println("<br>");
        cmd_client.println("<form method=get><input type=submit name=all value='Kabu mode On or Off'></form>");
        cmd_client.println("<br>");
        if (kabuMode){
          if (setNames){
            cmd_client.println("<div align='left'><font face='Verdana' color='#FFFFFF'>Kabu scores:</font></div>");
            cmd_client.println("<br>");
            cmd_client.println("<table border='1' width='500' cellpadding='5'>");
            cmd_client.println("<tr bgColor='#222222'>");
            cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>"+player1+": "+String(kabuScore1)+"<br></font></td>");
            cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=number name=p1Points min='-2' max ='100'></form></td>");    

            cmd_client.println("</tr>");
            cmd_client.println("<tr bgColor='#222222'>");
            cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>"+player2+": "+String(kabuScore2)+"<br></font></td>");
            cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=number name=p2Points min='-2' max ='100'></form></td>"); 

            cmd_client.println("</tr>");
            cmd_client.println("<tr bgColor='#222222'>");
            cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>"+player3+": "+String(kabuScore3)+"<br></font></td>");
            cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=number name=p3Points min='-2' max ='100'></form></td>"); 

            cmd_client.println("</tr>");
            cmd_client.println("<tr bgColor='#222222'>");
            cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>"+player4+": "+String(kabuScore4)+"<br></font></td>");
            cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=number name=p4Points min='-2' max ='100'></form></td>"); 
            cmd_client.println("</table>");
          } else {
            if (playerNamesSet == 0) {
              cmd_client.println("<div align='left'><font face='Verdana' color='#FFFFFF'>Please enter Player name:</font></div>");
              cmd_client.println("<br>");
              cmd_client.println("<table border='1' width='500' cellpadding='5'>");
              cmd_client.println("<tr bgColor='#222222'>");
              cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>"+player1+" <br></font></td>");
              cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=text name=p1Name ></form></td>");  
              cmd_client.println("</table>");  
            } if (playerNamesSet ==1){
              cmd_client.println("<div align='left'><font face='Verdana' color='#FFFFFF'>Please enter Player name:</font></div>");
              cmd_client.println("<br>");
              cmd_client.println("<table border='1' width='500' cellpadding='5'>");
              cmd_client.println("<tr bgColor='#222222'>");
              cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>"+player2+"<br></font></td>");
              cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=text name=p2Name ></form></td>"); 
              cmd_client.println("</table>");
            } if (playerNamesSet ==2){
              cmd_client.println("<div align='left'><font face='Verdana' color='#FFFFFF'>Please enter Player name:</font></div>");
              cmd_client.println("<br>");
              cmd_client.println("<table border='1' width='500' cellpadding='5'>");
              cmd_client.println("<tr bgColor='#222222'>");
              cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>"+player3+"<br></font></td>");
              cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=text name=p3Name ></form></td>"); 
              cmd_client.println("</table>");
            } if (playerNamesSet ==3){
              cmd_client.println("<div align='left'><font face='Verdana' color='#FFFFFF'>Please enter Player name:</font></div>");
              cmd_client.println("<br>");
              cmd_client.println("<table border='1' width='500' cellpadding='5'>");
              cmd_client.println("<tr bgColor='#222222'>");
              cmd_client.println("<td bgcolor='#222222'><font face='Verdana' color='#CFCFCF' size='2'>"+player4+"<br></font></td>");
              cmd_client.println("<td align='center' bgcolor='#222222'><form method=get><input type=text name=p4Name ></form></td>"); 
              cmd_client.println("</table>");
            }
          }
        }
        cmd_client.println("<br>");
        cmd_client.println("<form method=get><input type=submit name=all value='Display Seconds On or Off'></form>");
        cmd_client.println("<form method=get><input type=submit name=all value='Display Temperature On or Off'></form>");
        cmd_client.println("<br>");

        cmd_client.println("</body></html>");
        readString="";

        cmd_client.stop();
      } 
      multiplexLoop(multiplexDelay);
    }}
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
  timeClient.setTimeOffset(isDaylightSaving());
  timeClient.setUpdateInterval(updateIntervallTime * 60000);
  cmd_server.begin();

}


void loop() {

  #if OTA_ENABLED
  ArduinoOTA.handle();
  #endif

  if (WiFi.status() == WL_CONNECTED){
    multiplexLoop(multiplexDelay);  //multiplex Digits, load current Segment values

    showWebInterface();

    if (multiplexCount% 30 == 0) {
      timeClient.update();

      minutes = timeClient.getMinutes();
      hours = timeClient.getHours();
      seconds = timeClient.getSeconds();
      
      mainLoop(hours, minutes, seconds); 
    } 
  } else {
    multiplexLoop(multiplexDelay);
    if (multiplexCount% 50 == 0) {
      minutes = timeClient.getMinutes();
      hours = timeClient.getHours();
      seconds = timeClient.getSeconds();   

      mainLoop(hours, minutes, seconds); 
    }
  }
}