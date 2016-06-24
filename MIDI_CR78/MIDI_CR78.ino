#include <MIDI.h>
#include "SoftwareSerialIn/SoftwareSerialIn.h"

#define BD_PIN 1
#define SD_PIN 11
#define LB_PIN 2
#define HB_PIN A3
#define LC_PIN A4
#define RS_PIN A1
#define C_PIN A2
#define CB_PIN 3
#define TB_SHORT_PIN 5
#define TB_LONG_PIN 4
#define TB_KILL_PIN 6
#define MB_PIN A5

#define GU_STOP_PIN 12
#define GU_SLOW_PIN 13

#define CY_PIN 10
#define HH_PIN 9
#define M_PIN 8
#define CY_KILL_PIN 7

#define ACCENT_PIN A0

#define REGULAR_PIN_COUNT 14

byte voice_map[] = {BD_PIN, SD_PIN, LB_PIN, HB_PIN, 
                  LC_PIN, RS_PIN, C_PIN, CB_PIN, 
                  TB_SHORT_PIN, TB_LONG_PIN, 
                  MB_PIN, M_PIN, HH_PIN, CY_PIN};

byte voice_gates[] = {0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0, 0, 0,
                    0, 0};

SoftwareSerialIn serialIn(0);
MIDI_CREATE_INSTANCE(SoftwareSerialIn, serialIn, midiIn);



void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  byte voice = pitch - 60;

  if (voice >= 0 && voice < REGULAR_PIN_COUNT) { 
  
    if (velocity > 100) {
      digitalWrite(ACCENT_PIN, HIGH);
    }

    byte voice_pin = voice_map[voice];
  
    switch (voice_pin) {
      
      case TB_SHORT_PIN:
      case TB_LONG_PIN:
        digitalWrite(TB_KILL_PIN, HIGH);
        break;
  
      case CY_PIN:
      case HH_PIN:
      case M_PIN:
        digitalWrite(CY_KILL_PIN, LOW);
        break;
    }

    voice_gates[voice] = 8;
  }

  else if (voice == REGULAR_PIN_COUNT) {
    digitalWrite(GU_SLOW_PIN, LOW);
    digitalWrite(GU_STOP_PIN, LOW);
  }
  else if (voice == REGULAR_PIN_COUNT + 1) {
    digitalWrite(GU_SLOW_PIN, HIGH);
    digitalWrite(GU_STOP_PIN, LOW);
  }
}

void handleNoteOff(byte channel, byte pitch, byte velocity)
{
  byte voice = pitch - 60;

  if (voice >= 0 && voice < REGULAR_PIN_COUNT) { 
    
    byte voice_pin = voice_map[voice];
  
    switch (voice_pin) {
      
      case TB_SHORT_PIN:
      case TB_LONG_PIN:
        digitalWrite(TB_KILL_PIN, LOW);
        break;
  
      case CY_PIN:
      case HH_PIN:
      case M_PIN:
        digitalWrite(CY_KILL_PIN, HIGH);
        break;
    }
  }
  
  else if (voice == REGULAR_PIN_COUNT || voice == REGULAR_PIN_COUNT + 1) {
    digitalWrite(GU_STOP_PIN, HIGH);
  }
}


int i;

ISR(TIMER1_COMPA_vect) {

  for (i=0; i<REGULAR_PIN_COUNT; i++) {
    byte voice_pin = voice_map[i];
    if (voice_gates[i] > 1) {
      digitalWrite(voice_pin, HIGH);
      voice_gates[i] -= 1;
    } else if (voice_gates[i] == 1) {
      digitalWrite(voice_pin, LOW);
      voice_gates[i] = 0;
    }    
  }   
 
}


// -----------------------------------------------------------------------------

void setup()
{
  int j;
  for (j = 0; j < REGULAR_PIN_COUNT; j++) {
    pinMode(voice_map[j], OUTPUT);
    digitalWrite(voice_map[j], LOW);
  }

  pinMode(CY_KILL_PIN, OUTPUT);
  digitalWrite(CY_KILL_PIN, HIGH);
  
  pinMode(TB_KILL_PIN, OUTPUT);
  digitalWrite(TB_KILL_PIN, HIGH);
  
  pinMode(ACCENT_PIN, OUTPUT);
  digitalWrite(ACCENT_PIN, LOW);

  pinMode(GU_STOP_PIN, OUTPUT);
  pinMode(GU_SLOW_PIN, OUTPUT);
  digitalWrite(GU_STOP_PIN, HIGH);

  midiIn.setHandleNoteOn(handleNoteOn);
  midiIn.setHandleNoteOff(handleNoteOff);

  cli();//stop interrupts
  
  //set timer1 interrupt at 1kHz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0;
  // set timer count for 1khz increments
  OCR1A = 1999;// = (16*10^6) / (1000*8) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS11 bit for 8 prescaler
  TCCR1B |= (1 << CS11);   
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  
  sei();//allow interrupts
    
    
  midiIn.begin(10);
  midiIn.turnThruOff();
}

void loop()
{
  midiIn.read();
}

