## Wemos PlatformIO Projects
 
 ## [wemos_platformio](./wemos_platformio)
 * **WiFi** + **OTA** for Upload
 * Test Modes for Mappings
 
 #### Multiplexing
  
 **_wemos_sketch.cpp_** :
 
     void loop() {
       ArduinoOTA.handle();          //Handles OTA Updates if available
 
       multiplexLoop(multiplexDelay);// Multiplexes all Digits (1 Left + 1 Right at a time)
       numbersLoop(1000);            //set digitsLeft[] and digitsRight Segment values to be displayed              
     }
 
 create **_wifi_settings.h_** in /src to add WiFi configuration:
 
     const char* ssid = "Network SSID";
     const char* password = "WiFi Password";
     const char* host = "Wemos_OTA";
 
 _**modes.h**_
 
 Different Mode Functions setting Segment values ( _digitsLeft[]_ + _digitsRight[]_)
 
 e.g Looping through Numbers 0-9:
 
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
     
 **Note**: _multiplexCount_ = Amount of multiplexLoop() calls for Timing

 
 ## [wemosplatformio_react](./wemos_react_platformio)
 * **WiFi** + **OTA** for Upload
 * React Web App over HTTP **WebServer**
 * User Interface to control Modes
 * **REST** Endpoints
 * WiFi Access Point for Debugging
 * **NTP** fo Time Watch mode

 ---
 