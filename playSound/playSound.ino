#include "notes.h";

//library for LCD DIsplay
#include <LiquidCrystal_I2C.h>;
#include <SoftwareSerial.h>;

// Configure software serial port
SoftwareSerial SIM900(12, 13); 
char incoming_char=0;


//define Pin numbers
const int microphonePin= 7;
const int buzzerPin=11;
const int movDetectPin= 8 ;

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
 SIM900.begin(19200);
  Serial.begin(19200);


  // Give time to your GSM shield log on to network
  delay(20000);  

  //setup pinmodes
  pinMode(microphonePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(movDetectPin, INPUT);

  pinMode(upButtonPin,INPUT_PULLUP);
  pinMode(downButtonPin,INPUT_PULLUP);
  pinMode(selectButtonPin,INPUT_PULLUP);


 // set up the LCD's number of columns and rows:
 lcd.init();
 lcd.backlight();

sendSMS();
 
  updateMenu();
  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r"); 
  delay(100);
  // Set module to send SMS data to serial out upon receipt 
  SIM900.print("AT+CNMI=2,2,0,0,0\r");
  delay(100);
  
  
 


}

void loop() {
  // put your main code here, to run repeatedly:

 // Display any text that the GSM shield sends out on the serial monitor
  if(SIM900.available() >0) {
    //Get the character from the cellular serial port
    incoming_char=SIM900.read(); 
    //Print the incoming character to the terminal
    Serial.print(incoming_char); 
  }

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
void sendSMS() {
  // AT command to set SIM900 to SMS mode
  SIM900.print("AT+CMGF=1\r"); 
  delay(100);

  // REPLACE THE X's WITH THE RECIPIENT'S MOBILE NUMBER
  // USE INTERNATIONAL FORMAT CODE FOR MOBILE NUMBERS
  SIM900.println("AT+CMGS=\"+4915201798490\""); 
  delay(100);
  
  // REPLACE WITH YOUR OWN SMS MESSAGE CONTENT
  SIM900.println("Message example from Arduino Uno."); 
  delay(100);

  // End AT command with a ^Z, ASCII code 26
  SIM900.println((char)26); 
  delay(100);
  SIM900.println();
  // Give module time to send SMS
  delay(5000); 
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
      lcd.print(">MenuItem1");
      lcd.setCursor(0, 1);
      lcd.print(" MenuItem2");
      break;
    case 2:
      lcd.clear();
      lcd.print(" MenuItem1");
      lcd.setCursor(0, 1);
      lcd.print(">MenuItem2");
      break;
    case 3:
      lcd.clear();
      lcd.print(">MenuItem3");
      lcd.setCursor(0, 1);
      lcd.print(" MenuItem4");
      break;
    case 4:
      lcd.clear();
      lcd.print(" MenuItem3");
      lcd.setCursor(0, 1);
      lcd.print(">MenuItem4");
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
  lcd.clear();
  lcd.print(">Executing #1");
  sendSMS();
  delay(1500);
}
void action2() {
  lcd.clear();
  lcd.print(">Executing #2");
  delay(1500);
}
void action3() {
  lcd.clear();
  lcd.print(">Executing #3");
  delay(1500);
}
void action4() {
  lcd.clear();
  lcd.print(">Executing #4");
  delay(1500);
}










void readAnalogMicrophoneInput(){
  //Read actual analog Value
  microphoneAnalogVal=analogRead(analogMicrophonePin);

  //Print it, so it can be viewed with Tools→Serial Plotter
  Serial.println(microphoneAnalogVal+"");

 

}

void PlayAlarm(){


}

void detectMotion(){

  //Read sensor value
  motionStatus=digitalRead(movDetectPin);

  //if something is detected, sensor returns HIGH
  if(motionStatus == HIGH)
  {
    playHighSound();
  }
}

//reads the digital microphone input 
void readMicrophoneInputDigital(){

  microphoneDigitalVal = digitalRead(microphonePin); 

  //sensor returns false false when something is detected
  if (!microphoneDigitalVal ) {

    //playHighSound();
    
    
  }
 
}

//plays a low C note for 1 Second
void playLowSound(){
  tone(buzzerPin, NOTE_C2, 1000);
  delay(1200);
  noTone(buzzerPin);

}

//plays a high C note for 1 Second
void playHighSound(){

  tone(buzzerPin, NOTE_C5, 1000);
  delay(1200);
  noTone(buzzerPin);

  
}

void playHarryPotter(){
  // change this to make the song slower or faster
  int tempo = 144;

  // change this to whichever pin you want to use
  int buzzer = 11;

  // notes of the moledy followed by the duration.
  // a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
  // !!negative numbers are used to represent dotted notes,
  // so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
    int melody[] = {


    
      REST, 2, NOTE_D4, 4,
      NOTE_G4, -4, NOTE_AS4, 8, NOTE_A4, 4,
      NOTE_G4, 2, NOTE_D5, 4,
      NOTE_C5, -2, 
      NOTE_A4, -2,
      NOTE_G4, -4, NOTE_AS4, 8, NOTE_A4, 4,
      NOTE_F4, 2, NOTE_GS4, 4,
      NOTE_D4, -1, 
      NOTE_D4, 4,

      NOTE_G4, -4, NOTE_AS4, 8, NOTE_A4, 4, //10
      NOTE_G4, 2, NOTE_D5, 4,
     NOTE_F5, 2, NOTE_E5, 4,
      NOTE_DS5, 2, NOTE_B4, 4,
      NOTE_DS5, -4, NOTE_D5, 8, NOTE_CS5, 4,
      NOTE_CS4, 2, NOTE_B4, 4,
      NOTE_G4, -1,
      NOTE_AS4, 4,
        
      NOTE_D5, 2, NOTE_AS4, 4,//18
      NOTE_D5, 2, NOTE_AS4, 4,
      NOTE_DS5, 2, NOTE_D5, 4,
      NOTE_CS5, 2, NOTE_A4, 4,
      NOTE_AS4, -4, NOTE_D5, 8, NOTE_CS5, 4,
      NOTE_CS4, 2, NOTE_D4, 4,
      NOTE_D5, -1, 
      REST,4, NOTE_AS4,4,  

      NOTE_D5, 2, NOTE_AS4, 4,//26
      NOTE_D5, 2, NOTE_AS4, 4,
      NOTE_F5, 2, NOTE_E5, 4,
      NOTE_DS5, 2, NOTE_B4, 4,
      NOTE_DS5, -4, NOTE_D5, 8, NOTE_CS5, 4,
      NOTE_CS4, 2, NOTE_AS4, 4,
      NOTE_G4, -1, 
    };


    int notes = sizeof(melody) / sizeof(melody[0]) / 2;

    // this calculates the duration of a whole note in ms (60s/tempo)*4 beats
    int wholenote = (60000 * 4) / tempo;

    int divider = 0, noteDuration = 0;


    // iterate over the notes of the melody. 
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration*0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);
    
    // stop the waveform generation before the next note.
    noTone(buzzer);
    
  }
}