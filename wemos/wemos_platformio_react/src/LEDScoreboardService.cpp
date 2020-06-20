#include <LEDScoreboardService.h>

LEDScoreboardService::LEDScoreboardService(AsyncWebServer* server,
                                     SecurityManager* securityManager
                                    ) :
    _httpEndpoint(LEDScoreboardState::read,
                  LEDScoreboardState::update,
                  this,
                  server,
                  LEDSCOREBOARD_SETTINGS_ENDPOINT_PATH,
                  securityManager,
                  AuthenticationPredicates::IS_AUTHENTICATED),
    _webSocket(LEDScoreboardState::read,
               LEDScoreboardState::update,
               this,
               server,
               LEDSCOREBOARD_SETTINGS_SOCKET_PATH,
               securityManager,
               AuthenticationPredicates::IS_AUTHENTICATED){

  // configure settings service update handler to update LED state
  addUpdateHandler([&](const String& originId) { onConfigUpdated(); }, false);
}

// 7-Segment Codes for "0"-"9"
 const char LEDScoreboardState::numbers[10]= 
  {
    0xBB,
    0x18,
    0xEA,
    0x7A,
    0x59,
    0x73,
    0xF3,
    0x1A,
    0x7B,   
    0xFB,
  };
void LEDScoreboardService::begin() {
  std::copy(LEDScoreboardState::numbers, LEDScoreboardState::numbers + 8, _state.digitsLeft);
  std::copy(LEDScoreboardState::numbers, LEDScoreboardState::numbers + 8, _state.digitsRight);
  onConfigUpdated();
}

void LEDScoreboardService::onConfigUpdated() {
  //digitalWrite(BLINK_LED, _state.ledOn ? LED_ON : LED_OFF);
}

void LEDScoreboardService::registerConfig() {

  String configTopic;
  String subTopic;
  String pubTopic;

  DynamicJsonDocument doc(256);

  doc["cmd_t"] = "~/set";
  doc["stat_t"] = "~/state";
  doc["schema"] = "json";
  doc["brightness"] = false;

  String payload;
  serializeJson(doc, payload);
}
