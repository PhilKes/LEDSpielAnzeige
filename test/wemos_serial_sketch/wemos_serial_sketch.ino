int shiftDigitDataPin=D1;
int shiftDigitClkPin=D3;
int shiftDigitLatchPin=D2;

int shiftSegDataPin=D0;
int shiftSegClkPin=D6;
int shiftSegLatchPin=D5;

byte digitData=B00000001;
 
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(shiftDigitDataPin, OUTPUT);
  pinMode(shiftDigitClkPin, OUTPUT);
  pinMode(shiftDigitLatchPin, OUTPUT);

  pinMode(shiftSegDataPin, OUTPUT);
  pinMode(shiftSegClkPin, OUTPUT);
  pinMode(shiftSegLatchPin, OUTPUT);
 //initDigits();


  
  Serial.begin(115200);
  Serial.println("Ready");
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

void setSegs1(int left){
    digitalWrite(shiftSegLatchPin, LOW);
    delayMicroseconds(10);
    // shift out the 16 bits:
    shiftOut(shiftSegDataPin, shiftSegClkPin, MSBFIRST, left&255);  

    digitalWrite(shiftSegLatchPin, HIGH);
    delayMicroseconds(100);
}


void setDigits(int value){
    digitalWrite(shiftDigitLatchPin, LOW);
    delayMicroseconds(10);
    shiftOut(shiftDigitDataPin, shiftDigitClkPin, MSBFIRST, value&255);
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
String s; 
int mod=0;
int del=5;
void loop() {
  while(Serial.available()) {
  
    s= Serial.readString();// read the incoming data as string
    int idx = s.indexOf('=');

    if(idx==-1){
      break;
    }
    int val = s.substring(idx+1).toInt();
    if(s.charAt(0)=='m'){
      mod=1;
      del=val;     
    }else{
      del=5;
      mod=0;
    }
    
    if(s.charAt(0)=='s'){
      setSegs1(val);
    }
    else if(s.charAt(0)=='d'){
      setDigits(val);
    }
    Serial.println(s);
    Serial.flush();
  }
  if(mod==1){
    while(Serial.available()<1){
      setDigits(0);
      setSegs1(255);
      delay(del);
      setSegs1(0);
     setDigits(255);
      delay(del);
    }
  }
}
