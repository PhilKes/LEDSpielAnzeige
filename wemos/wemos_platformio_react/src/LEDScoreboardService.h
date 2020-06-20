#ifndef LEDScoreboardService_h
#define LEDScoreboardService_h

#include <HttpEndpoint.h>
#include <WebSocketTxRx.h>

// Note that the built-in LED is on when the pin is low on most NodeMCU boards.
// This is because the anode is tied to VCC and the cathode to the GPIO 4 (Arduino pin 2).
#ifdef ESP32
#define LED_ON 0x1
#define LED_OFF 0x0
#elif defined(ESP8266)
#define LED_ON 0x0
#define LED_OFF 0x1
#endif

#define LEDSCOREBOARD_SETTINGS_ENDPOINT_PATH "/rest/ledScoreboard"
#define LEDSCOREBOARD_SETTINGS_SOCKET_PATH "/ws/ledScoreboard"

class LEDScoreboardState {
  
 public:
  static const char numbers[10];

//current Display data of all 8 left Displays
char digitsLeft[8]={numbers[8]}; 


//current Display data of all 8 right Displays
 char digitsRight[8]={numbers[8]};

  static void read(LEDScoreboardState& settings, JsonObject& root) {
    const JsonArray& jsonDigitsLeft= root.createNestedArray("digitsLeft");
    const JsonArray& jsonDigitsRight= root.createNestedArray("digitsRight");
    for (size_t i = 0; i < 8; i++){
      jsonDigitsLeft.add(settings.digitsLeft[i]);
      jsonDigitsRight.add(settings.digitsRight[i]);
    }
    
    //root["digitsLeft"] = settings.digitsLeft;
    //root["digitsRight"] = settings.digitsRight;
  }

  static StateUpdateResult update(JsonObject& root, LEDScoreboardState
& ledScoreboardState) {
    //char defaultDigits[8] ={numbers[8]};
    if(!root["digitsLeft"] || !root["digitsRight"] ){
      return StateUpdateResult::UNCHANGED;
    }
    const JsonArray& jsonDigitsLeft= root["digitsLeft"].as<JsonArray>();
    const JsonArray& jsonDigitsRight= root["digitsRight"].as<JsonArray>();
    for (size_t i = 0; i < 8; i++){
      ledScoreboardState.digitsLeft[i]=jsonDigitsLeft[i].as<char>();
      ledScoreboardState.digitsRight[i]=jsonDigitsRight[i].as<char>();
    }
    
    //ledScoreboardState.digitsLeft = root["digitsLeft"] ;
    //ledScoreboardState.digitsRight = root["digitsRight"];

    return StateUpdateResult::CHANGED;
    /*if (lightState.ledOn != newState) {
      lightState.ledOn = newState;
      return StateUpdateResult::CHANGED;
    }
    return StateUpdateResult::UNCHANGED;*/
  }

  static void haRead(LEDScoreboardState
& settings, JsonObject& root) {
    //root["digitsLeft"] = settings.digitsLeft;
    //root["digitsRight"] = settings.digitsRight;
    const JsonArray& jsonDigitsLeft= root.createNestedArray("digitsLeft");
    const JsonArray& jsonDigitsRight= root.createNestedArray("digitsRight");
    for (size_t i = 0; i < 8; i++){
      jsonDigitsLeft.add(settings.digitsLeft[i]);
      jsonDigitsRight.add(settings.digitsRight[i]);
    }
  }

  static StateUpdateResult haUpdate(JsonObject& root, LEDScoreboardState
& ledScoreboardState) {

    //ledScoreboardState.digitsLeft=root["digitsLeft"];
    //ledScoreboardState.digitsRight=root["digitsRight"];

    const JsonArray& jsonDigitsLeft= root["digitsLeft"].as<JsonArray>();
    const JsonArray& jsonDigitsRight= root["digitsRight"].as<JsonArray>();
    for (size_t i = 0; i < 8; i++){
      ledScoreboardState.digitsLeft[i]=jsonDigitsLeft[i].as<char>();
      ledScoreboardState.digitsRight[i]=jsonDigitsRight[i].as<char>();
    }
    return StateUpdateResult::CHANGED;
    /*
    String state = root["state"];
    // parse new led state 
    boolean newState = false;
    if (state.equals(ON_STATE)) {
      newState = true;
    } else if (!state.equals(OFF_STATE)) {
      return StateUpdateResult::ERROR;
    }
    // change the new state, if required
    if (lightState.ledOn != newState) {
      lightState.ledOn = newState;
      return StateUpdateResult::CHANGED;
    }
    return StateUpdateResult::UNCHANGED;*/
  }
};

class LEDScoreboardService : public StatefulService<LEDScoreboardState> {
 public:
  LEDScoreboardService(AsyncWebServer* server,
                    SecurityManager* securityManager);
  void begin();

 private:
  HttpEndpoint<LEDScoreboardState> _httpEndpoint;
  WebSocketTxRx<LEDScoreboardState> _webSocket;

  void registerConfig();
  void onConfigUpdated();
};

#endif
