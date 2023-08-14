#include <Bluepad32.h>

GamepadPtr myGamepad = nullptr;

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedGamepad(GamepadPtr gp) {

  if (myGamepad == nullptr) {
    Serial.println("CALLBACK: Gamepad is connected");
    // Additionally, you can get certain gamepad properties like:
    // Model, VID, PID, BTAddr, flags, etc.
    GamepadProperties properties = gp->getProperties();
    Serial.printf("Gamepad model: %s, VID=0x%04x, PID=0x%04x\n",
                  gp->getModelName().c_str(), properties.vendor_id,
                  properties.product_id);
    myGamepad = gp;
  }

}

void onDisconnectedGamepad(GamepadPtr gp) {
    if (myGamepad == gp) {
      extern void StopAll();
      StopAll();
      Serial.println("CALLBACK: Gamepad is disconnected");
      myGamepad = nullptr;
    }
}


void BeginGamePad(){
  BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);
  
  // "forgetBluetoothKeys()" should be called when the user performs
  // a "device factory reset", or similar.
  // Calling "forgetBluetoothKeys" in setup() just as an example.
  // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
  // But might also fix some connection / re-connection issues.
  BP32.forgetBluetoothKeys();
}

void UpdateGamePad(int *l_trig, int *r_trig, int *btns, uint8_t *dpad, int *x_axis, int *rx_axis, int *ry_axis){
  BP32.update();
  if(myGamepad==nullptr)
    return;

  *x_axis = myGamepad->axisX();       // left X Axis (-511 - 512) for turning
  *rx_axis = myGamepad->axisRX();     // right X Axis (-511 - 512) for rotating (must be stopped!)
  *ry_axis = myGamepad->axisRY();
  *l_trig = myGamepad->brake();       // (0 - 1023): Left Trigger
  *r_trig = myGamepad->throttle();    // (0 - 1023): Right Trigger

  *btns = myGamepad->buttons();
  /*
        0x10 : Left Bumper
        0x20 : Right Bumper
        0x01 : A
        0x02 : B
        0x04 : X
        0x08 : Y
  */
        
  *dpad = myGamepad->dpad(); 
  /*
        up   :0x01 
        down :0x02
        right:0x04
        left :0x08
  */
  
  return;
}
