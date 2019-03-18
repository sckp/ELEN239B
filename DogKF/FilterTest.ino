#include <Kalman.h> //for kalman filter

//Kalman(process_noise, sensor_noise, estimated_error, initial_value)
Kalman kalmanFilterOne(0, 20, 1023, 0); //intiated for high noise
Kalman kalmanFilterTwo(0, 20, 1023, 0); //intiated for high noise

//sensor pins
int trig1 = 7;
int echo1 = 6;
int trig2 = 3;
int echo2 = 2;

double sm1, sm2, fm1, fm2;
double duration, distance;

void setup() {
  Serial.begin(9600);

  // sensor pin setup
  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);
}

void loop() {
  sm1 = (double) read(trig1, echo1); //read data from sensor one
  sm2 = (double) read(trig2, echo2); //read data from sensor two  

  //write sensor data to SD
  //Serial.println(sm1);
  //Serial.println(sm2); 

  fm1 = kalmanFilterOne.getFilteredValue(sm1); //filter the measurement
  fm2 = kalmanFilterTwo.getFilteredValue(sm2); //filter the measurement
  Serial.println(fm1);
  Serial.println(fm2); 
}

double read (int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distance = (duration/2) / 29.1;   //divide by 29.1 to calculate cm of distance
  return distance;
}
