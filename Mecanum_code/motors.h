typedef struct {
  int en, in1, in2;
} Motor;

void InitMotors();
void SetMotor(Motor motor, int spd);

void StopAll();

void Strafe(int rx_axis, int ry_axis);
void RotateCar(int val);
//void ShiftCar(int dpad);
void MoveCar(int cur_spd, int btns, int x_axis);
