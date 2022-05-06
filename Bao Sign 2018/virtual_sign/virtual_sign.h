#ifndef VIRTUAL_SIGN_H
#define VIRTUAL_SIGN_H

#include <stdio.h>
#include <cstdlib>
#include <stdint.h>
#include <math.h>

#define LOW 0
#define HIGH 255
#define PI 3.14159265

typedef uint8_t byte;
typedef int boolean;

#define pinMode(p, m)
#define randomSeed(n)
uint32_t random(uint32_t maxPlusOne);

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) ((bitvalue) ? bitSet(value, bit) : bitClear(value, bit))

int digitalRead(int pin);
double analogRead(int pin);
void digitalWrite(int pin, int value);
void delay(int msec);

#define BIN 123
class Ser {
 public:
  void begin(int baud);
  void print(const char *s);
  void print(int i);
  void println(int i);
  void println(const char *s);
  void println(int n, int bin);
  operator bool() { return true; }
};

extern Ser Serial;

class Servo {
  public:
    Servo(void);
    void attach(int pin, int i1, int i2);
    void write(int i);
};


#endif
