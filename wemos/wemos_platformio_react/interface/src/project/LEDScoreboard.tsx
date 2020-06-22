import React, { Component } from 'react';
import { Redirect, Switch, RouteComponentProps } from 'react-router-dom'

import { Tabs, Tab } from '@material-ui/core';

import { PROJECT_PATH } from '../api';
import { MenuAppBar } from '../components';
import { AuthenticatedRoute } from '../authentication';

import LightStateRestController from './LightStateRestController';
import LightStateWebSocketController from './LightStateWebSocketController';
import LEDScoreboardRestController from './LEDScoreboardRestController';

class LEDScoreboard extends Component<RouteComponentProps> {

  handleTabChange = (event: React.ChangeEvent<{}>, path: string) => {
    this.props.history.push(path);
  };

  render() {
    return (
      <MenuAppBar sectionTitle="LED Scoreboard">
        <Tabs value={this.props.match.url} onChange={this.handleTabChange} variant="fullWidth">
        <Tab value={`/${PROJECT_PATH}/led`} label="LEDScoreboard Controller" />
          <Tab value={`/${PROJECT_PATH}/rest`} label="REST Controller" />
          <Tab value={`/${PROJECT_PATH}/socket`} label="WebSocket Controller" />
        </Tabs>
        <Switch>
        <AuthenticatedRoute exact path={`/${PROJECT_PATH}/led`} component={LEDScoreboardRestController} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/rest`} component={LightStateRestController} />
          <AuthenticatedRoute exact path={`/${PROJECT_PATH}/socket`} component={LightStateWebSocketController} />
          <Redirect to={`/${PROJECT_PATH}/led`} />
        </Switch>
      </MenuAppBar>
    )
  }

}

export default LEDScoreboard;
