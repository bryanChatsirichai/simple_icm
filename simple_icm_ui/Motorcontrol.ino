// ******** MOTOR CONTROL Functions **********
// MOTOR_STEPS 200 //
// RPM 1000
// MS_STEP 8 // default 1/8 microstep, 200*8 = 1600 microstepping
// CALI_ACCEL 400
// 1 step 1.8 degrees
/* 
 *  -- Calc RPM --
 *   
 * Calculate RPM by ratio
 * May not be needed since the speed
 * in AccelMotor is in steps/s
 */
float calcRPM(int ms_steps, float seconds) {
  // return (float)(steps * MS_STEP)/seconds;
  Serial.print("ms_steps");
  Serial.println(ms_steps);
  delay(500);
  return (float)(ms_steps)/seconds;
}

/*
 * Calculating the acceleration
 * - Requires some finetuning
 */
float calcAccel(int steps, float seconds) {
  Serial.print("steps");
  Serial.println(steps);
  delay(500);
  float max_speed = calcRPM(steps, seconds);
  return (float)max_speed / seconds;
}

/*
 * to MilliSeconds converter
 */
long toMS(float seconds) {
  return seconds * 1000;
}

/*
 * Move motor to desired location
 * - Given the position
 * - Move the motor to pos
 * - Return motor to pos
 *  orientation 0 direction
 *  orientation 1 opp-direction
 */

void moveMotor(int pos_desired, float motor_time) {
  Serial.print("@moveMotor motor_time = ");
  Serial.println(motor_time);

  AccelStepper *stepper;
  stepper = &main_motor;

  int pos_current;
  pos_current = lens_current;
  
  int steps_to_move = (pos_desired - pos_current) * MS_STEP;

  // Serial.print("Current Position: ");
  // Serial.println(pos_current);
  // Serial.print("pos_desired: ");
  // Serial.println(pos_desired);
  // Serial.print("Steps to move: ");
  // Serial.println(steps_to_move);

  //camera movement base on motor time
  if (motor_time != 0) {
    Serial.print("steps_to_move");
    Serial.println(steps_to_move);
    delay(100);
    //stepper->setAcceleration(calcAccel(abs(steps_to_move), (float)motor_time));

    //try
    stepper->setMaxSpeed(steps_to_move / motor_time);
    stepper->setAcceleration(steps_to_move / motor_time);
  }
  else{
    //motor time pass in as 0 use default config
    //eg) for callibration, go back when done...
    //stepper->setMaxSpeed(RPM);
    stepper->setAcceleration(CALI_ACCEL);
    stepper->setMaxSpeed(RPM);
  }

  // motor movement depends on orientation 
  // if +ve, move clockwise
  // else -ve, move anti-clockwise

  if(orientation == 0){
    stepper->moveTo(pos_desired * MS_STEP);
  }
  else{
      stepper->moveTo(-pos_desired * MS_STEP);
  }

  //blocking statement
  //delay(shutter_spd);
  while (stepper->distanceToGo() != 0) {
    stepper->run();
  }

  //zoom / focus current postion after moving to desired position
  // if (type) { // ZOOM
  //   zoom_current = pos_desired;
  // } else { // FOCUS
  //   focus_current = pos_desired;
  // }
}

void setAccel(float accel) {
  AccelStepper *stepper;
  stepper = &main_motor;
  stepper->setAcceleration(accel);
}

void setCurrentPos(float value) {
  AccelStepper *stepper;
  stepper = &main_motor;
  if(orientation == 0 ){
    stepper->setCurrentPosition(value);
  }
  else{
    stepper->setCurrentPosition(-value);
  }
}


