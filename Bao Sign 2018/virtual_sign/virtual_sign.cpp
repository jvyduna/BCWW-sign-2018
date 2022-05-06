#include <unistd.h>
#include <sys/stat.h>

#include "virtual_sign.h"

boolean dirty;
boolean HOT = 0;
boolean BAO = 0;
boolean CHICKA = 0;
boolean NWOW = 0;
boolean WOW = 0;
char NW = '?';

Ser Serial;

void Ser::begin(int baud) {
}

void Ser::print(int i) {
  fprintf(stderr, "%d", i);
}

void Ser::println(int i) {
  fprintf(stderr, "%d\n", i);
}

void Ser::print(const char *s) {
  fprintf(stderr, "%s", s);
}

void Ser::println(const char *s) {
  fprintf(stderr, "%s\r\n", s);
}

void Ser::println(int n, int bin) {
  if (bin != BIN) {
    fprintf(stderr, "Weird print call\n");
  } else {
    for (int i = 0; i < 12; i++) {
      fprintf(stderr, "%c", (n & 2048 >> i) ? '1' : '0');
    }
    fprintf(stderr, "\n");
  }
}

Servo::Servo(void) {

}

void Servo::attach(int pin, int i1, int i2) {

}

void Servo::write(int i) {
  if (i == 40) NW = 'N';
  else if (i == 140) NW = 'W';
  else NW = 'X';
  dirty = 1;
}

uint32_t random(uint32_t maxPlusOne) {
  return rand() % maxPlusOne;
}


inline bool file_exists (const char * name) {
  struct stat buffer;   
  return (stat (name, &buffer) == 0); 
}

int digitalRead(int pin) {
  switch(pin) {
    case 8:
      return !file_exists("/tmp/bao.touch");
    default:
      printf("Unknown pin %d\r\n", pin);
      while(1);
  }
}

void digitalWrite(int pin, int value) {
  value = !value;  // Caller sets to nonzero to pull low
  if (pin == 2) HOT = value;
  else if (pin == 3) BAO = value;
  else if (pin == 4) CHICKA = value;
  else if (pin == 5) NWOW = value;
  else if (pin == 6) WOW = value;
  else {
    fprintf(stderr, "Unknown pin %d\r\n", pin);
    while(1);
  }
  dirty = 1;
}

double analogRead(int pin) {
  return 0.0;
}

void set_color(boolean on) {
  if (on) {
    printf("\e[91m");
  } else {
    printf("\e[90m");
  } 
}

void print_sign() {
  char nwow[4] = { '_', 'O', 'W', '\0' };
  nwow[0] = NW;

  printf("\033[F");  // Return to start of previous line
  set_color(HOT); printf("HOT ");
  set_color(BAO); printf("BAO ");
  set_color(CHICKA); printf("CHICKA ");
  set_color(NWOW); printf("%s ", nwow);
  set_color(WOW); printf("WOW!");
  printf("\e[0m\n"); 
  dirty = 0;
}

void delay(int msec) {
  if (dirty) print_sign();
  usleep(msec * 1000);
}

void setup();
void loop();
int main(int argc, char **argv) {
  dirty = 1;
  printf("\n");
  setup();
  while (1) {
    loop();
    delay(0);
  }
}
