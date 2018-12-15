/*
Projeto que utiliza um relógio de tempo real para acionar uma bomba para irrigação

Utiliza a bioblioteca:
DS3231.h de Eric Ayars
http://hacks.ayars.org/2011/04/ds3231-real-time-clock.html
*/

#include <DS3231.h>
#include <Wire.h>
#include <TM1637Display.h>

#define ALRM1_MATCH_EVERY_SEC  0b1111  // once a second
#define ALRM1_MATCH_SEC        0b1110  // when seconds match
#define ALRM1_MATCH_MIN_SEC    0b1100  // when minutes and seconds match
#define ALRM1_MATCH_HR_MIN_SEC 0b1000  // when hours, minutes, and seconds match

#define ALRM2_ONCE_PER_MIN     0b111   // once per minute (00 seconds of every minute)
#define ALRM2_MATCH_MIN        0b110   // when minutes match
#define ALRM2_MATCH_HR_MIN     0b100   // when hours and minutes match

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3
#define Rele 13

const uint8_t SEG_DONE[] = {
  0x0,           // d
  0x0,   // O
  0x0,                           // n
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G            // E
};

int sensor = A0;

int humidade ;

DS3231 Clock;
TM1637Display display(CLK, DIO);

bool h12;
bool PM;
byte ADay, AHour, AMinute, ASecond, ABits;
bool ADy, A12h, Apm;

//Utilizado para atribuir a hora
byte Hora;
byte Minuto;
byte Segundo;
byte tela=0;



void AtribuiHorario(byte& Hour, byte& Minute, byte& Second) {
  // A hora do alarme deve ser
  // no formato HHMMSS
  boolean GotString = false;
  char InChar;
  byte Temp1, Temp2;
  char InString[20];

  byte j=0;
  while (!GotString) {
    if (Serial.available()) {
      InChar = Serial.read();
      InString[j] = InChar;
      j += 1;
      if ((InChar == '\n')) {
        GotString = true;
      }
    }
  }
  if (InString[0] == 'd') {
      Clock.turnOffAlarm(1);
  } else {
      // Carrega hora
      Temp1 = (byte)InString[0] -48; //48 é ASCII de 0
      Temp2 = (byte)InString[1] -48;
      Hour = Temp1*10 + Temp2;
      // now Minute
      Temp1 = (byte)InString[2] -48;
      Temp2 = (byte)InString[3] -48;
      Minute = Temp1*10 + Temp2;
      // now Second
      Temp1 = (byte)InString[4] -48;
      Temp2 = (byte)InString[5] -48;
      Second = Temp1*10 + Temp2;
      Clock.setA1Time(0x0, Hora, Minuto, Segundo, ALRM1_MATCH_HR_MIN_SEC, true, false, false);
      Clock.turnOnAlarm(1);
  }
}


void setup() {
	// Start the I2C interface
	Wire.begin();
	// Start the serial interface
	Serial.begin(9600);
  display.setBrightness(0x0);
  pinMode(Rele, OUTPUT);
  digitalWrite(Rele, HIGH);
}

void loop() {

  tela = (tela+1)%10;

  if (tela<8) {
    if (Clock.getSecond()%2==1) display.showNumberDecEx(Clock.getHour(h12, PM)*100+Clock.getMinute(), (0x40), true);
    else display.showNumberDec(Clock.getHour(h12, PM)*100+Clock.getMinute(), true);
  } else if (tela<9){
    display.showNumberHexEx(0xc,0,false);
    display.showNumberDec(Clock.getTemperature(), false,2,0);
  } else {
    humidade = analogRead(sensor);
    humidade = map(humidade,850,250,0,100);
    display.setSegments(SEG_DONE);
    display.showNumberDec(humidade, false,2,0);
    Serial.print("Mistura : ");
    Serial.print(humidade);
    Serial.println("%");
  }

	// Indicate whether an alarm went off
	if (Clock.checkIfAlarm(1)) {
    Irrigar();
	}

	delay(1000);
}



void serialEvent() {
  while (Serial.available()) {
    AtribuiHorario(Hora, Minuto, Segundo);
    Serial.print(Hora);
    Serial.print(' ');
    Serial.print(Minuto);
    Serial.print(' ');
    Serial.println(Segundo);
  }
}


void Irrigar() {
    if (humidade<80) {
        Serial.println("Irrigar as plantinhas...");
        display.showNumberHexEx(0xa1,0,false);
        digitalWrite(Rele, LOW);
        delay(5000);
        digitalWrite(Rele, HIGH);
    }
}
