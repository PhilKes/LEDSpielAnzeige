#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

const char* ssid = "WLAN Ke";             //!!!!!!!!!!!!!!!!!!!!! modify this
const char* password = "3616949541664967";                //!!!!!!!!!!!!!!!!!!!!!modify this

int shiftDigitDataPin=D0;
int shiftDigitClkPin=D3;
int shiftDigitLatchPin=D2;

int shiftSegDataPin=D1;
int shiftSegClkPin=D6;
int shiftSegLatchPin=D5;

byte digitData=B00000001;

WiFiServer server(80);
 
void setup() {
  Serial.begin(115200);
  delay(10);
 
 
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

// ShiftOut Segment values
void shiftSeg(unsigned int value){
    digitalWrite(shiftSegLatchPin, LOW);
    // shift out the 16 bits:
    shiftOut(shiftSegDataPin, shiftSegClkPin, MSBFIRST, (byte)value&255);  
    shiftOut(shiftSegDataPin, shiftSegClkPin, MSBFIRST, (byte)((value>>8)&255));  

    digitalWrite(shiftSegLatchPin, HIGH);
}

// Shift the "1" to the left, if last Digit reached, reset to first Digit
void shiftDigits(){
    digitalWrite(shiftDigitLatchPin, LOW);
    digitData=digitData<<1;
    if(digitData==0){
      digitData=1;
      digitalWrite(shiftDigitDataPin, HIGH);
    }
    else
    {
      digitalWrite(shiftDigitDataPin,LOW);
    }
    digitalWrite(shiftDigitClkPin,HIGH);
    digitalWrite(shiftDigitClkPin,LOW);
    
    digitalWrite(shiftSegLatchPin, HIGH);
}
 
void loop() {
    
 
}
