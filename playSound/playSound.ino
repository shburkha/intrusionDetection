#include "notes.h";

//library for LCD DIsplay
#include <LiquidCrystal.h>



//define Pin numbers
const int microphonePin= 7;
const int buzzerPin=11;
const int movDetectPin= 8 ;

const int upButtonPin = 0;
const int downButtonPin = 0;
const int selectButtonPin = 0;

const int analogMicrophonePin=A0;


// !----global Variables----!

//Setup LCD display Pins
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

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

int menuLayer = 1;

void setup() {

  //setup pinmodes
  pinMode(microphonePin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(movDetectPin, INPUT);

 // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // Print a message to the LCD.
  lcd.print("hello, world!");


  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
 
  //Read Inputs
  if(isDetectionActive){
    readAnalogMicrophoneInput();
    readMicrophoneInputDigital();
    detectMotion();
  }

  if(isAlarmTriggered){
    PlayAlarm();
  }

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