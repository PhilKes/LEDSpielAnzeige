# Wemos_react_platformio

## Getting Started
 #### Settings
 1. Make sure in [/factory_settings](./factory_settings.ini) WiFi Access Point is enabled:
`-D FACTORY_AP_PROVISION_MODE=AP_MODE_ALWAYS`
 2. [Build & Upload](#building--uploading) the Project
 3. Connect to *ESP8266-React* WiFi network via Phone

#### WiFi
 1. WiFi Connection-> Scan Networks -> Connect to local WiFi
 2. Preferably set a Static IP for Local Development (see [.env.development](./interface/.env.development))

 #### NTP
1. Network Time -> NTP Settings
2. Select correct Time Zone -> Save
 
### Building & Uploading
![Build+Upload](../../doc/platformio/upload_screen.png)

To Enable/Disable OTA Upload [platformio.ini](./platformio.ini)):

`; Uncomment & modify the lines below in order to configure OTA updates
;upload_protocol=espota
;upload_port = 192.168.178.55
;upload_auth=esp-react`

#### Debugging
in [main.cpp](./src/main.cpp) and [wemos_sketch.h](./src/wemos/wemos_sketch.h):

`#define DEBUG true`

to enable Debugging via Serial-Connection (USB + `Serial.print`)

Default Environment: d1_mini

# Code
## Wemos Backend Sketch
[main.cpp](./src/main.cpp)

Runs ESP8266 React Web Server + Wemos Sketch Loop

[wemos_sketch.h](./src/wemos_sketch.h):

`wemosLoop(digitsLeft,digitsRight,ntpStatus)`

* Executes Multiplex with current digits as Segment Values (`digitsLeft` & `digitsRight` changed by HTTP Requests from WebServer)
* `ntpStatus` can be used to get current Time (`setTime(ntpStatus->getTime())`)

[LEDScoreboardService](./src/LEDScoreboardService.h)

* Receives state (`digitsLeft` & `digitsRight`) updates from frontend HTTP requests via ArduinoJson Objects
* Accesses `NTPStatus` to get current LocalTime

---

## Wemos React Frontend App

[LEDScoreboardRestController.tsx](./interface/src/project/LEDScoreboardRestController.tsx)

Subpage for sending HTTP Requests to control `digitsLeft` & `digitsRight`

`LEDScoreboardRestControllerForm` (React component for Inputs to change digit Values / modes)

Given component `props`:
* `data` : corresponds to [LEDScoreboardState in backend](./src/LEDScoreboardService.h)
* `loadData()` : Fetches state from backend
* `handleValueChange(String propertyKey)` : Sets value of given property
* `saveData()` : Sends updated state to backend


---

#### Libraries Used

* [React](https://reactjs.org/)
* [Material-UI](https://material-ui.com/)
* [notistack](https://github.com/iamhosseindhv/notistack)
* [ArduinoJson](https://github.com/bblanchon/ArduinoJson)
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
* [AsyncMqttClient](https://github.com/marvinroger/async-mqtt-client)
