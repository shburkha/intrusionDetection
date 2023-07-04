//library for LCD DIsplay
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include "notes.h"

// Configure software serial port
SoftwareSerial SIM900(12, 13); 

//define Pin numbers
const int microphonePin= 7;
const int buzzerPin=11;
const int movDetectPin= 8 ;
const int ledPin=4;

//TODO: pin numbers
const int upButtonPin = 2;
const int downButtonPin = 1;
const int selectButtonPin = 0;

const int analogMicrophonePin=A0;


// !----global Variables----!

//Setup LCD display Pins
LiquidCrystal_I2C lcd(0x27,20,4);

// delay used for playing music
const int pauseMusicDelay = 200;

//counter for test purposes
int count=0;

//sensor values
int microphoneAnalogVal; 
int microphoneDigitalVal;
int motionStatus=0;

//Intrusion 
bool isAlarmTriggered=false;
bool isDetectionActive=false;

int menu = 1;

void setup() {
  Serial.begin(9600);
  SIM900.begin(9600);


  //setup pinmodes
  pinMode(microphonePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(movDetectPin, INPUT);
  pinMode(ledPin,OUTPUT);

  pinMode(upButtonPin,INPUT_PULLUP);
  pinMode(downButtonPin,INPUT_PULLUP);
  pinMode(selectButtonPin,INPUT_PULLUP);
 
 setupGSM();
 digitalWrite(ledPin,HIGH);


 // set up the LCD's number of columns and rows:
  setupDisplay();



  updateMenu();

}

void loop() {
  // put your main code here, to run repeatedly:

  updateSerial();

  menuLoop();

  //Read Inputs
  if(isDetectionActive){
    readAnalogMicrophoneInput();
   // readMicrophoneInputDigital();
    detectMotion();
  }

  if(isAlarmTriggered){
    PlayAlarm();
  }

}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    SIM900.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(SIM900.available()) 
  {
    Serial.write(SIM900.read());//Forward what Software Serial received to Serial Port
  }
}


void setupGSM(){
  Serial.println("Initializing..."); 
  delay(1000);

  SIM900.println("AT"); //Handshaking with SIM900
  updateSerial();
  
  SIM900.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  SIM900.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
}


void setupDisplay(){
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Starting Up...");
}

void sendSMS(String text) {

  SIM900.println("AT+CMGS=\"+4915201798490\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  SIM900.print(text); //text content
  updateSerial();
  SIM900.write(26);
}
void menuLoop(){

  if (!digitalRead(downButtonPin)){
    menu++;
    updateMenu();
    delay(100);
    while (!digitalRead(downButtonPin));
  }
  if (!digitalRead(upButtonPin)){
   
    menu--;
    updateMenu();
    delay(100);
    while(!digitalRead(upButtonPin));
  }
  if (!digitalRead(selectButtonPin)){
    executeAction();
    updateMenu();
    delay(100);
    while (!digitalRead(selectButtonPin));
  }
}
void updateMenu(){
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.print(">Status Detection");
      lcd.setCursor(0, 1);
      lcd.print("Test Alarm");
      lcd.setCursor(0, 2);
      lcd.print("SMS Test");
      lcd.setCursor(0, 3);
      lcd.print("Activate/Deactivate");
      break;
    case 2:
      lcd.clear();
      lcd.print("Status Detection");
      lcd.setCursor(0, 1);
      lcd.print(">Test Alarm");
      lcd.setCursor(0, 2);
      lcd.print("SMS Test");
      lcd.setCursor(0, 3);
      lcd.print("Activate/Deactivate");
      break;
    case 3:
      lcd.clear();
      lcd.print("Status Detection");
      lcd.setCursor(0, 1);
      lcd.print("Test Alarm");
      lcd.setCursor(0, 2);
      lcd.print(">SMS Test");
      lcd.setCursor(0, 3);
      lcd.print("Activate/Deactivate");
      break;
    case 4:
      lcd.clear();
      lcd.print("Status Detection");
      lcd.setCursor(0, 1);
      lcd.print("Test Alarm");
      lcd.setCursor(0, 2);
      lcd.print("SMS Test");
      lcd.setCursor(0, 3);
      lcd.print(">Activate/Deactivate");
      break;
    case 5:
      menu = 4;
      break;
  }
  }
void executeAction() {
  switch (menu) {
    case 1:
      action1();
      break;
    case 2:
      action2();
      break;
    case 3:
      action3();
      break;
    case 4:
      action4();
      break;
  }
}

void action1() {
   //Status Alarm
  lcd.clear();
  lcd.print("Detection is");
  lcd.setCursor(0, 1);

  if(isDetectionActive){
    lcd.print("Activated");
  }else{
    lcd.print("Deactivated");
  }

  delay(1500);
}
void action2() {
  lcd.clear();
  lcd.print(">Executing Test Alarm");
  isAlarmTriggered=!isAlarmTriggered;
  delay(1500);
}
void action3() {
  lcd.clear();
  lcd.print("Sending SMS Test...");
  sendSMS("Hallo?, das ist ein Text");
  delay(1500);
}
void action4() {
   lcd.clear();

   if(isDetectionActive){
    lcd.print("Deactivating Detection...");
    isDetectionActive=false;
    
  }else{
    lcd.print("Activating Detection...");
   isDetectionActive=true;
  }
  delay(1000);
  action1();
}










void readAnalogMicrophoneInput(){
  //Read actual analog Value
  microphoneAnalogVal=analogRead(analogMicrophonePin);

  //Print it, so it can be viewed with Tools→Serial Plotter
  Serial.println(microphoneAnalogVal+"");

 

}
void callSomeone() {
  // REPLACE THE X's WITH THE NUMER YOU WANT TO DIAL
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  SIM900.println("ATD + +4915201798490;");
  delay(100);
  SIM900.println();
  
 // In this example, the call only last 30 seconds
 // You can edit the phone call duration in the delay time
  delay(30000);
  // AT command to hang up
  SIM900.println("ATH"); // hang up
}

void PlayAlarm(){

  PlayLowSound();

}

void detectMotion(){

  //Read sensor value
  motionStatus=digitalRead(movDetectPin);

  //if something is detected, sensor returns HIGH
  if(motionStatus == HIGH)
  {
    isAlarmTriggered=true;
  }
}

//reads the digital microphone input 
void readMicrophoneInputDigital(){

  microphoneDigitalVal = digitalRead(microphonePin); 

  //sensor returns false false when something is detected
  if (!microphoneDigitalVal ) {

    isAlarmTriggered=true;
    
    
  }
 
}

//plays a low C note for 1 Second
void PlayLowSound(){
  tone(buzzerPin, NOTE_C2, 1000);
  delay(1200);
  noTone(buzzerPin);

}

//plays a high C note for 1 Second
void PlayHighSound(){

  tone(buzzerPin, NOTE_C5, 1000);
  delay(1200);
  noTone(buzzerPin);

  
}

