//library for LCD DIsplay
#include <LiquidCrystal_I2C.h>

//Arduino library for GSM Module
#include <SoftwareSerial.h>

//Thread library to do things simultaneously
#include "protothreads.h"

//music notes definitions
#include "notes.h"





//define Pin numbers
const int microphonePin= 7;
const int buzzerPin=11;
const int movDetectPin= 8 ;
const int ledPin=4;
const int upButtonPin = 2;
const int downButtonPin = 6;
const int selectButtonPin = 5;
const int gsmButtonPin=9;

const int analogMicrophonePin=A0;
// !----global Variables----!

// initialize gsm module
SoftwareSerial SIM900(12, 13); 

//initialize LCD display Pins
LiquidCrystal_I2C lcd(0x27,20,4);

//sensor values
int microphoneAnalogVal; 
int microphoneDigitalVal;
int motionStatus=0;

//Intrusion booleans
bool isAlarmTriggered=false;
bool isDetectionActive=false;

bool isMicrophoneActive=true;
bool isMotionSensorActive=true;

//User Phone Number
const String userPhoneNumber= "+4915201798490";

//menu index
int menu = 1;

//variable for SMS message
String msg="";

int alarmCounter=0;
const int alarmLimit=5;


//Threads: keep most loops separated to be able to do it simultaneously

//Menu Thread is for reading button input and refreshing the menu
pt ptMenu;
int menuThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  for(;;) {
    if (!digitalRead(downButtonPin)){
        menu++;
        updateMenu();
        PT_SLEEP(pt, 100);
        while (!digitalRead(downButtonPin));
      }
      if (!digitalRead(upButtonPin)){
      
        menu--;
        updateMenu();
        PT_SLEEP(pt, 100);
        while(!digitalRead(upButtonPin));
      }
      if (!digitalRead(selectButtonPin)){
        executeAction();
        updateMenu();
        PT_SLEEP(pt, 100);
        while (!digitalRead(selectButtonPin));
      }
    PT_YIELD(pt);
  }
  PT_END(pt);
}

//updateSerial Thread refreshes the Serial input and output, mostyl used for GSM communication
pt ptUpdateSerial;
int updateSerialThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  for(;;) {

   // PT_SLEEP(pt,500);
  
    if(SIM900.available()>0){
      msg= SIM900.readString();
      Serial.print(msg);
      handleSMS(msg);
      PT_SLEEP(pt,10);
    }
   
  
/*
    while (Serial.available()) 
    {
      SIM900.write(Serial.read());//Forward what Serial received to Software Serial Port
    }
    while(SIM900.available()) 
    {
      Serial.write(SIM900.read());//Forward what Software Serial received to Serial Port
      
    }
    */
    
    PT_YIELD(pt);
  }
  PT_END(pt);
}
//Alarm Thread checks if alarm was triggered and plays the sound
pt ptAlarm;
int alarmThread(struct pt* pt) {
  PT_BEGIN(pt);
  // Loop forever
  for(;;) {
    if(isAlarmTriggered){
      //play Sound
      tone(buzzerPin, NOTE_C2, 1000);
      PT_SLEEP(pt,2000);
      alarmCounter++;
      noTone(buzzerPin);
      
      //deactivate Alarm After time
      if(alarmCounter >=alarmLimit){
        isAlarmTriggered=false;
        isDetectionActive=false;
        sendSMS("ALARM! Intrusion detected!");
        alarmCounter=0;
      }
    } 
    PT_YIELD(pt);
  }
  PT_END(pt);
}

//setup function gets called once when starting
void setup() {
  //Setting the baud Rate
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

  isDetectionActive=false;
  isAlarmTriggered=false;
 
  setupGSM();

  digitalWrite(ledPin,HIGH);
  // set up the LCD's number of columns and rows:
  setupDisplay();
  
  updateMenu();

  //Start THreads
  PT_INIT(&ptMenu);
  PT_INIT(&ptUpdateSerial);
  PT_INIT(&ptAlarm);
}

void loop() {


  //loop Threads
  PT_SCHEDULE(menuThread(&ptMenu));
  PT_SCHEDULE(updateSerialThread(&ptUpdateSerial));
  PT_SCHEDULE(alarmThread(&ptAlarm));
  
  //Read Sensor inputs
  if(isDetectionActive){
    //microphone
    if(isMicrophoneActive){
      readAnalogMicrophoneInput();
      // readMicrophoneInputDigital();
    }
    //motion Sensor
    if(isMotionSensorActive){
       detectMotion();
    }
  }
}

//communication between GSM module and Arduino
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

//Setup GSM Module
void setupGSM(){

  //Turn Off GSM Module 
  SIM900.println("AT+CPOWD=1"); //Handshaking with SIM900
  updateSerial();
  delay(2000);

  //Power on GSM Module
  digitalWrite(gsmButtonPin, HIGH);
  delay(1000);
  digitalWrite(gsmButtonPin,LOW);
  delay(5000);


  Serial.println("Initializing..."); 
  delay(1000);

  SIM900.println("AT"); //Handshaking with SIM900
  updateSerial();
  
  SIM900.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  SIM900.println("AT+CNMI=2,2,0,0,0\r");
  //SIM900.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
}


void setupDisplay(){
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Starting Up...");
}

void sendSMS(String text) {
  SIM900.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  SIM900.println("AT+CMGS=\""+userPhoneNumber+"\"" );//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  SIM900.print(text); //text content
  updateSerial();
  SIM900.write(26);
}

void handleSMS(String msg) // Receiving the SMS and extracting the Sender Mobile number & Message Text
{
  msg.trim();
  
  if(msg.startsWith("+CMT")){

    int phoneNumberStartIndex = msg.indexOf("\"");
    int phoneNumberEndIndex = msg.indexOf("\"", phoneNumberStartIndex + 1);
    int messageIndex = msg.lastIndexOf("\"" );
    //check if indexes are not empty  
    if (phoneNumberStartIndex != -1 && phoneNumberEndIndex != -1 && messageIndex != -1) {
      String phoneNumber = msg.substring(phoneNumberStartIndex + 1, phoneNumberEndIndex);
      String message = msg.substring(messageIndex +1);
      

      //format message, remove backspaces and whitespaces
      String result = "";
      for (int i = 0; i < message.length(); i++) {
        char c = message.charAt(i);
        
        if (c != '\b' && c!= '\n' && c!=" ") {
          result += c;
        }
      }
      message=result;

      //Check if Message was sent from user
      if(phoneNumber.equals( userPhoneNumber)){
        checkSMSForCommand(message);
      } 
    }
  }
}

void updateMenu(){
  switch (menu) {
    case 0:
      menu = 1;
      break;
    case 1:
      lcd.clear();
      lcd.print(">Status");
      lcd.setCursor(0, 1);
      lcd.print("Activate/Deactivate");
      lcd.setCursor(0, 2);
      lcd.print("Shedule");
      lcd.setCursor(0, 3);
      lcd.print("Motion Sensor");
      break;
    case 2:
      lcd.clear();
      lcd.print("Status");
      lcd.setCursor(0, 1);
      lcd.print(">Activate/Deactivate");
      lcd.setCursor(0, 2);
      lcd.print("Shedule");
      lcd.setCursor(0, 3);
      lcd.print("Motion Sensor");
      break;
    case 3:
         lcd.clear();
      lcd.print("Status");
      lcd.setCursor(0, 1);
      lcd.print("Activate/Deactivate");
      lcd.setCursor(0, 2);
      lcd.print(">Shedule");
      lcd.setCursor(0, 3);
      lcd.print("Motion Sensor");
      break;
    case 4:
      lcd.clear();
      lcd.print("Status");
      lcd.setCursor(0, 1);
      lcd.print("Activate/Deactivate");
      lcd.setCursor(0, 2);
      lcd.print("Shedule");
      lcd.setCursor(0, 3);
      lcd.print(">Motion Sensor");
      break;
    case 5:
    lcd.clear();
      lcd.print("Activate/Deactivate");
      lcd.setCursor(0, 1);
      lcd.print("Shedule");
      lcd.setCursor(0, 2);
      lcd.print("Motion Sensor");
      lcd.setCursor(0, 3);
      lcd.print(">Microphone Sensor");
      break;
    case 6:
      lcd.clear();
      lcd.print("Shedule");
      lcd.setCursor(0, 1);
      lcd.print("Motion Sensor");
      lcd.setCursor(0, 2);
      lcd.print("Microphone Sensor");
      lcd.setCursor(0, 3);
      lcd.print(">SMS Test");
      break;
    case 7:
      menu = 6;
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
    case 5:
      action5();
      break;
    case 6:
      action6();
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
  isDetectionActive=!isDetectionActive;
  if(isDetectionActive){
    lcd.print("Detection OFF");
  }else{
    lcd.print("Detection is turned on in 30 Seconds");
    delay(30000);
  }
  delay(1000);
  action1();

}
void action3() {
  lcd.clear();
  lcd.print("Select your Shedule");
  delay(1500);
}
void action4() {
  lcd.clear();
  lcd.print("Switching Motion Sensor...");
  delay(1000);
  lcd.clear();
  isMotionSensorActive= !isMotionSensorActive;
  if(isMotionSensorActive){
    lcd.print("Motion Sensor is ON");
  }else{
    lcd.print("Motion Sensor is now OFF");
  }
  delay(2000);
}

void action5(){

  lcd.clear();
  lcd.print("Switching Microphone Sensor...");
  delay(1000);
  lcd.clear();
  isMicrophoneActive= !isMicrophoneActive;
  if(isMicrophoneActive){
    lcd.print("Microphone Sensor is ON");
  }else{
    lcd.print("Microphone Sensor is now OFF");
  }
  delay(2000);
}
void action6(){
  lcd.clear();
  lcd.print("Sending SMS Test...");
  sendSMS("SMS Test");
  delay(1500);
}


void checkSMSForCommand(String command){

  command.trim();

  //commands for Detection
  if(command.equalsIgnoreCase("Activate")){
    isDetectionActive=true;
  }else if(command.equalsIgnoreCase("Deactivate")){
    isDetectionActive=false;
  }
  //commands for motion sensor
  else if(command.equalsIgnoreCase("Motion On")){
    isMotionSensorActive=true;
  } else if(command.equalsIgnoreCase("Motion Off")){
    isMotionSensorActive=true;
  }
  //commands for microphone 
  else if(command.equalsIgnoreCase("Mic On")){
    isMicrophoneActive=true;
  } else if(command.equalsIgnoreCase("Mic Off")){
    isMicrophoneActive=true;
  }
  else if(command.equalsIgnoreCase("Status")){
    if(isDetectionActive){
      sendSMS("Detection is ON");
    }else{
      sendSMS("Detection is OFF");
    }
  }

}

void callSomeone() {
  //call command
  SIM900.println("ATD + \""+userPhoneNumber+"\"" );
  delay(100);
  SIM900.println();
  
  //delay before hanging up
  delay(30000);
  // AT command to hang up
  SIM900.println("ATH"); // hang up
}

void detectMotion(){
  if(isMotionSensorActive){
    //Read sensor value
    motionStatus=digitalRead(movDetectPin);
    //if something is detected, sensor returns HIGH
    if(motionStatus == HIGH)
    {
      isAlarmTriggered=true;
    }
  }
}

void readAnalogMicrophoneInput(){
  if(isMicrophoneActive){
    //Read actual analog Value
    microphoneAnalogVal=analogRead(analogMicrophonePin);

    //Print it, so it can be viewed with Tools→Serial Plotter
    //Serial.println(microphoneAnalogVal+"");
  }
}

//reads the digital microphone input 
void readMicrophoneInputDigital(){

  if(isMicrophoneActive){
    microphoneDigitalVal = digitalRead(microphonePin); 
    //sensor returns false false when something is detected
    if (!microphoneDigitalVal ) {
      isAlarmTriggered=true;
    }
  } 
}