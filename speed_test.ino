#include <time.h>

//pin definitions
#define APIN 5
#define BPIN 6
#define IRPIN 10

//global variables
#define PERCPWM_RATIO 2.55  //Serial input PWM percentage to analogwrite
#define RADPWM_RATIO (255/68.7)  //rad/s to analogwrite
#define TC 500  //500 ms
#define KP 30    //gain
volatile unsigned long t0;
volatile unsigned long t1;
volatile unsigned long t_delta[4];
volatile float desTheta = 0;
volatile float eTheta = 0;
volatile float avg_t_delta;
volatile int i_delta = 0;
volatile float curTheta = 0;
volatile int input = 0;


//data structure for speed and direction of motor
struct Velocity {
  int spd, dir;
};

void setup() {
  // put your setup code here, to run once:
  pinMode(APIN, OUTPUT);
  //pinMode(BPIN, OUTPUT);
  pinMode(IRPIN, INPUT);
  attachInterrupt(IRPIN, infraredISR, RISING);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Velocity vel;
  int tick = millis();
  if (Serial.available() > 0){  //check if data is received from serial
    String curString = Serial.readString(); //data is a string
    char curChar[curString.length()+1];
    strcpy(curChar, curString.c_str());     //convert string to char array 
    int curNum = atoi(curChar);             //returns exact int send by user
    vel = motor_convertSpeed(curNum);
    desTheta = vel.spd/RADPWM_RATIO;
    Serial.print("desTheta = ");
    Serial.print(desTheta);
    Serial.print(" s\n");
    Serial.print("curTheta = ");
    Serial.print(curTheta);
    Serial.print(" rad/s\n");
    Serial.print("eTheta = ");
    Serial.print(eTheta);
    Serial.print(" rad/s\n");
    //Serial.print("input = ");
    //Serial.print(input);
    //Serial.print("\n");
  }

  avg_t_delta = average();
  //Serial.print("average time delta = ");
  //Serial.print(avg_t_delta);
  //Serial.print("\n");
  if (NULL != avg_t_delta){
    curTheta = ((1/((float)3.14159))/(avg_t_delta/1000000));
  }
  eTheta = desTheta - curTheta;
  int input = KP*eTheta*RADPWM_RATIO;
  if(255 < input){
    input = 255;
  }
  else if(0 > input){
    input = 0;
  }
  analogWrite(APIN, input);
  int tock = millis();
  //delay(TC - (tock - tick));
}

//for averaging the t_delta array
unsigned long average(){
  unsigned long sum = 0;
  for (int i = 0 ; i <= 3 ; i++){
    if(NULL != t_delta[i]){
      sum += t_delta[i];
    }
  }
  sum = sum/4;
  //Serial.print("average time = ");
  //Serial.print(sum);
  //Serial.print("\n");
  return sum;
}


// converting the serial input to a velocity for analog read
Velocity motor_convertSpeed(int curNum){
  int dir;
  if ( 0 <= curNum){
    if ( 100 < curNum) {
      curNum = 100;
    }
    dir = 0;
  }
  else{
    if ( -100 > curNum) {
      curNum = -100;
    }
    dir = 1;
    curNum = -curNum;
  }
  float spd = (float)curNum*PERCPWM_RATIO;
  Velocity vel;
  vel.spd = (int)spd;
  vel.dir = dir;
  return vel;  
}


//Interrupt for the infrared sensor.
void infraredISR(){
  if(NULL != t0){
    t1 = micros();
    t_delta[i_delta] = t1 - t0;
    t0 = t1;
  }
  else{
    t0 = micros();
  }
  i_delta = i_delta + 1;
  if(3 < i_delta){
    i_delta = 0;
  }
}
