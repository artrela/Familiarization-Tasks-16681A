// https://www.youtube.com/watch?v=HRaZLCBFVDE

namespace Motor {
  static int pos_update = 0;
  static int pos_last = 0;

  const float pos_kp = 50;
  const float pos_kd = 49.5;
  const float pos_ki = 0.00;

}

namespace Pins {
  const int encA = 3;
  const int encB = 2;
  const int driverPin1 = 9;
  const int driverPin2 = 10;
  const int pwm = 11;
}

namespace axis {
  const int lower = 0;
  const int upper = 360;
}

// are we targeting pos/vel
int target_mode = 0; // 0 for position, 1 for velocity
// what is the value we are targeting
int des = 160;
// init prev time
long t_last = micros();
// init last error
long e_last = 0;

void setup() {

  Serial.begin(9600);
  // set pins
  pinMode(Pins::encA, INPUT);
  pinMode(Pins::encB, INPUT);
  pinMode(Pins::driverPin1, OUTPUT);
  pinMode(Pins::driverPin1, OUTPUT);

  // create an interrupt, used for reading the encoder as the hall sensor
  // triggers outputs
  attachInterrupt(digitalPinToInterrupt(Pins::encA), readEncoder, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:


  // TO CALCULATE error
  // current time
  long t = micros();
  // change in time 
  float deltaT = (float)(t - t_last)/(1e-6);
  // reset t
  t_last = t;

  int pos = 0;
  noInterrupts(); // disable interrupts temporarily while reading
  pos = Motor::pos_update;
  interrupts(); // turn interrupts back on

  // ERROR
  int e = des - pos;
  // DERIVATIVE
  float de_dt = (e - e_last)/(deltaT);
  // INTEGRAL
  float e_i = e_i + e*deltaT;

  // CONTROL LOOP
  float u = (Motor::pos_kp*e) + (Motor::pos_kd * de_dt) + (Motor::pos_ki * e_i);

  e_last = e; 

  float pwr = fabs(u);
  if (pwr > 255)
  {
    pwr = 255;
  }

  int dir = 1;
  if (u < 0){
    dir = -1;
  }

  setMotor(dir, pwr);

  Serial.print(axis::lower);
  Serial.print(" ");
  Serial.print(axis::upper);
  Serial.print(" ");
  Serial.println(pos);

}

// void test()
// {

//   setMotor(1, 100);
//   delay(1000);
//   Serial.println(Motor::pos);

//   setMotor(-1, 100);
//   delay(1000);
//   Serial.println(Motor::pos);

//   setMotor(0, 100);
//   delay(1000);
//   Serial.println(Motor::pos);

// }

void readEncoder() {

  int b = digitalRead(Pins::encB);
  // if b came after a, then the motor is going forward
  if (b>0) {Motor::pos_update++;}
  // if after, then it is going backwards
  else {Motor::pos_update--;}

}

void setMotor(int dir, int pwmVal) {

  analogWrite(Pins::pwm, pwmVal);

  if(dir == 1)
  {
    digitalWrite(Pins::driverPin1, HIGH);
    digitalWrite(Pins::driverPin2, LOW);
  }
  else if(dir == -1)
  {
    digitalWrite(Pins::driverPin1, LOW);
    digitalWrite(Pins::driverPin2, HIGH);
  }
  else
  {
    digitalWrite(Pins::driverPin1, LOW);
    digitalWrite(Pins::driverPin2, LOW);
  }
  
}
