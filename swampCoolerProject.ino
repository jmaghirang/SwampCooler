//Alexis Elquist. Julianne Maghirang, Samuel Suk
//Group #32
//CPE 301 Semester Project
//Swamp Cooler

//includes
//the lcd1602 display
#include <LiquidCrystal.h>
//the fan motor
#include <Servo.h>
//the dth11, temp and humid sensor
#include "DHT.h"
//the ds1307 real time clock
#include <RTClib.h>
//stepper motor
#include <stepper.h>

//defines
#define LED_YELLOW_PIN 9
#define LED_GREEN_PIN 8
#define LED_BLUE_PIN 7
#define LED_RED_PIN 6
#define DHT_PIN A1
#define BUTTON_PIN 18
#define DHT_PIN A1
#define WATER_SENSOR_INPUT A0
#define WATER_LEVEL_LIMIT 100
#define TEMP_HIGH_LIMIT 25
#define SYSTEM_STATE A2
#define INCREMENT_SERVO_ANGLE A5
#define DECREMENT_SERVO_ANGLE A6
#define SERVO_PIN 36
#define FAN_SPEED 255
#define FAN_PIN 13

//prototypes for components
void sensorDisplay();
void monitorWaterLevel();
int getState();
void disabledState();
void idleState();
void errorState();
void runningState();
void clearLEDS();
void getButtonPushed();
unsigned int adc_read(unsigned char);
void setServoPos();
void fanSpeed(int);

//global variables
int servoPos;
int system_state;
float h;
float t;
int waterLevel;
int state;
int btnPushed;
DateTime now;

//functions
//monitor the water levels in the resorvoir
void monitorWaterLevel(){
  if (waterLevel < WATER_LEVEL_LIMIT){
    Serial.print("\n");
    Serial.print("ERROR: Water level is too low");
    Serial.print("\t");
    Serial.print("Water level: ");
    Serial.print(waterLevel, DEC);
    Serial.print("\n");
  }
  delay(500);
}

//humid and temp continuously monitored and reported
void sensorDisplay(){
  h = dht.readHumidity();
  t = dht.readTemperature();
  dht.read(DHT_PIN);
  lcd.setCursor(0, 0);
  lcd.print("Temperature: ");
  lcd.print(t);
  lcd.setCursor(0, 1);
  lcd.print("Humidity: ");
  lcd.print(h);
  Serial.print("\n");
  Serial.print(t);
  delay(500);
}

//monitor the state of the system
int getState(){
  //checks water level
  unsigned int w = waterLevel(WATER_SENSOR_INPUT);
  //checks temperature and humidity
  t = dht.readTemperature();
  dht.read(DHT_PIN);
  //when button is getting pushed
  getButtonPushed();

  //if else statement to check
  // 0 means disabled or by default, 1 means enabled
  //system is disabled
  if (system_state == 0){
    return 3;
  }
  //system is idle
  else if (waterLevel > WATER_LEVEL_LIMIT && t < TEMP_HIGH_LIMIT){
    return 2;
  }
  //system is running
  else if (waterLevel > WATER_LEVEL_LIMIT && t > TEMP_HIGH_LIMIT){
    return 1;
  }
  //error
  else{
    return 0;
  }
}

//states of the system
//ADC library can be used
void disabledState(){
  digitalWrite(LED_YELLOW_PIN, HIGH);
  fanSpeed(0);
  getButtonPushed();
}

void idleState(){
  digitalWrite(LED_GREEN_PIN, HIGH);
  sensorDisplay();
  monitorWaterLevel();
  setServoPos();
  fanSpeed(0);
  getButtonPushed();
}

void error_state(){
  digitalWrite(LED_RED_PIN, HIGH);
  sensorDisplay();
  monitorWaterLevel();
  setServoPos();
  fanSpeed(0);
  getButtonPushed();
}

void runningState(){
  digitalWrite(LED_BLUE_PIN, HIGH);
  sensorDisplay();
  monitorWaterLevel();
  setServoPos();
  fanSpeed(1);
  getButtonPushed();
}

void clearLEDs(){
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_BLUE_PIN, LOW);
}

void fanSpeed(int enable){
  if (enable){
    t = dht.readTemperature();
    if (t > TEMP_HIGH_LIMIT){
      digitalWrite(FAN_PIN, FAN_SPEED);
    }
    else if (t < TEMP_HIGH_LIMIT){
      digitalWrite(FAN_PIN, 0);
    }
  }
  else{
    digitalWrite(FAN_PIN, 0);
  }
}

void getButtonPushed(){
  unsigned int b = btnPushed(SYSTEM_STATE);
  if (btnPushed == HIGH && system_state == 1){
    system_state = 0;
    printTime();
    delay(1000);
  }
  else if (btnPushed == HIGH && system_state == 0){
    system_state = 1;
    printTime();
    delay(1000);
  }
}


void setServoPos(){
  unsigned int i = incBtn(INCREMENT_SERVO_ANGLE);
  unsigned int d = decBtn(DECREMENT_SERVO_ANGLE);
  if (incBtn == HIGH){
    if (servoPos < 180)
      servoPos++;
  }
  if (decBtn == HIGH){
    if (servoPos > 0)
      servoPos--;
  }
  servo.write(servoPos);
}

void printTime(){
  //check real time clock now
  DateTime now = rtc.now();
  //if system is enabled
  if (system_state == 1){
    Serial.print("\n");
    Serial.print("Enabled: (");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  }
  //if system is disabled
  else if (system_state == 0){
    Serial.print("\n");
    Serial.print("Disabled: (");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  //initializes the sensor
  dht.begin();
  //initializes the LCD screen interface
  lcd.begin(16, 2);
  //attaches servo variable to the pin
  servo.attach(MOTOR_PIN);
  servoPos = 0;
  system_state = 0;
  //initializes the real time clock
  rtc.begin();

  //check if real time clock is working
  //if so, display date and time
  if (!rtc.isrunning()){
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  //system resets or clears
  setSystemState(0);
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned int w = waterLevel(WATER_SENSOR_INPUT);
  int tempState;

  state = getState();
  if (system_state == 1){
    clearLEDS();
  }
    do{
      tempState = getState();
      //switch statement for system state
      switch (state){
      case 0:
        error_state();
        break;
      case 1:
        running_state();
        break;
      case 2:
        idle_state();
        break;
      default:
        break;
      }
      delay(2000);
    } 
    while (system_state == 1 && tempState == state){
    clearLEDS();
    }
  }
  
  else if (system_state == 0){
    disabled_state();
    delay(2000);
  }
}
