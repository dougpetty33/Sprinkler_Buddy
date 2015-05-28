#include <Ethernet.h>
#include <SPI.h>

int latchPin = 7;
int clockPin = 9;
int dataPin = 8;

byte mac[] ={0x00, 0x0f, 0xff, 0x00, 0x00, 0xa2};


void setup() {
  //set pins to output so you can control the shift register
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  Ethernet.begin(mac);
}

void loop() {
  // count from 0 to 255 and display the number 
  // on the LEDs
  for (int numberToDisplay = 0; numberToDisplay < 256; numberToDisplay++) {
    // take the latchPin low so 
    // the LEDs don't change while you're sending in bits:
    digitalWrite(latchPin, LOW);
    // shift out the bits:
    shiftOut(dataPin, clockPin, MSBFIRST, numberToDisplay);  
//
//  for(int i = 0; i < 8; i++)
//  {
//     digitalWrite(clockPin, HIGH);
//    digitalWrite(clockPin, LOW); 
//  }
    //take the latch pin high so the LEDs will light up:
    digitalWrite(latchPin, HIGH);
    // pause before next value:
    delay(500);
  }
}
