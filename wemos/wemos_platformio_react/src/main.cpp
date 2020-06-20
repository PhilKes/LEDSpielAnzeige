#include <ESP8266React.h>
#include <LightStateService.h>
//#include <FS.h>
#include <LittleFS.h>
#include <LEDScoreboardService.h>

#define SERIAL_BAUD_RATE 115200

AsyncWebServer server(80);
ESP8266React esp8266React(&server, &LittleFS);
LightStateService lightStateService = LightStateService(&server,
                                                        esp8266React.getSecurityManager());
LEDScoreboardService ledScoreboardService = LEDScoreboardService(&server,
                                                        esp8266React.getSecurityManager());

void setup() {
  // start serial and filesystem
  Serial.begin(SERIAL_BAUD_RATE);

  // start the file system (must be done before starting the framework)
#ifdef ESP32
  LittleFS.begin(true);
#elif defined(ESP8266)
  LittleFS.begin();
#endif

  // start the framework and demo project
  esp8266React.begin();

  // load the initial light settings
  lightStateService.begin();

  ledScoreboardService.begin();

  // start the server
  server.begin();
}

void loop() {
  // run the framework's loop function
  esp8266React.loop();
}
