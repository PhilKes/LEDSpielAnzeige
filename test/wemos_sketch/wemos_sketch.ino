//#include <ESP8266WiFi.h>
//#include <ESP8266mDNS.h>
//#include <WiFiUdp.h>

//const char* ssid = "WLAN Ke";             //!!!!!!!!!!!!!!!!!!!!! modify this
//const char* password = "3616949541664967";                //!!!!!!!!!!!!!!!!!!!!!modify this

int shiftDigitDataPin=D1;
int shiftDigitClkPin=D3;
int shiftDigitLatchPin=D2;

int shiftSegDataPin=D0;
int shiftSegClkPin=D6;
int shiftSegLatchPin=D5;

byte digitData=B00000001;

//WiFiServer server(80);
 
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(shiftDigitDataPin, OUTPUT);
  pinMode(shiftDigitClkPin, OUTPUT);
  pinMode(shiftDigitLatchPin, OUTPUT);

  pinMode(shiftSegDataPin, OUTPUT);
  pinMode(shiftSegClkPin, OUTPUT);
  pinMode(shiftSegLatchPin, OUTPUT);
 //initDigits();


  
   digitalWrite(LED_BUILTIN, HIGH);
   delay(500);
   digitalWrite(LED_BUILTIN, LOW);
   delay(500);
   digitalWrite(LED_BUILTIN, HIGH);


}

//Init all Digits to 1st On, all other off
void initDigits(){
  setDigits(255);
}

//Init all Segments to on
void initSegs(){
  setSegs(255,255);
}

// ShiftOut Segment values
void setSegs(byte left,byte right){
    digitalWrite(shiftSegLatchPin, LOW);
    // shift out the 16 bits:
    shiftOut(shiftSegDataPin, shiftSegClkPin, MSBFIRST, left);  
    shiftOut(shiftSegDataPin, shiftSegClkPin, MSBFIRST, right);  

    digitalWrite(shiftSegLatchPin, HIGH);
     delayMicroseconds(100);
}

void setSegs1(byte left){
    digitalWrite(shiftSegLatchPin, LOW);
    delayMicroseconds(10);
    // shift out the 16 bits:
    shiftOut(shiftSegDataPin, shiftSegClkPin, MSBFIRST, left);  

    digitalWrite(shiftSegLatchPin, HIGH);
    delayMicroseconds(100);
}


void setDigits(byte value){
    digitalWrite(shiftDigitLatchPin, LOW);
    delayMicroseconds(10);
    shiftOut(shiftDigitDataPin, shiftDigitClkPin, MSBFIRST, value);
    digitalWrite(shiftDigitLatchPin, HIGH);
    delayMicroseconds(100);
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
    delayMicroseconds(100);
    
}
 
void loop() {
  setSegs1(255);
  while(true){
     for(int i=0;i<8;i++){
        if(i==0){
          setDigits(255);
        }else{
           setDigits(0);
        }
        delayMicroseconds(100);
     }
  }
}
