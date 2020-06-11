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

  Serial.begin(115200);
  delay(10);
 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  //Serial.println(ssid);
 
  //WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
 
  //while (WiFi.status() != WL_CONNECTED) {
 //   delay(500);
 //   Serial.print(".");
 // }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  //server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL : ");
  Serial.print("http://");
//  Serial.print(WiFi.localIP());
  Serial.println("/");

 //initDigits();
 //initSegs();
 Serial.println("Init complete");
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
}


void setDigits(byte value){
    digitalWrite(shiftDigitLatchPin, LOW);
    shiftOut(shiftDigitDataPin, shiftDigitClkPin, MSBFIRST, value);
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
}
 
void loop() {
  setDigits(0);
  setSegs(0,0);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3000);
  
  setSegs(255,255);
  setDigits(0);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3000);
  
  setSegs(255,255);
  setDigits(255);
  digitalWrite(LED_BUILTIN, LOW);
  delay(3000);
  
  setSegs(0,0);
  setDigits(255);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(3000);
  
}
