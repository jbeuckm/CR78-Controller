#include <MIDI.h>

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

byte cy_kill_gate = 0;
byte tb_kill_gate = 0;
byte accent_gate = 0;

MIDI_CREATE_DEFAULT_INSTANCE();



void handleNoteOn(byte channel, byte pitch, byte velocity)
{
  byte voice = pitch - 60;

  if (voice >= 0 && voice < REGULAR_PIN_COUNT) { 

    voice_gates[voice] = 3;
  
    if (velocity > 100) {
      accent_gate = 250;
    }
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
        tb_kill_gate = 5;
        break;
  
      case CY_PIN:
      case HH_PIN:
      case M_PIN:
        cy_kill_gate = 5;
        break;
    }
  }
  
  else if (voice == REGULAR_PIN_COUNT || voice == REGULAR_PIN_COUNT + 1) {
    digitalWrite(GU_STOP_PIN, HIGH);
  }
}


int i;

ISR(TIMER1_COMPA_vect) {

  for (i=0; i<14; i++) {
    byte voice_pin = voice_map[i];
    if (voice_gates[i] > 1) {
      digitalWrite(voice_pin, HIGH);
      voice_gates[i] -= 1;
    } else if (voice_gates[i] == 1) {
      digitalWrite(voice_pin, LOW);
      voice_gates[i] = 0;
    }    
  }

    if (cy_kill_gate > 1) {
      digitalWrite(CY_KILL_PIN, HIGH);
      cy_kill_gate -= 1;
    } else if (cy_kill_gate == 1) {
      digitalWrite(CY_KILL_PIN, LOW);
      cy_kill_gate = 0;
    }    
  
    if (tb_kill_gate > 1) {
      digitalWrite(TB_KILL_PIN, HIGH);
      tb_kill_gate -= 1;
    } else if (tb_kill_gate == 1) {
      digitalWrite(TB_KILL_PIN, LOW);
      tb_kill_gate = 0;
    }    
 
    if (accent_gate > 1) {
      digitalWrite(ACCENT_PIN, HIGH);
      accent_gate -= 1;
    } else if (accent_gate == 1) {
      digitalWrite(ACCENT_PIN, LOW);
      accent_gate = 0;
    }    
 
}


// -----------------------------------------------------------------------------

void setup()
{
  int j;
  for (j = 0; j < 14; j++) {
    pinMode(voice_map[j], OUTPUT);
  }

  pinMode(CY_KILL_PIN, OUTPUT);
  pinMode(TB_KILL_PIN, OUTPUT);
  pinMode(ACCENT_PIN, OUTPUT);

  pinMode(GU_STOP_PIN, OUTPUT);
  pinMode(GU_SLOW_PIN, OUTPUT);
  digitalWrite(GU_STOP_PIN, HIGH);

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);

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
    
    
    MIDI.begin(10);
    MIDI.turnThruOff();
}

void loop()
{
  MIDI.read();
}

