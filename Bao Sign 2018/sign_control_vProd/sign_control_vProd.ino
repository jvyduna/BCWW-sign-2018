/*
  Bao Sign 2022

  Author: Jeff Vyduna
  Contact: web@jeffvyduna.com
  License: MIT

  Program Description: Blink 5 signs: HOT, BAO, CHICKA, [N|W]OW, WOW!

  10 position Ribbon/IDC cable pinout:
    PIN1 (red): GND
    PIN2      : GND
    PIN3      : Sign 1, HOT
    PIN4      : Sign 2, BAO
    PIN5      : Sign 3, CHICKA
    PIN6      : Sign 4, N/WOW
    PIN7      : Sign 5, WOW!
    PIN8      : Sign 4's Servo: Transform W to N
    PIN9      : VCC
    PIN10     : VCC

*/

#define DEBUG                   // Debugging print utilities. Takes a lot of memory. Comment this line for "production"
#include "DebugUtils.h"
#include "SignMessages.h"       // Defines symbols like _BCWW and BAO_CHICKA_WOW_WOW
#include <Servo.h>              // Servo PWM control.

// Hardware config
const byte firstRelayPin = 2;   // Relay for HOT; +1 = BAO, etc
const byte numRelays = 5;
const byte NWOW_IDX = 2;        // bit vector index of which sign in a HBCWWN vector is the swappable [N/W]OW sign
const byte servoPin = 7;
const byte baoNowModePin = 8;   // When low (using internal pullup), override normal mode to Serving BaoNow mode.
const int servoNPos = 40;       // Calibrate to the servo arm position when W is covered to show N
const int servoWPos = 140;      // Calibrate to the servo arm position when W uncovered to show W
const unsigned int servoDelay = 1000; // milliseconds to wait for flap to change position

// Global variables
boolean sign4isN = true;                 // True when we think it reads NOW, not WOW
Servo nwServo;

unsigned int bpm;
unsigned long beatPeriod;                // Tempo (BPM) = 1000/beatPeriod*60
unsigned int b1_2, b1_4, b3_4, b1_8, b1, b2, b4, b8;   // BeatPeriod */ factors
boolean baoNowMode;

void startupDiagnostic();
void setTempo(unsigned int _bpm);
void setAllOff();
void runBaoNowAnims();
void runRegularAnims();
void flickerOn(byte message);
void flickerOff(byte message);
void setMessage(byte message);
void setSign(int signIndex, boolean state);
void sign4toW();
void sign4toN();
void sign4toNSlow();
void on_off(byte message, int period);

// Setup arrays of functions so we can call a random function via random index
#define NUMBER_OF_ANIMATIONS 12           // highest animX() + 1
void (*animations[NUMBER_OF_ANIMATIONS])();
#define NUMBER_OF_BAONOW_ANIMATIONS 8    // highest baoNowAnimX() + 1
void (*baoNowAnimations[NUMBER_OF_BAONOW_ANIMATIONS])();
void anim0();
void anim1();
void anim2();
void anim3();
void anim4();
void anim5();
void anim6();
void anim7();
void anim8();
void anim9();
void anim10();
void anim11();
void baoNowAnim0();
void baoNowAnim1();
void baoNowAnim2();
void baoNowAnim3();
void baoNowAnim4();
void baoNowAnim5();
void baoNowAnim6();
void baoNowAnim7();
void baoNowAnim2helper1();
void baoNowAnim4helper();
void setupFunctions() {
  animations[0] = anim0;  animations[1] = anim1;  animations[2] = anim2;  animations[3] = anim3;
  animations[4] = anim4;  animations[5] = anim5;  animations[6] = anim6;  animations[7] = anim7;
  animations[8] = anim8;  animations[9] = anim9;  animations[10] = anim10; animations[11] = anim11;

baoNowAnimations[0] = baoNowAnim0;  baoNowAnimations[1] = baoNowAnim1;  baoNowAnimations[2] = baoNowAnim2;
baoNowAnimations[3] = baoNowAnim3;  baoNowAnimations[4] = baoNowAnim4;  baoNowAnimations[5] = baoNowAnim5;
baoNowAnimations[6] = baoNowAnim6;  baoNowAnimations[7] = baoNowAnim7;
}


void d(int ms) { delay(ms); }
void d1()   { delay(beatPeriod); }
void d1_2() { delay(b1_2); }
void d1_4() { delay(b1_4); }
void d3_4() { delay(b3_4); }
void d1_8() { delay(b1_8); }
void d2()   { delay(b2); }
void d4()   { delay(b4); }
void d8()   { delay(b8); }


/////////////////////////////////
//    Setup - called once on start-up
/////////////////////////////////

void setup() {
  #ifdef DEBUG
    Serial.begin(57600);  // For debug print statements over serial port
    for (int i = 0; i < 10; i++) {
      if (Serial) break;
      delay(1000);
    }
    Serial.println("BCWW Neon Sign v2.0");
  #endif

  for (int pin = firstRelayPin; pin < firstRelayPin + numRelays; pin++) {
    pinMode(pin, OUTPUT);
  }
  pinMode(baoNowModePin, INPUT_PULLUP);
  nwServo.attach(servoPin, 700, 2300);

  setupFunctions();
  setTempo(50);
  startupDiagnostic();
  randomSeed(analogRead(0));  // Different seed value each time
  DEBUG_PRINTLN("Setup complete\n");
}


/////////////////////////////////
//    Loop
/////////////////////////////////

void loop() {
  setTempo(50);
  if (digitalRead(baoNowModePin) == LOW) {
    DEBUG_PRINTLN("Manual BaoNow mode button detected.");
    runBaoNowAnims();
  } else {
    runRegularAnims();
  }
}

void runRegularAnims() {
  // Play a random animation, don't repeat until all have been used
  DEBUG_PRINTLN("Resetting used animations vector")
  unsigned long animsUsed = pow(2,NUMBER_OF_ANIMATIONS) - 1;
  while (animsUsed > 0) {
    if (digitalRead(baoNowModePin) == LOW) {
      DEBUG_PRINTLN("Stopping to switch to Bao Now mode.");
      return;
    }
    byte currentAnim = random(NUMBER_OF_ANIMATIONS);
    if (bitRead(animsUsed,currentAnim)) {
      #ifdef DEBUG
        Serial.print("Remaining Regular animation vector: ");
        Serial.println(animsUsed, BIN);
        Serial.print("Starting Regular anim: ");
        Serial.println(currentAnim);
      #endif
      animations[currentAnim]();
      bitClear(animsUsed,currentAnim);
    }
  }
}


void runBaoNowAnims() {
  // Play a random animation, don't repeat until all have been used
  DEBUG_PRINTLN("Resetting used BaoNow animations vector")
  unsigned long BNAnimsUsed = pow(2,NUMBER_OF_BAONOW_ANIMATIONS) - 1;
  while (BNAnimsUsed > 0) {
    if (digitalRead(baoNowModePin) != LOW) {
      DEBUG_PRINTLN("Stopping to switch to Regular mode.");
      return;
    }
    byte currentAnim = random(NUMBER_OF_BAONOW_ANIMATIONS);
    if (bitRead(BNAnimsUsed,currentAnim)) { // Prevent random repeats until all consumed
      #ifdef DEBUG
        Serial.print("Remaining BaoNow animation vector: ");
        Serial.println(BNAnimsUsed, BIN);
        Serial.print("Starting BaoNow anim: ");
        Serial.println(currentAnim);
      #endif
      baoNowAnimations[currentAnim]();
      bitClear(BNAnimsUsed,currentAnim);
    }
  }
}

/////////////////////////////////
//    Animations
/////////////////////////////////

void anim0() {  // Flicker on, On for 4 beats, flicker off
  setAllOff(); d4();
  flickerOn(_BCWW);
  d4();
  flickerOff(_BCWW); d4();
}

void flickerOn(byte message) {
  for (int i=0; i<6; i++) { // Flicker on
    setMessage(message); d(20+random(50));
    setAllOff(); d(random(200/(i+1)));
  }
  setMessage(message);
}
void flickerOff(byte message) {
  for (int i=0; i<4; i++) { // Flicker off
    setMessage(message); d(random(200/(i+1)));
    setAllOff(); d(random(100*(i+1)));
  }
}


void anim1() {  // On for 16s
  setAllOff();
  d4();
  setMessage(_BCWW);
  d(16000);
}
void anim2() {  // Slow blink
  for (int i=0; i<6; i++) {
    setMessage(_BCWW);
    d2();
    setAllOff();
    d2();
  }
}
void anim3() { // Slow sequence with space
  for (int i=0; i<3; i++) {
    setMessage(_B___); d1();
    setMessage(__C__); d1();
    setMessage(___W_); d1();
    setMessage(____W); d1();
  }
}
void anim4() { // Med sequence with space
  for (int i=0; i<3; i++) {
    setMessage(_B___); d1_2(); setAllOff(); d1_2();
    setMessage(__C__); d1_2(); setAllOff(); d1_2();
    setMessage(___W_); d1_2(); setAllOff(); d1_2();
    setMessage(____W); d1_2(); setAllOff(); d1_2();
  }
}
void anim5() { // Medium build
  for (int i=0; i<4; i++) {
    setMessage(_B___); d1_2();
    setMessage(_BC__); d1_2();
    setMessage(_BCW_); d1_2();
    setMessage(_BCWW); d1_2();
  }
}
void anim6() {
  int _bpm = bpm;
  for (int i=0; i<random(8); i++) {
    setTempo(100 + random(250));
    anim5();
    d(random(2000));
    anim3();
    d(random(2000));
    anim4();
    d(random(2000));
  }
  setTempo(_bpm);
}
void anim7() {
  int _bpm = bpm;
  for (int i=0; i<4+random(32); i++) {
    setTempo(100 + random(60));
    for (int j=0; j<8+random(8); j++) {
      switch(random(4)) {
        case 0: setMessage(_B___); break;
        case 1: setMessage(__C__); break;
        case 2: setMessage(___W_); break;
        case 3: setMessage(____W); break;
      }
      d1_4();
    }
  }
  setTempo(_bpm);
}
// Flip randomly
void anim8() {
  boolean states[numRelays] = { 0, };
  for (int i=0; i<30+random(30); i++) {
    byte pinToFlip = firstRelayPin + random(4);
    states[pinToFlip] = !states[pinToFlip];
    digitalWrite(pinToFlip, states[pinToFlip]);
    d(50 + random(500));
  }
}

void anim9() { // Chase with increasing temp, then backwards
  for (int i=0; i<1000; i+=9) {
    switch(i%4) {
      case 0: setMessage(_B___); break;
      case 1: setMessage(__C__); break;
      case 2: setMessage(___W_); break;
      case 3: setMessage(____W); break;
    }
    d(200*(cos(2*PI*i/1000)+1)/2);
  }
  d1();
  for (int i=0; i<1000; i+=9) {
    switch(i%4) {
      case 0: setMessage(____W); break;
      case 1: setMessage(___W_); break;
      case 2: setMessage(__C__); break;
      case 3: setMessage(_B___); break;
    }
    d(200*(cos(2*PI*i/1000)+1)/2);
  }
}
void anim10() { // Bao Bao..... chicka chicakaaahhhhh
  d4();
  setTempo(130);
  setMessage(_B___); d1_2(); setAllOff(); d1_2();
  setMessage(_B___); d1_2(); setAllOff(); d1_2();
  d2(); d4(); d8();
  setMessage(_B___); d1_2(); setAllOff(); d1_4();
  setMessage(_B___); d1_2(); setAllOff(); d1_4();
  d2(); d4(); d4(); d2(); d1_2();
  setMessage(__C__); d1_4(); setAllOff(); d1_4();
  setMessage(__C__); d1();
  setMessage(_B___); d1_2(); setAllOff(); d1_2();
  setMessage(_B___); d1_2(); setAllOff(); d1_2();
  d4();
}

void anim11() { // FlickerCandy
  d4();

  for (int i=0; i<7; i++) {
    setMessage(BAO); d(20+random(100*i+1));
    setAllOff(); d(random(50+300/(i+1)));
  }
  setMessage(BAO); d(2000);

  for (int i=0; i<7; i++) {
    setMessage(BAO); d(20+random(100));
    setAllOff(); d(random(50+200/(i+1)));
    setMessage(CHICKA); d(20+random(100));
  }
  setMessage(CHICKA); d(1200);

  for (int i=0; i<11; i++) {
    setMessage(CHICKA); d(20+random(100));
    setAllOff(); d(random(100));
    setMessage(WOW1); d(20+random(100));
    setAllOff(); d(random(75));
  }
  setMessage(WOW1); d(900);

  for (int i=0; i<17; i++) {
    setMessage(WOW1); d(20+random(200));
    setAllOff(); d(random(100));
    setMessage(WOW2); d(20+random(1+i*5));
    setAllOff(); d(random(75));
  }
  setMessage(WOW2); d(3000);

  setAllOff();
}



void baoNowStart() { //  Initial transition to BAO NOW mode
  DEBUG_PRINTLN("Starting BaoNow mode");
  setTempo(50);
  setAllOff(); d(4);
  setMessage(BAO_WOW); d(2);
  sign4toNSlow();
  for (int i=0; i<16; i++) {
    setMessage(HOT_BAO_NOW); d(100);
    setAllOff(); d(100);
  }
  const byte HBN[3] = {HOT, BAO, NOW};
  for (int i=0; i<18; i++) {
    setMessage(HBN[i%3]); d(100);
  }
  setAllOff(); d1();
  for (int i=1; i<=1024; i*=2) {
    setMessage(HOT_BAO_NOW); d(i);
    setAllOff(); d(1025-i);
  }
  setAllOff(); d1();
  setMessage(NOW); d2();
  setAllOff(); d1();
}

void baoNowAnim0() { // HOT + BAO + NOW
  for (int delayBase = 32; delayBase>=4; delayBase/=2) {
    setMessage(HOT); d(delayBase*100);
    setMessage(HOT_BAO); d(delayBase*100);
    setMessage(HOT_BAO_NOW); d(delayBase*100);
    setAllOff(); d(delayBase*50);
    setMessage(WOW2); d(delayBase*25);
    setAllOff(); d(delayBase*25);
  }
  d4();
}
void baoNowAnim1() { // HOT BAO, HOT NOW, WOW!
  setTempo(100);
  setAllOff(); d1();
  for (int i=0; i<4; i++) {
    setMessage(HOT_BAO); d1(); d1_4();
    setAllOff(); d1_8();
    setMessage(HOT_NOW); d1();
    setAllOff(); d1_4();
    setMessage(WOW2); d1_8();
    setAllOff(); d1_4(); d1_2();
  }
}

void baoNowAnim2() { // BAO, HOT, NOW, WOW!
  int _bpm = bpm;
  setAllOff(); d1();
  for (int i=0; i<4; i++) {
    setTempo(80+40*i);
    baoNowAnim2helper1();
  }
  for (int i=0; i<6; i++) {
    setMessage(HOT_BAO_NOW_WOW); d1_2();
    setAllOff(); d1_2();
  }
  for (int i=0; i<3; i++) {
    setMessage(HOT_BAO_NOW_WOW); d1_4();
    setAllOff(); d1_4();
  } d1_2();
  d4(); d4();
  setMessage(WOW2); d1_2(); setAllOff();
  d4();d4();
  setTempo(_bpm);
}
void baoNowAnim2helper1() { // BAO, HOT, NOW, WOW!
  for (int i=0; i<2; i++) {
    setMessage(BAO); d1();
    setAllOff(); d1_2();
    setMessage(HOT); d1_2();
    setMessage(NOW); d1_2();
    setAllOff();  d1_2();
    setMessage(WOW2); d1_2();
    setAllOff();  d1_2();
  }
}

void baoNowAnim3() { // Flicker BAO <wait> Flicker HOT NOW...
  int _bpm = bpm;
  setTempo(50);
  setAllOff(); d4();
  flickerOn(BAO); flickerOn(BAO);
  flickerOff(BAO);
  d2();
  flickerOn(HOT_NOW);
  d1();
  flickerOff(HOT_NOW);
  setTempo(_bpm);
}

void baoNowAnim4() { // HOT CHICKA BAO BAO, HOT CHICKA BAO <pause>
  int _bpm = bpm;
  setTempo(50);
  baoNowAnim4helper();
  setTempo(100);
  baoNowAnim4helper();
  setTempo(200);
  baoNowAnim4helper();
  setTempo(_bpm);
}
void baoNowAnim4helper() {
  for (int i=0; i<2; i++) {
    on_off(HOT, b1_2);
    on_off(CHICKA, b1_2);
    on_off(BAO, b1_2);
    on_off(BAO, b1_2);
    on_off(HOT, b1_2);
    on_off(CHICKA, b1_2);
    on_off(BAO, b1_2);
    d1_2();
  }
}
void on_off(byte message, int period) {
  setMessage(message); d(period/2);
  setAllOff(); d(period/2);
}

void baoNowAnim5() { // Freestyle MC. A Rap.
  d4();
  int _bpm = bpm;
  setTempo(90);

  on_off(HOT, b1_2);
  on_off(CHICKA, b1_2);
  on_off(CHICKA, b1_2);
  on_off(BAO, b1_2);
  on_off(BAO, b1_2);
  d1(); d1_2();

  on_off(BAO, b1_2);
  on_off(CHICKA, b1_2);
  on_off(CHICKA, b1_2);
  on_off(BAO, b1_2);
  on_off(WOW2, b1_2);
  d1(); d1_2();

  on_off(HOT, b1_2);
  on_off(CHICKA, b1_2);
  on_off(CHICKA, b1_2);
  on_off(BAO, b1_2);
  on_off(NOW, b1_2);
  d1(); d1_2();

  on_off(HOT, b1_2);
  on_off(BAO, b1_2);
  on_off(HOT, b1_2);
  on_off(NOW, b1_2);
  on_off(WOW2, b1_2);
  d1(); d1_2();

  on_off(BAO, b1_2);
  on_off(WOW2, b1_2);
  on_off(CHICKA, b1_2);
  on_off(BAO, b1_2);
  on_off(CHICKA, b1_2);
  on_off(BAO, b1_2);
  on_off(WOW2, b1_2);
  d1_2();

  on_off(NOW, b1_2);
  on_off(BAO, b1_2);
  on_off(HOT, b1_2);
  on_off(BAO, b1_2);
  on_off(CHICKA, b1_2);
  on_off(BAO, b1_2);
  on_off(WOW2, b1_2);
  d1_2();

  on_off(NOW, b1_2);
  on_off(BAO, b1_2);
  on_off(HOT, b1_2);
  on_off(BAO, b1_2);
  on_off(CHICKA, b1_2);
  on_off(BAO, b1_2);
  on_off(NOW, b1_2);
  d1_2();

  on_off(BAO, b1);
  on_off(CHICKA, b1);
  on_off(NOW, b1);
  on_off(WOW2, b1);

  on_off(BAO, b1_2);
  on_off(CHICKA, b1_2);
  on_off(CHICKA, b1_2);
  on_off(BAO, b1_2);
  on_off(BAO, b1_2);
  on_off(CHICKA, b1_2);
  on_off(CHICKA, b1_2);
  on_off(BAO, b1_2);
  on_off(BAO, b1_2);
  on_off(CHICKA, b1_2);
  on_off(NOW, b1_2);
  on_off(BAO, b1_2);
  on_off(HOT_NOW, b1);
  on_off(HOT_NOW, b1);


  setTempo(_bpm);
}

void baoNowAnim6() { // HOT NOW / WOW flickers
  d4();
  flickerOn(HB_N_); d2();
  flickerOn(HB_W_); d2();
  sign4toN(); d2();
  sign4toW(); d2();
  sign4toN(); d2();
  sign4toW(); d2();
  sign4toNSlow(); d4();
  sign4toW(); d2();
  flickerOn(H__W_); d2();
  sign4toNSlow(); d4();
  flickerOff(H__N_);
  setAllOff();
  sign4toW(); d2();
  flickerOn(_B_WW); d2();
  sign4toNSlow(); d1();
  flickerOff(_B_NW); d2();
  flickerOff(_B_NW); flickerOff(_B_NW); flickerOn(_B_NW); d1(); flickerOff(_B_NW);
  d4();
}


void baoNowAnim7() { // Flickers
  int _bpm = bpm;
  setTempo(50);
  setAllOff(); d4();
  flickerOn(HOT); flickerOn(HOT); d1(); flickerOn(HOT); flickerOff(HOT); d1();
  flickerOn(BAO); flickerOn(BAO); flickerOff(BAO); flickerOn(HOT); flickerOn(BAO); flickerOff(HOT); d1_2(); flickerOff(BAO); d1();
  flickerOn(BAO); flickerOn(NOW); d1_4(); flickerOff(BAO); flickerOn(NOW); flickerOff(BAO); flickerOn(NOW); d1(); flickerOff(NOW); d2();

  setTempo(_bpm);
}



////////////////////////////////////////////////
//    Utility functons used by the animations
///////////////////////////////////////////////

/* Takes a symbol that is a binary vector representation of the state of the 5 signs
     folowwed by the LSB that signifies if sign 4's realy should be in the "N" ("NOW") position
     So, "___ BAO CHICKA WOW WOW" = BCWW = B011110
*/
void setMessage(byte message) {
  // Only think about swapping sign4's position if it's going to be illuminated
  if (bitRead(message, NWOW_IDX)) {
    if (bitRead(message, 0)) sign4toN();
    else sign4toW();
  }
  #ifdef DEBUG
  // Serial.print("SetMessage: "); Serial.println(message,BIN);
  #endif

  message >>= 1; // Get ast N/W bit and into the on/off bits

  for (byte i = 0; i < numRelays; i++) {
    setSign(numRelays-1-i, bitRead(message >> i,0));
  }
}


// Doesn't check for servo state before turning off all lights in signs
void setAllOff() {
  for (int i = 0; i < numRelays; i++) {
    setSign(i, 0);
  }
}


// signIndex 0 is HOT, 4 is WOW!
void setSign(int signIndex, boolean state) {
  if (signIndex > numRelays) return;  // bounds checking

  // Pin high means relay interrupts the AC line voltage, turning off that rope light
  digitalWrite(firstRelayPin + signIndex, !state);
}

void sign4toW() {
  if (sign4isN) {
    nwServo.write(servoWPos);
    d(servoDelay);
    sign4isN = 0;
  }
}

void sign4toN() {
  if (!sign4isN) {
    nwServo.write(servoNPos);
    d(servoDelay);
    sign4isN = 1;
  }
}

void sign4toNSlow() {
  sign4toW();
  for (int t=0; t<3000; t++) {
    int pos = servoWPos + (servoNPos-servoWPos)*t/3000;
    nwServo.write(pos); d(1);
  }
  nwServo.write(servoNPos);
  sign4isN = 1;
}


///////////////////////////////
// Time, Delay, Speed Utilities
///////////////////////////////


void startupDiagnostic() {
  setMessage(H____); d1();
  setMessage(_B___); d1();
  setMessage(__C__); d1();
  setMessage(___W_); d1();
  setMessage(___N_); d1();
  setMessage(____W); d1();
  setMessage(_BCWW); d1();
  setAllOff();
}


void setTempo(unsigned int _bpm) {
  bpm = _bpm;
  beatPeriod = 1000 * 60 / _bpm;
  #ifdef DEBUG
//  Serial.print("\nBPM:" + String(_bpm) + "  Period: " + String(beatPeriod));
  #endif
  b1   = beatPeriod;
  b1_2 = beatPeriod/2;
  b1_4 = beatPeriod/4;
  b3_4 = beatPeriod*4/4;
  b1_8 = beatPeriod/8;
  b2   = beatPeriod*2;
  b4   = beatPeriod*4;
  b8   = beatPeriod*8;
}











