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


    setSegs(0,0);
    setDigits(0);
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
    shiftOut(shiftSegDataPin, shiftSegClkPin, LSBFIRST, left);  
    shiftOut(shiftSegDataPin, shiftSegClkPin, LSBFIRST, right);  

    digitalWrite(shiftSegLatchPin, HIGH);
}

void setSegs1(byte left){
    digitalWrite(shiftSegLatchPin, LOW);
    // shift out the 16 bits:
    shiftOut(shiftSegDataPin, shiftSegClkPin, LSBFIRST, left);  

    digitalWrite(shiftSegLatchPin, HIGH);
}


void setDigits(byte value){
    digitalWrite(shiftDigitLatchPin, LOW);
    shiftOut(shiftDigitDataPin, shiftDigitClkPin, LSBFIRST, value);
    digitalWrite(shiftDigitLatchPin, HIGH);
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

void segsMapping(){
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

void digitsMap(){
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

void onOff(){
  for(int i=0;i<8;i++){
    if(i==0){
      setDigits(255);
      setSegs(255,0);
    }else if(i==1){
      setDigits(255);
      setSegs(0,255);  
    }
    else{
      setDigits(0);
      setSegs(0,0);  
    }
    delayMicroseconds(1800);
  } 
}

void setDisplay(byte digit, byte seg){
  
}
 
byte getSegsLeft(int digit){
  if(digit==0){
    return 1;
  }
  return 255;
 }
byte getSegsRight(int digit){
  if(digit==6){
    return 1;
  }
  return 255;
 }

 void testMulti(){
    byte val=1;
  for(int i=0;i<8;i++){
     setSegs(0,0);
     setDigits(val);
     val=val<<1;
     setSegs(getSegsRight(i),getSegsLeft(i));
     delayMicroseconds(1700);
  }
}
volatile byte numbers[]= {
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

volatile int count=0;
volatile int num=0;
void testNumbers(){
      volatile byte val=1;
      for(int i=0;i<8;i++){
         setSegs(0,0);
         setDigits(val);
         val=val<<1;
         setSegs(numbers[num],numbers[num]);
         delayMicroseconds(1700);
      }
      count++;
      if(count > 25){
          num++; 
          if(num> 9){
            num=0;
          }
          count=0;
      }
 }

void loop() {
  testNumbers();

}
