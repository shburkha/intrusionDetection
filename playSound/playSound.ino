//library for LCD DIsplay
#include <LiquidCrystal_I2C.h>

//Arduino library for GSM Module
#include <SoftwareSerial.h>

//Thread library to do things simultaneously
#include "protothreads.h"

//music notes definitions
#include "notes.h"


//define Pin numbers
const int microphonePin = 7;
const int buzzerPin = 11;
const int movDetectPin = 8;
const int ledPin = 4;
const int upButtonPin = 2;
const int downButtonPin = 6;
const int selectButtonPin = 5;
const int gsmButtonPin = 9;

const int analogMicrophonePin = A0;
// !----global Variables----!

// initialize gsm module
SoftwareSerial SIM900(12, 13);

//initialize LCD display Pins
LiquidCrystal_I2C lcd(0x27, 20, 4);

//sensor values
int microphoneAnalogVal;
int microphoneDigitalVal;
int motionStatus = 0;

//Intrusion booleans
bool isAlarmTriggered = false;
bool isDetectionActive = false;

bool isMicrophoneActive = true;
bool isMotionSensorActive = true;

//User Phone Number
const String userPhoneNumber = "+4915201798490";

//menu index
int mainMenuIndex = 1;
bool isSheduleMenuActive = false;
int sheduleStart = 0;
int sheduleEnd = 0;
int sheduleMenuIndex = 1;
bool isSheduleActive = false;

//variable for SMS message
String msg = "";

int alarmCounter = 0;
const int alarmLimit = 5;


//Threads: keep most loops separated to be able to do it simultaneously

//Menu Thread is for reading button input and refreshing the menu
pt ptMenu;
int menuThread(struct pt* pt) {
  PT_BEGIN(pt);

  // Loop forever
  for (;;) {

    if (!digitalRead(downButtonPin)) {
      if (!isSheduleMenuActive) {
        //main menu
        mainMenuIndex++;
        updateMainMenu();
      } else {
        //shedule menu
        handleSheduleMenuInput(-1);
        updateSheduleMenu();
      }

      PT_SLEEP(pt, 100);
      while (!digitalRead(downButtonPin))
        ;
    }
    if (!digitalRead(upButtonPin)) {
      if (!isSheduleMenuActive) {
        //main menu
        mainMenuIndex--;
        updateMainMenu();
      } else {
        //shedule menu
        handleSheduleMenuInput(+1);
        updateSheduleMenu();
      }
      PT_SLEEP(pt, 100);
      while (!digitalRead(upButtonPin))
        ;
    }
    if (!digitalRead(selectButtonPin)) {
      if (!isSheduleMenuActive) {

        executeAction();
        if (!isSheduleMenuActive) {
          updateMainMenu();
        }


      } else {

        sheduleMenuIndex++;
        updateSheduleMenu();
      }


      PT_SLEEP(pt, 100);
      while (!digitalRead(selectButtonPin))
        ;
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
  for (;;) {

    // PT_SLEEP(pt,500);

    while (SIM900.available()) {

      msg = (SIM900.readString());
      Serial.println(msg);
      handleSMS(msg);
      PT_SLEEP(pt, 10);
    }

    PT_YIELD(pt);
  }
  PT_END(pt);
}
//Alarm Thread checks if alarm was triggered and plays the sound
pt ptAlarm;
int alarmThread(struct pt* pt) {
  PT_BEGIN(pt);
  // Loop forever
  for (;;) {
    if (isAlarmTriggered) {
      //play Sound
      tone(buzzerPin, NOTE_C2, 1000);
      //Activate LED
      digitalWrite(ledPin, HIGH);
      PT_SLEEP(pt,1200);
      digitalWrite(ledPin, LOW);
      noTone(buzzerPin);


      PT_SLEEP(pt, 2000);
      alarmCounter++;


      //deactivate Alarm After time
      if (alarmCounter >= alarmLimit) {
        isAlarmTriggered = false;
        isDetectionActive = false;
        sendSMS("ALARM! Intrusion detected!");
        PT_SLEEP(pt, 10000);
        callSomeone();
        alarmCounter = 0;
        isSheduleActive = false;
      }
    }
    PT_YIELD(pt);
  }
  PT_END(pt);
}

//Shedule Thread
pt ptShedule;
int sheduleThread(struct pt* pt) {
  PT_BEGIN(pt);
  // Loop forever

  for (;;) {
    PT_SLEEP(pt, 60000);

    if (isSheduleActive) {
      //Get Time
      String currentTime = getTimeFromGSM();

      String currentHour = currentTime.substring(currentTime.indexOf(",") + 1, currentTime.indexOf(":"));
      Serial.println("Hour " + currentHour);

      int hour = currentHour.toInt();
      //Special case
      if (sheduleEnd < sheduleStart) {
        if (hour >= sheduleStart || hour <= sheduleEnd) {
          if (!isDetectionActive) {
            isDetectionActive = true;
          }
        }
      } else if (hour >= sheduleStart && hour <= sheduleEnd) {
        if (!isDetectionActive) {
          isDetectionActive = true;
        }
      } else {
        if (isDetectionActive) {
          isDetectionActive = false;
        }
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
  pinMode(ledPin, OUTPUT);
  pinMode(upButtonPin, INPUT_PULLUP);
  pinMode(downButtonPin, INPUT_PULLUP);
  pinMode(selectButtonPin, INPUT_PULLUP);

  isDetectionActive = false;
  isAlarmTriggered = false;
  isSheduleMenuActive = false;

  setupGSM();

 
  // set up the LCD's number of columns and rows:
  setupDisplay();

  updateMainMenu();

  //Start THreads
  PT_INIT(&ptMenu);
  PT_INIT(&ptUpdateSerial);
  PT_INIT(&ptAlarm);
  PT_INIT(&ptShedule);
}

void loop() {


  //loop Threads
  PT_SCHEDULE(menuThread(&ptMenu));
  PT_SCHEDULE(updateSerialThread(&ptUpdateSerial));
  PT_SCHEDULE(alarmThread(&ptAlarm));
  PT_SCHEDULE(sheduleThread(&ptShedule));

  //Read Sensor inputs
  if (isDetectionActive) {
    //microphone
    if (isMicrophoneActive) {
      // readAnalogMicrophoneInput();
      readMicrophoneInputDigital();
    }
    //motion Sensor
    if (isMotionSensorActive) {
      detectMotion();
    }
  }
}

//communication between GSM module and Arduino
void updateSerial() {
  delay(500);
  while (Serial.available()) {
    SIM900.write(Serial.read());  //Forward what Serial received to Software Serial Port
  }
  while (SIM900.available()) {
    Serial.write(SIM900.read());  //Forward what Software Serial received to Serial Port
  }
}

//Setup GSM Module
void setupGSM() {

  //Turn Off GSM Module
  SIM900.println("AT+CPOWD=1");
  updateSerial();
  delay(2000);

  //Power on GSM Module
  digitalWrite(gsmButtonPin, HIGH);
  delay(1000);
  digitalWrite(gsmButtonPin, LOW);
  delay(5000);


  Serial.println("Initializing...");
  delay(1000);

  SIM900.println("AT");  //Handshaking with SIM900
  updateSerial();

  SIM900.println("AT+CMGF=1");  // Configuring TEXT mode
  updateSerial();
  SIM900.println("AT+CNMI=2,2,0,0,0\r");

  //SIM900.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();
}


void setupDisplay() {
  lcd.init();
  lcd.backlight();
  lcd.setCursor(1, 0);
  lcd.print("Starting Up...");
  delay(1000);
}

void sendSMS(String text) {
  SIM900.println("AT+CMGF=1");  // Configuring TEXT mode
  updateSerial();
  SIM900.println("AT+CMGS=\"" + userPhoneNumber + "\"");
  updateSerial();
  SIM900.print(text);  //text content
  updateSerial();
  //26 closes the command
  SIM900.write(26);
}

void handleSMS(String msg)  // Receiving the SMS and extracting the Sender Mobile number & Message Text
{
  msg.trim();
  if (msg.startsWith("+CMT")) {
    int phoneNumberStartIndex = msg.indexOf("\"");
    int phoneNumberEndIndex = msg.indexOf("\"", phoneNumberStartIndex + 1);
    int messageIndex = msg.lastIndexOf("\"");
    //check if indexes are not empty
    if (phoneNumberStartIndex != -1 && phoneNumberEndIndex != -1 && messageIndex != -1) {
      String phoneNumber = msg.substring(phoneNumberStartIndex + 1, phoneNumberEndIndex);
      String message = msg.substring(messageIndex + 1);

      //format message, remove backspaces and whitespaces
      String result = "";
      for (int i = 0; i < message.length(); i++) {
        char c = message.charAt(i);

        if (c != '\b' && c != '\n' && c != " ") {
          result += c;
        }
      }
      message = result;
      //Check if Message was sent from user
      if (phoneNumber.equals(userPhoneNumber)) {
        checkSMSForCommand(message);
      }
    }
  }
}
void handleSheduleMenuInput(int menuValue) {

  switch (sheduleMenuIndex) {
    case 0:
      sheduleMenuIndex = 1;
      break;
    case 1:
      if (menuValue < 0) {
        if (sheduleStart - 1 < 0) {
          sheduleStart = 24;
        } else {
          sheduleStart--;
        }

      } else if (menuValue > 0) {

        if (sheduleStart + 1 > 24) {
          sheduleStart = 0;
        } else {
          sheduleStart++;
        }
      }
      break;
    case 2:
      if (menuValue < 0) {
        if (sheduleEnd - 1 < 0) {
          sheduleEnd = 24;
        } else {
          sheduleEnd--;
        }

      } else if (menuValue > 0) {

        if (sheduleEnd + 1 > 24) {
          sheduleEnd = 0;
        } else {
          sheduleEnd++;
        }
      }
      break;
  }
  updateSheduleMenu();
}

void updateSheduleMenu() {
  switch (sheduleMenuIndex) {
    case 0:
      sheduleMenuIndex = 1;
      break;
    case 1:
      lcd.clear();
      lcd.print("Set Shedule");
      lcd.setCursor(0, 1);
      lcd.print("Select Start Time");
      lcd.setCursor(0, 2);
      lcd.print("Time " + String(sheduleStart));
      break;
    case 2:
      lcd.clear();
      lcd.print("Set Shedule");
      lcd.setCursor(0, 1);
      lcd.print("Select End Time");
      lcd.setCursor(0, 2);
      lcd.print("Time " + String(sheduleEnd));
      break;
    case 3:
      lcd.clear();
      lcd.print("Shedule is set from");
      lcd.setCursor(0, 1);
      lcd.print(String(sheduleStart));
      lcd.setCursor(0, 2);
      lcd.print("to");
      lcd.setCursor(0, 3);
      lcd.print(String(sheduleEnd));
      delay(2000);
      isSheduleMenuActive = false;
      sheduleMenuIndex = 1;
      updateMainMenu();
      break;
  }
}

void updateMainMenu() {
  switch (mainMenuIndex) {
    case 0:
      mainMenuIndex = 1;
      break;
    case 1:
      lcd.clear();
      lcd.print(">Status");
      lcd.setCursor(0, 1);
      lcd.print("Activate/Deactivate");
      lcd.setCursor(0, 2);
      lcd.print("Set Shedule");
      lcd.setCursor(0, 3);
      lcd.print("Shedule ON/OFF");
      break;
    case 2:
      lcd.clear();
      lcd.print("Status");
      lcd.setCursor(0, 1);
      lcd.print(">Activate/Deactivate");
      lcd.setCursor(0, 2);
      lcd.print("Set Shedule");
      lcd.setCursor(0, 3);
      lcd.print("Shedule ON/OFF");
      break;
    case 3:
      lcd.clear();
      lcd.print("Status");
      lcd.setCursor(0, 1);
      lcd.print("Activate/Deactivate");
      lcd.setCursor(0, 2);
      lcd.print(">Set Shedule");
      lcd.setCursor(0, 3);
      lcd.print("Shedule ON/OFF");
      break;
    case 4:
      lcd.clear();
      lcd.print("Status");
      lcd.setCursor(0, 1);
      lcd.print("Activate/Deactivate");
      lcd.setCursor(0, 2);
      lcd.print("Set Shedule");
      lcd.setCursor(0, 3);
      lcd.print(">Shedule ON/OFF");
      break;
    case 5:
      lcd.clear();
      lcd.print("Activate/Deactivate");
      lcd.setCursor(0, 1);
      lcd.print("Set Shedule");
      lcd.setCursor(0, 2);
      lcd.print("Shedule ON/OFF");
      lcd.setCursor(0, 3);
      lcd.print(">Motion Sensor");
      break;
    case 6:
      lcd.clear();
      lcd.print("Set Shedule");
      lcd.setCursor(0, 1);
      lcd.print("Shedule ON/OFF");
      lcd.setCursor(0, 2);
      lcd.print("Motion Sensor");
      lcd.setCursor(0, 3);
      lcd.print(">Microphone Sensor");
      break;
    case 7:
      lcd.clear();
      lcd.print("Shedule ON/OFF");
      lcd.setCursor(0, 1);
      lcd.print("Motion Sensor");
      lcd.setCursor(0, 2);
      lcd.print("Microphone Sensor");
      lcd.setCursor(0, 3);
      lcd.print(">SMS Test");
      break;
    case 8:
      mainMenuIndex = 7;
      break;
  }
}
void executeAction() {
  switch (mainMenuIndex) {
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
    case 7:
      action7();
  }
}

void action1() {
  //Status Alarm
  lcd.clear();
  lcd.print("Detection is");
  lcd.setCursor(0, 1);

  if (isDetectionActive) {
    lcd.print("Activated");
  } else {
    lcd.print("Deactivated");
  }
  delay(1500);
}
void action2() {
  lcd.clear();
  isDetectionActive = !isDetectionActive;
  if (!isDetectionActive) {
    lcd.print("Detection OFF");
  } else {
    lcd.print("Detection is turned on in 30 Seconds");
    delay(30000);
  }
  delay(1000);
  action1();
}
void action3() {
  lcd.clear();
  lcd.print("Select your Shedule");
  isSheduleMenuActive = true;
  updateSheduleMenu();
  delay(1500);
}
void action4() {

  isSheduleActive = !isSheduleActive;

  lcd.clear();
  lcd.print("Shedule is");
  lcd.setCursor(0, 1);
  if (isSheduleActive) {
    lcd.print("ON");
  } else {
    lcd.print("OFF");
  }
  delay(1500);
}


void action5() {
  lcd.clear();
  lcd.print("Switching Motion Sensor...");
  delay(1000);
  lcd.clear();
  isMotionSensorActive = !isMotionSensorActive;
  if (isMotionSensorActive) {
    lcd.print("Motion Sensor is ON");
  } else {
    lcd.print("Motion Sensor is now OFF");
  }
  delay(2000);
}
void action6() {
  lcd.clear();
  lcd.print("Switching Microphone Sensor...");
  delay(1000);
  lcd.clear();
  isMicrophoneActive = !isMicrophoneActive;
  if (isMicrophoneActive) {
    lcd.print("Microphone Sensor is ON");
  } else {
    lcd.print("Microphone Sensor is now OFF");
  }
  delay(2000);
}
void action7() {
  lcd.clear();
  lcd.print("Sending SMS Test...");
  sendSMS("SMS Test");
  delay(1500);
}

String getTimeFromGSM() {
  String timeString = "";

  SIM900.println("AT+CMGF=1");  // Configuring TEXT mode
  updateSerial();
  SIM900.println("AT+CCLK?");
  delay(500);  // Wait for the response

  while (SIM900.available()) {
    char c = SIM900.read();
    timeString += c;
    delay(2);  // Add a small delay to allow more characters to arrive if available
  }
  // Parse the time from the received response
  int startIndex = timeString.indexOf("\"") + 1;
  int endIndex = timeString.lastIndexOf("\"");
  timeString = timeString.substring(startIndex, endIndex);

  return timeString;
}


void checkSMSForCommand(String command) {

  command.trim();

  //commands for Detection
  if (command.equalsIgnoreCase("Activate")) {
    isDetectionActive = true;
  } else if (command.equalsIgnoreCase("Deactivate")) {
    isDetectionActive = false;
  }
  //commands for motion sensor
  else if (command.equalsIgnoreCase("Motion On")) {
    isMotionSensorActive = true;
  } else if (command.equalsIgnoreCase("Motion Off")) {
    isMotionSensorActive = true;
  }
  //commands for microphone
  else if (command.equalsIgnoreCase("Mic On")) {
    isMicrophoneActive = true;
  } else if (command.equalsIgnoreCase("Mic Off")) {
    isMicrophoneActive = true;
  } else if (command.equalsIgnoreCase("Status")) {
    if (isDetectionActive) {
      sendSMS("Detection is ON");
    } else {
      sendSMS("Detection is OFF");
    }
  }
}

void callSomeone() {
  //call command
  SIM900.println("ATD + \"" + userPhoneNumber + "\"");
  delay(100);
  SIM900.println();

  //delay before hanging up
  delay(30000);
  // AT command to hang up
  SIM900.println("ATH");  // hang up
}

void detectMotion() {
  if (isMotionSensorActive) {
    //Read sensor value
    motionStatus = digitalRead(movDetectPin);
    //if something is detected, sensor returns HIGH
    if (motionStatus == HIGH) {
      isAlarmTriggered = true;
    }
  }
}

void readAnalogMicrophoneInput() {
  if (isMicrophoneActive) {
    //Read actual analog Value
    microphoneAnalogVal = analogRead(analogMicrophonePin);

    //Print it, so it can be viewed with Tools→Serial Plotter
    //Serial.println(microphoneAnalogVal+"");
  }
}

//reads the digital microphone input
void readMicrophoneInputDigital() {
  if (isMicrophoneActive) {
    microphoneDigitalVal = digitalRead(microphonePin);
    //sensor returns false false when something is detected
    if (!microphoneDigitalVal) {
      isAlarmTriggered = true;
    }
  }
}