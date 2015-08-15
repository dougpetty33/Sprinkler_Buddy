#include <JsonArray.h>
#include <JsonHashTable.h>
#include <JsonObjectBase.h>
#include <JsonParser.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <SPI.h>
#include <TextFinder.h>
#include <EEPROM.h>
#include <String.h>

//need to tighten up variable names
//need to find a better way to store html
//need to find a way to be able to modify values inline
//possibly a function that serves up html appropriately



byte mac[] = {
  0,0,0,0,0,0};
byte ip[] = {
  192, 168, 0, 5};
IPAddress multi(239,255,255,250);
int multi_port = 1900;
  
byte zones[] = {
  1, 2, 4, 8, 16, 32, 64, 128};
byte zStatus[] = {
  false, false, false, false, false, false, false, false};
char buffer[200];
char packetBuffer[50];
char  ReplyBuffer[] = "http/1.1 200 ok\r\nST: zerolimits:sprinkler\r\n\r\n";  
char cBuffer[5];
char cmdBuffer[50];
byte currentState = 0;
EthernetClient client;
EthernetServer server(80);
EthernetUDP udp;
boolean isStatic = false;
const int net_config = 0;
const int mac_addy = 1;
const int ip_addy = 7;

//shift pins
const int data = 10;
const int clock = 5;
const int latch = 9;
const int OE = 6;
byte zone = 0;
byte state = 0;
char cTest = 'a';
char *svptr;
char *ip_str;



void setup(){
  Serial.begin(9600);
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);
  pinMode(latch, OUTPUT);
  pinMode(OE, OUTPUT);
  digitalWrite(OE, LOW);
  currentState = 0;
  updateStatus(-1, true);
  isStatic = EEPROM.read(0);
  for(int i = 0; i < 6; i++){
    mac[i] = EEPROM.read(mac_addy + i);
  }
  //If a static ip has been entered lets use that.
  if(isStatic == 1)
  {
            for(int i = 0; i < 4; i++){
              ip[i] = EEPROM.read(ip_addy + i);
        
          } 
    Ethernet.begin(mac, ip);
  }
  //The user has opted for DHCP
  else
  {
    //If DHCP has failed lets give it 5 sec before we try again.
    while(Ethernet.begin(mac) == 0)
    {
     Serial.println("oops");
      delay(5000);
   }
  }
  server.begin();
  udp.beginMulti(multi, multi_port, (uint8_t) 15);
  delay(3000);
  Serial.println(Ethernet.localIP());
}

void loop(){
  handleSsdp();
  client = server.available();
  //If the client is connected we should see what they want
  if(client){
    TextFinder finder(client);
    while (client.connected()) {
      if (client.available()) {
        //looks for the part of the string we care about
        if(finder.getString("","/", cBuffer,sizeof(cBuffer))){
          if (strcmp(cBuffer,"POST") == 0) {
            Serial.println("found Post");
            finder.find("\n\r");
            finder.find("\n");
            int k = 0;
            while(k < 50){
              cTest = client.read();
              if((byte)cTest == 255)
                break;
              else
                cmdBuffer[k] = cTest;
              k++; 
            }
            sendHeader(client);
            parseCmd(cmdBuffer);
            resetArray();
          }
          else if(strcmp(cBuffer, "GET" == 0)){
            Serial.println("found GET");
            sendHeader(client);
            client.println("Got your request bro");
          }
          Serial.println("a");
        }//END FINDER
        Serial.println("b");
      }//END CLIENT AVAILABLE
      delay(1);
      client.stop();
      Serial.println("c");
    }//END CLIENT CONNECTED
    Serial.println("d");
  }
}

void parseCmd(char jString[]){
  JsonParser<32> parser;
  JsonHashTable hashTable = parser.parseHashTable(jString);
  if(!hashTable.success())
  {
    Serial.println("We Failed");
    return;
  } 
  char* command = hashTable.getString("cmd");
  if(strcmp(command, "sys") == 0)
  {  
    if(hashTable.getBool("stat"))
    {
      ip_str = hashTable.getString("ip");
      strtok_r(ip_str, ".", &svptr);
    }
    else
    {
      EEPROM.write(net_config, false);
    }
  }
  else if(strcmp(command, "zone") == 0)
  {
    updateStatus((int)hashTable.getLong("zone"), hashTable.getBool("status")); 
  }
}

void updateStatus(int op, boolean state)
{
  digitalWrite(latch, LOW);
  //resets all the zones
  if(op == -1)
  {
    shiftOut(data, clock, MSBFIRST, 0);
    for(int i = 0; i < 8; i++)
    {
      zStatus[i] = false; 
    }
    digitalWrite(latch, HIGH);
    return;
  }
  //incase we are turning a zone on
  if(state == true)
  {
    //we are already on
    if(zStatus[op] == true)
    {
      return; 
    }
    currentState += zones[op];
    zStatus[op] = true;  
  } 
  //in case we want the zone off
  else
  {
    //we are already off
    if(zStatus[op] == false)
    {
      return; 
    }
    currentState -= zones[op];
    zStatus[op] = false;
  }
  Serial.println(currentState);
  shiftOut(data, clock, MSBFIRST, currentState);
  digitalWrite(latch, HIGH);
}

void handleSsdp()
{
   int packetSize = udp.parsePacket();
  if (packetSize)
  {
    Serial.print("Recieved SSDP");
    Serial.print("From ");
    IPAddress remote = udp.remoteIP();
    for (int i = 0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
        Serial.print(".");
      }
    }
    
    // read the packet into packetBufffer
    udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println(packetBuffer);

    // send a reply, to the IP address and port that sent us the packet we received
    udp.beginPacket(udp.remoteIP(), multi_port);
    udp.write(ReplyBuffer);
    udp.endPacket();
  }
  delay(10);
}

void resetArray()
{
  for(int i = 0; i < sizeof(cmdBuffer); i++)
  {
    cmdBuffer[i] = '\0';
  } 
}

void sendHeader(EthernetClient client){
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
}

void softReset(){
  wdt_enable(WDTO_30MS);
  while(true){
  }
}

int chcmp(char * string1, char * string2)
{
  if(sizeof(string1) != sizeof(string2))
    return -1;
  else
  {
    for(int i = 0; i < sizeof(string1); i++)
    {
      if(string1[i] != string2[i])
      {
        return -1;
      }
    }
    return 0;
  }
}














