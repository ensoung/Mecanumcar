#include <arduino.h>
#include "motors.h"

Motor motor_fl = { 14, 27, 26 };
Motor motor_fr = { 32, 33, 25 };
Motor motor_rl = { 15, 4, 2 };   // usb-c 15pin board //{15, 16, 4};  // micro-usb 19pin
Motor motor_rr = { 19, 5, 18 };  // usb-c 15pin board //{19, 5, 18}; // micro-usb 19pin

#define LEFT_JOYSTICK_THRESHOLD 40
#define STRAFE_SPEED 70

void InitMotors() {
  pinMode(motor_fl.en, OUTPUT); //define all of the pins to the motors
  pinMode(motor_fl.in1, OUTPUT);
  pinMode(motor_fl.in2, OUTPUT);

  pinMode(motor_fr.en, OUTPUT);
  pinMode(motor_fr.in1, OUTPUT);
  pinMode(motor_fr.in2, OUTPUT);

  pinMode(motor_rl.en, OUTPUT);
  pinMode(motor_rl.in1, OUTPUT);
  pinMode(motor_rl.in2, OUTPUT);

  pinMode(motor_rr.en, OUTPUT);
  pinMode(motor_rr.in1, OUTPUT);
  pinMode(motor_rr.in2, OUTPUT);
}

void SetMotor(Motor motor, int spd) {   // sets the motor to a given speed and if limit if the number goes through the max
  if (spd > 255) spd = 255;
  else if (spd < -255) spd = -255;
  else if (spd == 0) {
    digitalWrite(motor.in1, LOW);
    digitalWrite(motor.in2, LOW);
  } else {
    // set first wheel direction
    digitalWrite(motor.in1, spd > 0);
    digitalWrite(motor.in2, !(spd > 0));
  }
  analogWrite(motor.en, abs(spd));
}

void StopAll() {  //stop all of the motors
  SetMotor(motor_fl, 0);
  SetMotor(motor_fr, 0);
  SetMotor(motor_rl, 0);
  SetMotor(motor_rr, 0);
}
int maximum(int x, int y) {
  if (x > y) {
    return x;
  } else {
    return y;
  }
}

void RotateCar(int dpad) {  // rotate the car 
  //if( abs(rx_axis)>30 ) {
  if (dpad & 0x0c) {  // right:0x04, left :0x08
    //int val = map(abs(rx_axis), 30, 512, 0, 128) * (rx_axis>0?1:-1);
    int rotate_spd = 110;

    rotate_spd *= dpad & 0x04 ? 1 : -1;
    SetMotor(motor_fl, rotate_spd);
    SetMotor(motor_fr, -1 * rotate_spd);
    SetMotor(motor_rl, rotate_spd);
    SetMotor(motor_rr, -1 * rotate_spd);
  }
}
void Strafe(int rx_axis, int ry_axis) {
  rx_axis *= -1;
  ry_axis *= -1;
  if (abs(rx_axis) > LEFT_JOYSTICK_THRESHOLD || abs(ry_axis) > LEFT_JOYSTICK_THRESHOLD) {
    Serial.printf("%d %d \n", rx_axis, ry_axis);
    double delta = atan2(ry_axis, rx_axis);

    delta *= 57;
    if (delta < 0)
      delta += 360.0;
    // printf("%f \n", delta);
    double magnitude = sqrt(rx_axis ^ 2 + ry_axis ^ 2);
    printf("%f angle\n", delta);

    // Serial.print("forward");
    double denominator = maximum(abs(ry_axis) + abs(rx_axis), 511);
    printf("%f denom\n", denominator);

    int frontLeftPower =  (((ry_axis + rx_axis) / denominator)*55);
    int backLeftPower = (((ry_axis - rx_axis) / denominator)*55);
    int frontRightPower = (((ry_axis - rx_axis) / denominator)*55);
    int backRightPower = (((ry_axis + rx_axis) / denominator)*55);

    frontLeftPower += frontLeftPower>0? STRAFE_SPEED : -STRAFE_SPEED;
    backLeftPower += backLeftPower>0? STRAFE_SPEED : -STRAFE_SPEED;
    frontRightPower += frontRightPower>0? STRAFE_SPEED : -STRAFE_SPEED;
    backRightPower += backRightPower>0? STRAFE_SPEED : -STRAFE_SPEED;

    // int frontLeftPower = (((ry_axis + rx_axis) / denominator) * 100);
    // int backLeftPower = (((ry_axis - rx_axis) / denominator) * 100);
    // int frontRightPower = (((ry_axis - rx_axis) / denominator) * 100);
    // int backRightPower = (((ry_axis + rx_axis) / denominator) * 100);





    SetMotor(motor_fl, frontLeftPower);
    SetMotor(motor_fr, frontRightPower);
    SetMotor(motor_rl, backLeftPower);
    SetMotor(motor_rr, backRightPower);
    printf("%d front left %d backleft %d frontright %d backright \n", frontLeftPower,
           backLeftPower, frontRightPower, backRightPower);
  }
}

/*
void ShiftCar(int dpad){ 
  int shift_spd=100;
  
  // right:0x04, left :0x08
 
  shift_spd *= dpad&0x08? 1:-1;

  SetMotor(motor_fl, shift_spd);
  SetMotor(motor_fr, (-1)*shift_spd);
  SetMotor(motor_rl, (-1)*shift_spd);
  SetMotor(motor_rr, shift_spd);
}
*/

void MoveCar(int cur_spd, int btns, int x_axis) {
  int spd_fl, spd_fr, spd_rl, spd_rr;
  int steering = 0;
  spd_fl = spd_fr = spd_rl = spd_rr = cur_spd;

  if (x_axis && abs(cur_spd) > 25) {  // steering (-512~512)
    if (abs(x_axis) < 30)
      x_axis = 0;
    else {
      steering = map(abs(x_axis), 30, 512, 0, 75) * (x_axis > 0 ? 1 : -1);
      steering = steering * (cur_spd > 0 ? 1 : -1);  // switch +- for backward

      spd_fl += steering;
      spd_fr -= steering;
      spd_rl += steering;
      spd_rr -= steering;
    }
  }

  int shift_spd = 100;
  if (btns & 0x30) {  // 0x10 : Left Bumper,  0x20 : Right Bumper
    shift_spd *= cur_spd > 0 ? 1 : -1;

    if (btns & 0x10) {     // left shift
      spd_fl += shift_spd; /*base*/
      spd_fr -= shift_spd;
      spd_rl -= shift_spd;
      spd_rr += shift_spd;  // base
    } else {                // left shift
      spd_fl -= shift_spd;
      spd_fr += shift_spd;  // base
      spd_rl += shift_spd;  /*base*/
      spd_rr -= shift_spd;
    }
  }

  Serial.printf("[%d] %d, %d, %d, %d\n", steering, spd_fl, spd_fr, spd_rl, spd_rr);
  SetMotor(motor_fl, spd_fl);
  SetMotor(motor_fr, spd_fr);
  SetMotor(motor_rl, spd_rl);
  SetMotor(motor_rr, spd_rr);
}
