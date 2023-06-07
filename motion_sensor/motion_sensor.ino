int piezo=11; //Das Wort „piezo“ steht jetzt für den Wert 5.
int bewegung=8; //Das Wort „bewegung“ steht jetzt für den Wert 7.
int bewegungsstatus=0; //Das Wort „bewegungsstatus“ steht jetzt zunächst für den Wert 0. Später wird unter dieser Variable gespeichert, ob eine Bewegung erkannt wird oder nicht.

void setup() //Hier beginnt das Setup.
{
pinMode(piezo, OUTPUT); //Der Pin mit dem Piezo (Pin 5) ist jetzt ein Ausgang.
pinMode(bewegung, INPUT); //Der Pin mit dem Bewegungsmelder (Pin 7) ist jetzt ein Eingang.
}

void loop()
{ 
  bewegungsstatus=digitalRead(bewegung); //ier wird der Pin7 ausgelesen. Das Ergebnis wird unter der Variablen „bewegungsstatus“ mit dem Wert „HIGH“ für 5Volt oder „LOW“ für 0Volt gespeichert.
  
  if (bewegungsstatus == HIGH) // high = motion detected
  {
    digitalWrite(piezo, HIGH); //dann soll der Piezo piepsen.
    delay(1000); //...und zwar für für 5 Sekunden.
    digitalWrite(piezo, LOW); //...danach soll er leise sein.
  }
  else
  {
    digitalWrite(piezo, LOW); //...soll der Piezo-Lautsprecher aus sein.
  }
} //Mit dieser letzten Klammer wird der Loop-Teil geschlossen.