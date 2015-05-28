#include <EEPROM.h>
#include <SPI.h>
int offset = 0;
void setup(){
  Serial.begin(9600);
  byte mac[]={ 
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED   };
  byte ip[] = {
    192,168,33,165  };
  boolean isStatic = false;

  EEPROM.write(offset, isStatic);
  offset += 1;
  for(int i = 0; i < 6; i++)
  {
    EEPROM.write(offset, mac[i]);
    offset += 1; 
  }

  for(int i = 0; i < 4; i++)
  {
    EEPROM.write(offset, ip[i]);
    offset += 1; 
  }


}
void loop(){


}

