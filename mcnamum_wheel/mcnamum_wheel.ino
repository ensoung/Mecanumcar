#include "gamepad.h"
#include "motors.h"

#define MAX_SPEED 128 // -255~255 까지가 motor pwm 가동 범위임.
#define TRIGGER_THRESHOLD 50  //(0 - 1023)
#define AXIS THRESHOLD 25 // (-511 - 512)
#define INC_SPEED 20

#define BACKWARD -1
#define STOP 0
#define FORWARD 1

int car_status=STOP;
int cur_speed;

unsigned long cur;


void setup() {
  BeginGamePad();
  
  Serial.begin(115200);
  InitMotors();
  cur=millis();
}

int l_trig, r_trig, btns, x_axis, rx_axis, ry_axis;
uint8_t dpad;

void loop() {
  if( (millis()-cur) > 100){  //make the code run every 100 miliseconds
    int new_speed=0;
    cur=millis();
    UpdateGamePad( &l_trig, &r_trig, &btns, &dpad, &x_axis, &rx_axis, &ry_axis);  //update all of the gamepad data

    // speed control by R-L trigger
    if(r_trig>TRIGGER_THRESHOLD)
      new_speed = map(r_trig, TRIGGER_THRESHOLD, 1023, 0, MAX_SPEED); // put a trigger threshold and remap the data
    else r_trig=0;

    if(l_trig>TRIGGER_THRESHOLD)
      new_speed = map(l_trig, TRIGGER_THRESHOLD, 1023, 0, -MAX_SPEED); // put a trigger threshold and remap the data
    else l_trig=0;
    
    if(new_speed==0){ // when neither left or right trigger is pressed
      cur_speed += cur_speed>0? -INC_SPEED : INC_SPEED;  // slowly decrease the speed


      
      if(abs(cur_speed)<=INC_SPEED){  //if the absolute of the current speed is lower then the increment speed then stop 
        car_status=STOP;
        StopAll();
        cur_speed=0;
      }
      /* just for checking speed decrease.
      static int cur_speed_bk;
      if(cur_speed!=cur_speed_bk){
        cur_speed_bk=cur_speed;
        Serial.println(cur_speed);
      }
      */
    }else if(l_trig && r_trig){  // R-L Trigger both pressed mean breaking 0~1023 0~0x3ff
      int breaking;
      if(car_status==FORWARD){
        breaking = l_trig;
      }else if(car_status=BACKWARD){
        breaking = r_trig;
      }else
        return; // already stopped
        
      if(breaking>1000)
        breaking = cur_speed; // stop immediately
      else if(breaking>0x200)
        breaking = INC_SPEED*4;
      else if(breaking>0x80)
        breaking = INC_SPEED*3;
      else
        breaking = INC_SPEED*2;
      
      if(abs(cur_speed)>breaking){
        cur_speed -= breaking;
      } else { // Stoo if current speed is slower than breaking
        car_status=STOP;
        StopAll();
        cur_speed=0;
      }
    }else { // only one trigger is pressed
      if(new_speed){
        if(new_speed>cur_speed){  //add speed if trigger value is bigger than cur_speed or slowing down
          cur_speed +=INC_SPEED;
        } else{
          cur_speed += -INC_SPEED;
        }
        
        if(cur_speed>0) {
          car_status=FORWARD;
          // if there is no new speed then change the right and left trigger and change when the value is pressed
          // there is also a chance that you need to change everything
          if(cur_speed>new_speed && new_speed>0) cur_speed=new_speed; // verifying speed. 
        }else if(cur_speed<0) {
          car_status=BACKWARD;
          if(cur_speed<new_speed && new_speed<0) cur_speed=new_speed; // verifying speed
        }
      }
    }
    
    if(cur_speed){
      MoveCar(cur_speed, btns, x_axis); //if there is anything in the current speed then move the car and include the turn speed
    }else {
      if(dpad & 0x0c){ // right:0x04, left :0x08
        Serial.printf("rotate %x", dpad); //the rotation of the car 
        RotateCar(dpad);
      }else if(rx_axis || ry_axis){ //if anything on the left joystick is moved then strafe in the direction
        Strafe(rx_axis, ry_axis);
        //ShiftCar(dpad);
      }
    }
    // End of Speed control by R-L Trigger
    
    //Serial.println(cur_speed);
  }
  return;
}
