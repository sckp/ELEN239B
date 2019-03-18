#include <Servo.h>
#include <Kalman.h>
#include <SPI.h>
#include <SD.h>

//SD storage file
File myFile;

//Kalman(process_noise, sensor_noise, estimated_error, initial_value)
Kalman kalmanFilter(0.125, 32, 1023, 0); //intiated for high noise

//variabes for Kalman Filter
double sm1, sm2, fm1, fm2;

// sensor pins
int trig1 = 7;
int echo1 = 6;
int trig2 = 3;
int echo2 = 2;

//reset button pin
int resetButton = 8;

//servos
Servo leftServo; //pin 9
Servo rightServo; //pin 5

//starting values
int reset = 0;
int countForward = 0;
int countBackward = 0;
long duration, distance;
int randomNum;
int moveCount = 0;

// original belief of dog's position
float prior[] = {0.2, 0.2, 0.2, 0.2, 0.2};

//normalize is updated in loop()
float normalize = 0;

//number of elements in location array
int numElements = 5;

void setup() 
{  
  Serial.begin(9600);
  
  //setup SD storage file
  SD.begin(4);
  // sensor pin setup
  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);
  
  // button pin setup
  pinMode(resetButton, INPUT);

  // servo pin setup
  //leftServo.attach(9);
  //rightServo.attach(5);
}

void loop() 
{
  myFile = SD.open("sentest.txt", FILE_WRITE);
  //check if want to reset to orignal beliefs
  if (reset == HIGH) 
  {
    pressReset();
  }

  if(moveCount < 5) {
    servoForward();
    moveCount++;
  }
  //randomMove(); //choose random movement for dog

  sm1 = (double) read(trig1, echo1); //read data from sensor one
  sm2 = (double) read(trig2, echo2); //read data from sensor two
  
  fm1 = kalmanFilter.getFilteredValue(sm1); //filter the measurement
  fm2 = kalmanFilter.getFilteredValue(sm2); //filter the measurement

  //write sensor locations to SD
  myFile.print("Sensor One: "); 
  myFile.println(sm1);
  myFile.print("Sensor Two: ");
  myFile.println(sm2); 
  
  // begin assigning probabilities based off sensor reading
  // Dog is on either far end of environment
  if( (sm1 < 20 && sm2 > 80) || (sm2 < 20 && sm1 > 80))
  {
    myFile.println("Predicted edge"); 
    prior[0] *= 0.8;
    prior[1] *= 0.1;
    prior[2] *= 0.1;
    prior[3] *= 0.1;
    prior[4] *= 0.8;
      
   normalizer(); //Normalize the result so that the probabilities sum 1.0
  }
  //Dog is between middle and end of environment
  else if(((sm1 > 20 && sm2 < 40) && (sm2 > 60 && sm1 < 80))
  ||  ((sm2 > 20 && sm1 < 40) && (sm1 > 60 && sm2 < 80)))
  {
    myFile.println("Predicted between edge and middle"); 
    prior[0] *= 0.1;
    prior[1] *= 0.8;
    prior[2] *= 0.1;
    prior[3] *= 0.8;
    prior[4] *= 0.1;
      
    normalizer(); //Normalize the result so that the probabilities sum 1.0
  }
  //Dog is in the middle of the environment
  else 
  //if((sm1 > 40 && sm2 < 60) && (sm2 > 40 && sm1 < 60))
  {
    myFile.println("Predicted middle"); 
    prior[0] *= 0.1;
    prior[1] *= 0.1;
    prior[2] *= 0.8;
    prior[3] *= 0.1;
    prior[4] *= 0.1;
    
    normalizer(); //Normalize the result so that the probabilities sum 1.0
  } 
  outProb(); //serial print probability array
  normalize = 0;
  myFile.close();
}


// receive reading from sensor
float read(int trigPin,int echoPin)
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  //ping sensor returns time
  //divide by 29.1 to calculate cm of distance
  distance = (duration/2) / 29.1;
  return distance;
}

//turn dog 180 degrees
void servoTurn() 
{
    leftServo.write(60);
    rightServo.write(60);
    delay(2000);
    leftServo.write(90);
    rightServo.write(90);
    int temp = countForward;
    countForward = countBackward;
    countBackward = temp;
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
    countForward++;
    countBackward--;
}

//move servo backward x steps
void servoBackward() 
{
    leftServo.write(0);
    rightServo.write(180);
    delay(2000);
    leftServo.write(90);
    rightServo.write(90);
    countForward++;
    countBackward--;
}

//choose a random movement for dog
void randomMove() 
{
  randomNum = random(1, 2);

  if (randomNum == 1) {
  int movement = random(1, 4);
  if (movement == 1) 
  {
    Serial.println("Dog is turning.");
    servoTurn();
  }
  else if (movement == 2 && countForward <= 3) 
  {    
    Serial.println("Dog is going forward.");
    servoForward();
    
  }
  else if (movement == 3 && countBackward <= 3) 
  {
    Serial.println("Dog is going backward.");
    servoBackward();
  }
  }
}

//reset all to original beliefs
void pressReset() 
{
  Serial.println("Resetting original belief.");
  prior[0] = 0.2;
  prior[1] = 0.2;
  prior[2] = 0.2;
  prior[3] = 0.2;
  prior[4] = 0.2; 
}

//normalize the prior array so sum is equal to 1
void normalizer() 
{
 for(int i = 0; i < numElements; i++)
    {
      normalize += prior[i];
    }
    for(int i = 0; i < numElements; i++) {
      prior[i] = prior[i]/normalize;
    }
}

//output position probability array 
void outProb() 
{
  myFile.println("Probability Array:");
  for(int i = 0; i < numElements; i++)
    {
      myFile.print(prior[i]);
      myFile.println(" ");
    }
}
