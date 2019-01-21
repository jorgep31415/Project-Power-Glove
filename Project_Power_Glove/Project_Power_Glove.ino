/******************************************** HEADER FILES *********************************************/
/*******************************************************************************************************/
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_9DOF.h>

/**************************************** CONSTANT DEFINITIONS *****************************************/
/*******************************************************************************************************/
#define MOTOR 3
#define FLEX_POINTER A0
#define FLEX_MIDDLE A1
#define FLEX_RING A2
#define FLEX_PINKY A3
#define LED_LEFT 8
#define LED_RIGHT 9

#define MODE_THROTTLE 0
#define MODE_SIGNAL 1
#define MODE_COAST 2

/* "raise" pointer flex sensor threshold */
#define RAISE_THRESHOLD 70
#define LOWER_THRESHOLD 60

/* flex sensor thresholds for signals */
#define THR_POINTER 60
#define THR_MIDDLE 60
#define THR_RING 67
#define THR_PINKY 60
#define BRAKE_THRESHOLD -60

/****************************************** GLOBAL VARIABLES *******************************************/
/*******************************************************************************************************/
int pVal, mVal, rVal, yVal; /* flex sensor values */
int counter1, counter2; /* for loop counters */
bool unRaise1, unRaise2; /* for loop flags */
int roll; /* to hold roll of gyroscope */

int mode = MODE_THROTTLE; /* for mode selection */
int motorVal; /* motor value from 0-255 */
int gradVal;  /* used while starting up, for gradual increase in motor speed from rest */

/* Assign a unique ID to the sensors */
Adafruit_9DOF                 dof   = Adafruit_9DOF();
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);

/* Update this with the correct SLP for accurate altitude measurements */
float seaLevelPressure = SENSORS_PRESSURE_SEALEVELHPA;


/*************************************** FUNCTION DECLARATIONS *****************************************/
/*******************************************************************************************************/
void initSensors(); /* initialize sensors */
void initMotor(); /* initialize motor, to gradually work up from rest, to the first roll value */
void updateMode(int pVal); /* updates mode if there is switch from throttle to signal/coast, or back */
void checkForSignalOrCoastMode(int pVal); /* from throttle mode, checks for a change into signal or coast mode */
void checkForThrottleMode(int pVal); /* from signal or coast mode, checks for a change into throttle mode */
void checkSignal(int pVal, int mVal, int rVal, int yVal); /* checks for left and right turn signals, when in signal mode */
int getRoll(); /* gets the roll of gyroscope from orientation struct */
int findMotorValue(int roll); /* sets motor value with analogWrite */
void checkForBraking(int roll); /* if motor speed is too low, turn brake lights on */


/*********************************************** MAIN **************************************************/
/*******************************************************************************************************/
void setup() {
  Serial.begin(9600);
  pinMode(MOTOR, OUTPUT);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
  initSensors();
  initMotor();  /* gradual increase in speed until roll value */
}

void loop() {

  /* update all our input flex sensor values */
  pVal = analogRead(FLEX_POINTER);
  mVal = analogRead(FLEX_MIDDLE);
  rVal = analogRead(FLEX_RING);
  yVal = analogRead(FLEX_PINKY);

  /* For debugging */
//  Serial.print("Pointer flex: ");
//  Serial.print(pVal);
//  Serial.print("    ");
//  Serial.print("Middle flex: ");
//  Serial.print(mVal);
//  Serial.print("    ");
//  Serial.print("Ring flex: ");
//  Serial.print(rVal);
//  Serial.print("    ");
//  Serial.print("Pinky flex: ");
//  Serial.println(yVal);
  
  /* updates from throttle to signal/coast, or back, if needed */
  updateMode(pVal);

  if (mode == MODE_THROTTLE)
  {
    /* using the gyroscope */
    roll = getRoll();
    /* update motor value with gyroscope roll */
    motorVal = findMotorValue(roll);
//    Serial.println(motorVal);
    analogWrite(MOTOR, motorVal);
  }

  
  /* allows for left and right turn signals */
  if (mode == MODE_SIGNAL)
  {
    Serial.println("Signal mode");
    checkSignal(pVal, mVal, rVal, yVal);
  }

  /* lights braking signal if going too slow */
  checkForBraking(roll);
}


/************************************* FUNCTION IMPLEMENTATIONS ****************************************/
/*******************************************************************************************************/

void initSensors() {
  if(!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println(F("Ooops, no LSM303 detected ... Check your wiring!"));
    while(1);
  }
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 detected ... Check your wiring!");
    while(1);
  }
}

void initMotor() {
  roll = getRoll();
  motorVal = findMotorValue(roll);
  for (gradVal = 0; gradVal < motorVal; gradVal++)  //if motorVal = 127, takes ~3.2 seconds
  {
    analogWrite(MOTOR, gradVal);
    delay(25);
  }
}

/* Duration: 2 seconds max */
void checkForSignalOrCoastMode(int pVal) {
  if (pVal > RAISE_THRESHOLD) //raise #1
  {
    unRaise1 = false;
    for (counter1 = 0; counter1 < 100; counter1++)
    {
      pVal = analogRead(FLEX_POINTER);
      if (!unRaise1 && pVal < LOWER_THRESHOLD) //un-raise 1-2
        unRaise1 = true;
      if (unRaise1 && pVal > RAISE_THRESHOLD) //raise #2
      {
        unRaise2 = false;
        for (counter2 = 0; counter2 < 100; counter2++)
        {
          pVal = analogRead(FLEX_POINTER);
          if (!unRaise2 && pVal < LOWER_THRESHOLD)  //un-raise 2-3
            unRaise2 = true;
          if (unRaise2 && pVal > RAISE_THRESHOLD)  //raise #3
          {
            mode = MODE_COAST;
            break;
          }
          delay(10);
        }
        if (mode == MODE_THROTTLE) //if raise 1 and 2 went through, but not 3
          mode = MODE_SIGNAL;
        break;
      }
      delay(10);
    }
  }
}

/* Duration: 1 second max + 1.5 second possible grace period */
void checkForThrottleMode(int pVal) {
  if (pVal > RAISE_THRESHOLD) //raise #1
  {
    unRaise1 = false;
    for (counter1 = 0; counter1 < 100; counter1++)
    {
      pVal = analogRead(FLEX_POINTER);
      if (!unRaise1 && pVal < LOWER_THRESHOLD) //un-raise 1-2
      {
        unRaise1 = true;
      } 
      if (unRaise1 && pVal > RAISE_THRESHOLD) //raise #2
      {
        mode = MODE_THROTTLE;
        break;
      }
      delay(10);
    }
    if (mode == MODE_THROTTLE)  /* grace period in return to throttle mode */
      delay(1500);
  }
}

void updateMode(int pVal) {
  /* from throttle mode, checks for a change into signal or coast mode */
  if (mode == MODE_THROTTLE)
    checkForSignalOrCoastMode(pVal);
  /* from signal or coast mode, checks for a change into throttle mode */
  else
    checkForThrottleMode(pVal);  
}

void checkSignal(int pVal, int mVal, int rVal, int yVal) {
  while (pVal > THR_POINTER && mVal > THR_MIDDLE && rVal < THR_RING/* && yVal < THR_PINKY*/) /* left signal - 'pointer-middle' */
  {
    digitalWrite(LED_LEFT, HIGH);
    delay(500);
    digitalWrite(LED_LEFT, LOW);
    delay(500);

    pVal = analogRead(FLEX_POINTER);
    mVal = analogRead(FLEX_MIDDLE);
    rVal = analogRead(FLEX_RING);
    yVal = analogRead(FLEX_PINKY);
  }
  while (pVal < THR_POINTER && mVal > THR_MIDDLE && rVal > THR_RING/* && yVal > THR_PINKY*/) /* right signal - "middle-ring*/
  { 
    digitalWrite(LED_RIGHT, HIGH);
    delay(500);
    digitalWrite(LED_RIGHT, LOW);
    delay(500);

    pVal = analogRead(FLEX_POINTER);
    mVal = analogRead(FLEX_MIDDLE);
    rVal = analogRead(FLEX_RING);
    yVal = analogRead(FLEX_PINKY);
  }
}

int getRoll() {
  sensors_event_t accel_event;
  sensors_vec_t   orientation;

  /* calculate pitch and roll from the raw accelerometer data */
  accel.getEvent(&accel_event);
  dof.accelGetOrientation(&accel_event, &orientation);  /* updates the orientation.roll value */

  return orientation.roll;
}

int findMotorValue(int roll) {
  int val = map(roll,-70,70,0,255); /* maps values to 0 - 255 */
  if (roll < -70)  /* if out of range low, set to min (0) */
    val = 0;
  else if (roll > 70) /* if out of range high, set to max (255) */
    val = 255;
  return val;
}

void checkForBraking(int roll) {
  if (roll < BRAKE_THRESHOLD)
  {
    digitalWrite(LED_LEFT, HIGH);
    digitalWrite(LED_RIGHT, HIGH);
  }
  else
  {
    digitalWrite(LED_LEFT, LOW);
    digitalWrite(LED_RIGHT, LOW);
  }
}

