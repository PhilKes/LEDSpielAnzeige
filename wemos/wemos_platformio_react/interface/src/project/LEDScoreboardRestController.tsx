import React, { Component } from 'react';
import { ValidatorForm } from 'react-material-ui-form-validator';

import { Typography, Box, Checkbox,Select } from '@material-ui/core';
import SaveIcon from '@material-ui/icons/Save';

import { ENDPOINT_ROOT } from '../api';
import { restController, RestControllerProps, RestFormLoader, RestFormProps, FormActions, FormButton, SectionContent, BlockFormControlLabel } from '../components';

import { LEDScoreboardState } from './types';

export const LEDSCOREBOARD_ENDPOINT = ENDPOINT_ROOT + "ledScoreboard";

type LEDScoreboardRestControllerProps = RestControllerProps<LEDScoreboardState>;

class LEDScoreboardRestController extends Component<LEDScoreboardRestControllerProps> {

  componentDidMount() {
    this.props.loadData();
  }

  render() {
    return (
      <SectionContent title='LEDScoreboard' titleGutter>
        <RestFormLoader
          {...this.props}
          render={props => (
            <LEDScoreboardRestControllerForm {...props} />
          )}
        />
      </SectionContent>
    )
  }

}

export default restController(LEDSCOREBOARD_ENDPOINT, LEDScoreboardRestController);

type LEDScoreboardRestControllerFormProps = RestFormProps<LEDScoreboardState>;

function LEDScoreboardRestControllerForm(props: LEDScoreboardRestControllerFormProps) {
  const { data, saveData, loadData, handleValueChange } = props;
  return (
    <ValidatorForm onSubmit={saveData}>
      <BlockFormControlLabel
        control={
          <Select
            value={data.digitsLeft[0]}
            onChange={()=>handleValueChange('digitsLeft')}
            color="primary"
          />
        }
        label="Digits Right"
      />
      <BlockFormControlLabel
        control={
          <Select
            value={data.digitsRight[0]}
            onChange={()=>handleValueChange('digitsRight')}
            color="primary"
          />
        }
        label="Digits Right"
      />
      <FormActions>
        <FormButton startIcon={<SaveIcon />} variant="contained" color="primary" type="submit">
          Save
        </FormButton>
        <FormButton variant="contained" color="secondary" onClick={loadData}>
          Reset
        </FormButton>
      </FormActions>
    </ValidatorForm>
  );
}
