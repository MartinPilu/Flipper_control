// 1.7
// Added center pause

#include <Arduino.h>
#include <EEPROM.h>

#define RECALL_TAG 'B'

// flipper board standalone
int DirPin = 5;
int StepPin = 6;
int EnPin = 12;

// placa bombaj eje Y
// int DirPin = 6;
// int StepPin = 3;
// int EnPin = 8;

int SensorPin = 4;
int pauseBtn = 2;

struct Settings
{
  char tag;
  unsigned long sidePause;
  unsigned long centerPause;
  unsigned long bootpause;
  unsigned long flipTime;
  unsigned long flipPulses;
};

Settings settings = {
    'A',
    20000,
    0000,
    3000,
    2000,
    800};

bool takePhoto = true;

unsigned long startMillis = 0;

void SerialSend()
{
  Serial.print("Side pause: ");
  Serial.print(settings.sidePause);
  Serial.print(" ");
  Serial.print("Center pause: ");
  Serial.print(settings.centerPause);
  Serial.print(" ");
  Serial.print("Boot pause: ");
  Serial.print(settings.bootpause);
  Serial.print(" ");
  Serial.print("Flip time: ");
  Serial.print(settings.flipTime);
  Serial.print(" ");
  Serial.print("Flip pulses: ");
  Serial.println(settings.flipPulses);
}

void Store_settings()
{
  settings.tag = RECALL_TAG;
  EEPROM.put(0, settings);
  SerialSend();
}

void Recall_settings()
{
  Settings recallThis;
  EEPROM.get(0, recallThis);

  if (recallThis.tag == RECALL_TAG)
    settings = recallThis;
}

void SerialReceive()
{
  if (Serial.available())
  {
    char b = Serial.read();

    if (b == 's')
    {
      settings.sidePause = Serial.parseInt();
      SerialSend();
    }

    if (b == 'c')
    {
      settings.centerPause = Serial.parseInt();
      SerialSend();
    }

    if (b == 'b')
    {
      settings.bootpause = Serial.parseInt();
      SerialSend();
    }
    if (b == 'f')
    {
      settings.flipTime = Serial.parseInt();
      SerialSend();
    }
    if (b == 'p')
    {
      settings.flipPulses = Serial.parseInt();
      SerialSend();
    }
    if (b == 'e')
    {
      Serial.print("Store> ");
      Store_settings();
    }
  }
}

void bootPause()
{
  startMillis = millis();
  //Serial.println("Pausa");

  while (millis() < startMillis + settings.bootpause - 2000) // 2000 es el tiempo que tiene para mandar el msj de la foto.
  {
    SerialReceive();
  }
}

void printHelp()
{
  Serial.println("[ Flipper V1.5 ]");
  Serial.println("Commands:");

  Serial.println("sxxxx - Set Side stop time");
  Serial.println("cxxxx - Set Center stop time");
  Serial.println("bxxxx - Set Boot pause");
  Serial.println("fxxxx - Set Flip time");
  Serial.println("pxxxx - Set Flip pulses");
  Serial.println("e - save settings to EEPROM");
}

void pulsing()
{
  int f = (settings.flipTime * 1000) / settings.flipPulses;
  int fr = 0;

  for (int steps = 0; steps < settings.flipPulses; steps++)
  {
    digitalWrite(StepPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(StepPin, LOW);
    delayMicroseconds(f - 10);
  }
}

void pausa(unsigned long pauseTime)
{
  startMillis = millis();

  while (millis() < startMillis + pauseTime)
  {

    SerialReceive();

    // if (!digitalRead(pauseBtn))
    // {
    //   delay(500);
    //   while (!digitalRead(pauseBtn))
    //   {
    //   }
    //   delay(500);
    //   while (digitalRead(pauseBtn))
    //   {
    //     digitalWrite(13, HIGH);
    //   }
    //   delay(500);
    //   while (!digitalRead(pauseBtn))
    //   {
    //   }
    //   delay(500);
    //   digitalWrite(13, LOW);
    // }

    /*
       if (Serial.available() > 0){
       char action = Serial.read();

             if(action == 'o'){
             break;}
       }
    */
    //Serial.println("FIN pausa");
  }
}

void turnCW()
{
  Serial.println("M"); //start moving
  delay(1000);
  digitalWrite(EnPin, LOW);
  digitalWrite(DirPin, HIGH);
  pulsing();
  // digitalWrite(EnPin, HIGH);

  //  if (takePhoto == true) {
  delay(1000);
  if (digitalRead(SensorPin) == LOW)
  {
    Serial.println("A"); //On A side
  }
  else
  {
    Serial.println("B"); //On B side
  }
  //  }
}

void turnCCW()
{
  Serial.println("M"); //start moving
  delay(1000);
  digitalWrite(EnPin, LOW);
  digitalWrite(DirPin, LOW);
  pulsing();
  // digitalWrite(EnPin, HIGH);

  //  if (takePhoto == true) {
  delay(1000);
  if (digitalRead(SensorPin) == LOW)
  {
    Serial.println("A"); //On A side
  }
  else
  {
    Serial.println("B"); //On B side
  }
  //  }/
}

void setup()
{
  pinMode(EnPin, OUTPUT);
  digitalWrite(EnPin, LOW);
  pinMode(StepPin, OUTPUT);
  pinMode(DirPin, OUTPUT);
  pinMode(SensorPin, INPUT_PULLUP);

  Serial.begin(19200);

  printHelp();

  Recall_settings();
  SerialSend();
  bootPause();
  pinMode(13, OUTPUT);
  pinMode(pauseBtn, INPUT_PULLUP);
}

void loop()
{
  turnCW();
  pausa(settings.sidePause);
  SerialReceive();

  turnCCW();
  // digitalWrite(EnPin, HIGH);
  pausa(settings.centerPause);
  SerialReceive();

  turnCCW();
  pausa(settings.sidePause);
  SerialReceive();

  turnCW();
  // digitalWrite(EnPin, HIGH);
  pausa(settings.centerPause);
  SerialReceive();
}
