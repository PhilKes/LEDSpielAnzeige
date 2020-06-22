# LED Scoreboard - ESP8266 React

[Main Project Page .tsx](./src/project/LEDScoreboard)

## LED Scoreboard Project Routes:
_LEDScoreboard.tsx_:

    <Switch>
      <AuthenticatedRoute exact path={`/${PROJECT_PATH}/rest`} component={LightStateRestController} />
      <AuthenticatedRoute exact path={`/${PROJECT_PATH}/socket`} component={LightStateWebSocketController} />
      <Redirect to={`/${PROJECT_PATH}/rest`} />
    </Switch>

## REST Controller
HTTP Request to control WeMos


