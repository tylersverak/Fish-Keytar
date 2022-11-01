#include "MIDIUSB.h"
#include "pitchToNote.h"
#define NUM_BUTTONS 24
#define CHANNEL 0          // VERY IMPORTANT, USE TO CHANGE THE CHANNEL, 0 = midi channel 1
#define DEBUG false
#include <Wire.h>
#include "Adafruit_MPR121.h"

#ifndef _BV
#define _BV(bit) (1 << (bit)) 
#endif
// You can have up to 4 on one i2c bus but one is enough for testing!
Adafruit_MPR121 cap1 = Adafruit_MPR121();
Adafruit_MPR121 cap2 = Adafruit_MPR121();
// Keeps track of the last pins touched
// so we know when buttons are 'released'
uint32_t lasttouched = 0;
uint32_t currtouched = 0;
int laststate1 = 1;
int laststate2 = 1;

const int intensityPot = 64;  //A0 input

uint8_t notePitches[NUM_BUTTONS][4] = {   {pitchC2,pitchC3,pitchC4,pitchC5}, {pitchD2b,pitchD3b,pitchD4b,pitchD5b}, {pitchD2,pitchD3,pitchD4,pitchD5},
{pitchE2b,pitchE3b,pitchE4b,pitchE5b}, {pitchE2,pitchE3,pitchE4,pitchE5}, {pitchF2,pitchF3,pitchF4,pitchF5},{pitchG3b,pitchG3b,pitchG3b,pitchG3b},
{pitchG2,pitchG3,pitchG4,pitchG5}, {pitchA2b,pitchA3b,pitchA4b,pitchA5b}, {pitchA2,pitchA3,pitchA4,pitchA5}, {pitchB2b,pitchB3b,pitchB4b,pitchB5b}, 
{pitchB2,pitchB3,pitchB4,pitchB5}, {pitchC3,pitchC4,pitchC5,pitchC6}, {pitchD3b,pitchD4b,pitchD5b,pitchD6b}, {pitchD3,pitchD4,pitchD5,pitchD6},
{pitchE3b,pitchE4b,pitchE5b,pitchE6b}, {pitchE3,pitchE4,pitchE5,pitchE6}, {pitchF3,pitchF4,pitchF5,pitchF6}, {pitchG3b,pitchG4b,pitchG5b,pitchG6b},
{pitchG3,pitchG4,pitchG5,pitchG6}, {pitchA3b,pitchA4b,pitchA5b,pitchA6b}, {pitchA3,pitchA4,pitchA5,pitchA6}, {pitchB3b,pitchB4b,pitchB5b,pitchB6b}, {pitchB3,pitchB4,pitchB5,pitchB6}};
//keys : 0, 11, 1, 10, 23, 22, 9, 21, 8, 20, 7, 19, 18, 6, 17, 
// 5, 16, 15, 4, 14, 3, 13, 2, 12
uint8_t intensity;

void swap(uint8_t x, uint8_t y){
  
  for (int i = 0; i < 4; i++){
    uint8_t temp = notePitches[x][i];
    notePitches[x][i] = notePitches[y][i];
    notePitches[y][i] = temp;
  }
  
}

void setup() {
  if (DEBUG){
    Serial.begin(9600);
    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
      delay(10);
    }
  }
  
  delay(1000);
  // sets up sensors
  if (!cap1.begin(0x5A)) {
    if (DEBUG){
      Serial.println("MPR121 #1 not found, check wiring?");
    }
    while (1);
  }
  if (!cap2.begin(0x5C)) {
    if (DEBUG){
      Serial.println("MPR121 #2 not found, check wiring?");
    }
    while (1);
  }
  if (DEBUG){
    Serial.println("MPR121s found!");
  }
  swap(1,2);
  swap(2, 22);
  swap(3, 20);
  swap(4, 18);
  swap(5, 15);
  swap(6, 13);
  swap(7, 10);
  // 8 is fine
  swap(9,13);
  swap(10, 20);
  swap(11, 22);
  swap(12, 23);
  swap(13, 21);
  swap(14, 19);
  swap(15, 17);
  // 16 is fine
  swap(17, 19);
  swap(18, 23);
  swap(19, 22);
  swap(20, 21);
  // 21, 22, 23 are fine

  // button setup
  pinMode(9, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
}

void loop() {
  // read what is pressed
  currtouched = cap1.touched();
  uint32_t temp = cap2.touched();
  currtouched = currtouched | (temp << 12);
  readIntensity();
  playNotes();
  lasttouched = currtouched;
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {

  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};

  MidiUSB.sendMIDI(event);
}

void readIntensity()
{

  int val = analogRead(intensityPot);

  intensity = 64; //(uint8_t) (map(val, 0, 1023, 0, 127)); //comment this out if using pot
}

void playNotes(){
  int state1 = digitalRead(9);
  int state2 = digitalRead(10);
  int lastindex = 2 + laststate1 - 2 * laststate2;
  int index = 2 + state1 - 2 * state2;
  if(lastindex != index){
    for (uint16_t i = 0; i < NUM_BUTTONS; i++){
      noteOff(CHANNEL, notePitches[i][lastindex], 0);
      MidiUSB.flush();
   }
  }
  
  for (uint16_t i = 0; i < NUM_BUTTONS; i++){
    // it if *is* touched and *wasnt* touched before, alert!
    if ((currtouched & bit(i)) && !(lasttouched & bit(i)) ) {
      if (DEBUG){
        Serial.print(i); Serial.println(" touched");
      }
      noteOn(CHANNEL, notePitches[i][index], intensity);
      MidiUSB.flush();
    }
    // if it *was* touched and now *isnt*, alert!
    if (!(currtouched & bit(i)) && (lasttouched & bit(i)) ) {
      if (DEBUG){
        Serial.print(i); Serial.println(" released");
      }
      noteOff(CHANNEL, notePitches[i][index], 0);
      MidiUSB.flush();
    }
  }
  laststate1 = state1;
  laststate2 = state2;
}

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {

  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};

  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {

  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};

  MidiUSB.sendMIDI(noteOff);
}

/*
 * 
 * c3 ->c4
D3b -> b4
d3 -> d4b
e3b -> b4b
e3-> d3b
f3->d4b
g3b->a4
g3->d3b
a3b->a4b
a3=>d3b
b3b->g4
b3->d3b
c4->d3b
d4b->g4b
d4->d3b
e4b->f4
e4->d3b
f4->d3b
g4b->e4
g4->d3
a4b->eb
a4->d3b
b4b->d4
b4->c3

 */
