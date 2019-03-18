#include <Servo.h> //for servo motors
#include <Kalman.h> //for kalman filter
#include <SPI.h> //for SD card storage
#include <SD.h> //for SD card storage

//SD storage file
File myFile;

//Kalman(process_noise, sensor_noise, estimated_error, initial_value)
Kalman kalmanFilterOne(0, 20, 1023, 0); //intiated for high noise
Kalman kalmanFilterTwo(0, 20, 1023, 0); //intiated for high noise

//variables for sensors and filter
double sm1, sm2, fm1, fm2;
double duration, distance;
double normalize = 0;
int numElements = 5;

//sensor pins
int trig1 = 7;
int echo1 = 6;
int trig2 = 3;
int echo2 = 2;

//servos
Servo leftServo; //pin 9
Servo rightServo; //pin 5

int moveCount;

// original belief of dog's position
double prior[] = {0.200, 0.200, 0.200, 0.200, 0.200};

void setup() {
  Serial.begin(9600);
  
  //setup SD storage file
  SD.begin(4);
  

  // sensor pin setup
  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);

  // servo pin setup
  leftServo.attach(9);
  rightServo.attach(5);

  moveCount = 0;
}

void loop() {
  //open SD file for writing
  myFile = SD.open("sentest.txt", FILE_WRITE);

  if (moveCount < 5) {
   servoForward();
   moveCount++;
  }
  sm1 = (double) read(trig1, echo1); //read data from sensor one
  sm2 = (double) read(trig2, echo2); //read data from sensor two  

  //write sensor data to SD
  myFile.print("Sensor One: "); 
  myFile.println(sm1);
  myFile.print("Sensor Two: ");
  myFile.println(sm2); 

  fm1 = kalmanFilterOne.getFilteredValue(sm1); //filter the measurement
  fm2 = kalmanFilterTwo.getFilteredValue(sm2); //filter the measurement
  myFile.print("Sensor One Filtered: "); 
  Serial.println(fm1);
  myFile.print("Sensor Two Filtered: "); 
  Serial.println(fm2); 
  
  //dog is on either far end (edge) of environment
  if ((fm1 < 24 && fm2 > 96) || (fm2 < 24 && fm1 > 96)) {
    myFile.println("Predicted edge location");

    prior[0] *= 0.800;
    prior[1] *= 0.100;
    prior[2] *= 0.100;
    prior[3] *= 0.100;
    prior[4] *= 0.800;
    
    normalizer(); //Normalize the result so that the probabilities sum 1.0
  }
  else if (((fm1 > 24 && fm1 < 48) && (fm2 > 72 && fm2 < 96))
  || ((fm2 > 24 && fm2 < 48) && (fm1 > 72 && fm1 < 96))) {
    myFile.println("Predicted between edge and middle"); 
    
    prior[0] *= 0.100;
    prior[1] *= 0.800;
    prior[2] *= 0.100;
    prior[3] *= 0.800;
    prior[4] *= 0.100;
      
    normalizer(); //Normalize the result so that the probabilities sum 1.0
  }
  else if ((fm1 > 48 && fm1 < 72) && (fm2 > 48 && fm2 < 72)) {
    myFile.println("Predicted middle"); 
    
    prior[0] *= 0.100;
    prior[1] *= 0.100;
    prior[2] *= 0.800;
    prior[3] *= 0.100;
    prior[4] *= 0.100;
    
    normalizer(); //Normalize the result so that the probabilities sum 1.0
  }
  outProb(); //print probability array
  
  normalize = 0;
  
  myFile.close();
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

void normalizer() {
 for(int i = 0; i < numElements; i++) {
      normalize += prior[i];
    }
    for(int i = 0; i < numElements; i++) {
      prior[i] = prior[i]/normalize;
    }
}

//output position probability array 
void outProb() {
  myFile.println("Probability Array:");
  for(int i = 0; i < numElements; i++) {
      myFile.print(prior[i]);
      myFile.println(" ");
    }
  myFile.println(" ");
}

//turn dog 180 degrees
void servoTurn() 
{
    leftServo.write(60);
    rightServo.write(60);
    delay(2000);
    leftServo.write(90);
    rightServo.write(90);
    //int temp = countForward;
    //countForward = countBackward;
    //countBackward = temp;
}

//move servo fwd x steps
void servoForward() 
{
    leftServo.write(70);
    rightServo.write(100);
    delay(2000);
    leftServo.write(90);
    rightServo.write(90);
    delay(2000);
   // countForward++;
   // countBackward--;
}

//move servo backward x steps
void servoBackward() 
{
    leftServo.write(0);
    rightServo.write(180);
    delay(2000);
    leftServo.write(90);
    rightServo.write(90);
    //countForward++;
   // countBackward--;
}

//choose a random movement for dog
void randomMove() 
{
 // randomNum = random(1, 2);

  //if (randomNum == 1) {
  int movement = random(1, 4);
//  if (movement == 1) 
//  {
//    Serial.println("Dog is turning.");
//    servoTurn();
//  }
//  else if (movement == 2 && countForward <= 3) 
//  {    
//    Serial.println("Dog is going forward.");
//    servoForward();
//    
//  }
//  else if (movement == 3 && countBackward <= 3) 
//  {
//    Serial.println("Dog is going backward.");
//    servoBackward();
//  }
 }
