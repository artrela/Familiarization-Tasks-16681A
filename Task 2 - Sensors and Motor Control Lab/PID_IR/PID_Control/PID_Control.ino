// PID Control for Position: https://www.youtube.com/watch?v=HRaZLCBFVDE
// PID Control for Speed: https://www.youtube.com/watch?v=HRaZLCBFVDE

// are we targeting pos/vel
int target_mode =  1; // 0 for position, 1 for velocity
// what is the value we are targeting
int des = 60;
// init prev time
long t_last = micros();
// init last error
long e_last = 0;
namespace Pos
{
  static int update = 0;
  static int last = 0;

  const float kp = 50;
  const float kd = 49.5;
  const float ki = 0.00;

}

namespace Vel
{
  static float update = 0;
  static float last = 0;
  static float value = 0;
  static float filtered = 0;

  static float prevT = 0;
  static float deltaT = 0;

  // kp needs to be of a certain value or else
  // the motor wont move at all  
  const float kp = 5;
  const float ki = 0.00;

  static float e = 0;
  static float e_i = 0;

}

namespace Pins
{
  const int encA = 3;
  const int encB = 2;
  const int driverPin1 = 9;
  const int driverPin2 = 10;
  const int pwm = 11;
}

namespace axis
{
  const int lower = 0;
  const int upper = 360;
}

void setup()
{

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

void loop()
{
  if (target_mode == 0){runPosControl();}
  else if (target_mode == 1){runVelControl();}
}

void runPosControl()
{
  // TO CALCULATE error
  // current time
  long t = micros();
  // change in time
  float deltaT = (float)(t - t_last) / (1e-6);
  // reset t
  t_last = t;

  int pos = 0;
  noInterrupts(); // disable interrupts temporarily while reading
  pos = Pos::update;
  interrupts(); // turn interrupts back on

  // ERROR
  int e = des - pos;
  // DERIVATIVE
  float de_dt = (e - e_last) / (deltaT);
  // INTEGRAL
  float e_i = e_i + e * deltaT;

  // CONTROL LOOP
  float u = (Pos::kp * e) + (Pos::kd * de_dt) + (Pos::ki * e_i);

  // update the error term
  e_last = e;

  // cap the pwm signal strength
  float pwr = fabs(u);
  if (pwr > 255){pwr = 255;}

  // if our control signal needs a negative control signal
  // reverse the direction of the motor
  int dir = 1;
  if (u < 0){dir = -1;}

  // set the motor
  setMotor(dir, pwr);

  // print the results
  Serial.print(axis::lower);
  Serial.print(" ");
  Serial.print(axis::upper);
  Serial.print(" ");
  Serial.println(pos);
}

void runVelControl()
{ 
  int pos = 0;
  float vel = 0;
  noInterrupts(); // disable interrupts temporarily while reading
  pos = Pos::update;
  vel = Vel::update;
  interrupts(); // turn interrupts back on

  //filter the new found velocity
  Vel::filtered = 0.854*Vel::filtered + 0.0728*vel + 0.0728*Vel::last;
  Vel::last = vel;

  // Calculate error terms
  Vel::e = des - Vel::filtered;
  Vel::e_i = Vel::e_i + (Vel::e * Vel::deltaT);

  // calculate control signal u
  float u = (Vel::kp * Vel::e) + (Vel::ki * Vel::e_i);

  // cap the pwm signal strength
  int pwr = fabs(u);
  if (pwr > 255){pwr = 255;}

  // if our control signal needs a negative control signal
  // reverse the direction of the motor
  int dir = 1;
  if (u < 0){dir = -1;}

  // set the motor
  setMotor(dir, pwr);

  // // print the results
  Serial.print(axis::lower);
  Serial.print(" ");
  Serial.print(axis::upper);
  Serial.print(" ");
  Serial.println(Vel::filtered);
  // Serial.println(Vel::update);  
}


void readEncoder()
{
  int b = digitalRead(Pins::encB);

  int inc = 0;
  // if b came after a, then the motor is going forward
  if (b > 0){inc = 1;}
  // if after, then it is going backwards
  else{inc = -1;}
  Pos::update = Pos::update + inc;

  // only for vel control
  // current time
  long t = micros();
  // change in time
  Vel::deltaT = (float)(t - Vel::prevT) / (1e-6);

  // The velocity value is how much time has elasped since the last reading
  // (approx. manual reading) 180 counts per main shaft rev 
  // counts / sec * (60 sec/ min) * (1 rev / 180 counts)
  Vel::update = (inc / Vel::deltaT) / 180 * 60;
  Vel::prevT = t; 


}

void setMotor(int dir, int pwmVal)
{

  analogWrite(Pins::pwm, pwmVal);

  if (dir == 1)
  {
    digitalWrite(Pins::driverPin1, HIGH);
    digitalWrite(Pins::driverPin2, LOW);
  }
  else if (dir == -1)
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
