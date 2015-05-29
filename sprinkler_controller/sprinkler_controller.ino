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

const char string_0[] PROGMEM = "<script src=\"http://ajax.googleapis.com/ajax/libs/jquery/1.11.0/jquery.min.js\">";
const char string_1[] PROGMEM = "</script><script>var url = \"192.168.0.5\"; $(document).ready(function(){";
const char string_2[] PROGMEM =  "$(\"#net\").click(function(e){e.preventDefault(); $.post(url, '{\"cmd\":\"sys\", \"stat\": ' + $(\"#stat\").prop(\"checked\") + ', \"ip\": ' + $(\"#ip\").val() + '}');});";
const char string_3[] PROGMEM =  "$(\"input[name^=z_]:radio\").change(function(){";
const char string_4[] PROGMEM =  "var zone_radio = $(this); var zone = zone_radio.attr(\"data-zone\");";
const char string_5[] PROGMEM =  "var zone_status = $('input:radio[name=' + zone_radio.attr(\"name\") + ']:checked').val();";
const char string_6[] PROGMEM =  "$.post(url,'{\"cmd\":\"zone\", \"zone\": ' + zone + ', \"status\": ' + zone_status + '}');});});";
const char string_7[] PROGMEM = "</script></head><body style=\"background-color:#E8E8E8;\"><center><h1>Little Victories</h1><p style=\"background-color:black;\"><br></p>";
const char string_8[] PROGMEM = "<h2>System Settings</h2><table><tr><td>";
const char string_9[] PROGMEM = "DHCP:</td><td><input type=\"radio\" id=\"dhcp\" checked name =\"net1\" value=\"true\"><br></td></tr>";
const char string_10[] PROGMEM = "<tr><td>Static</td><td><input type=\"radio\" id=\"stat\" name=\"net1\" value=\"true\"><input type=\"text\" id=\"ip\"></td></tr><br>";
const char string_11[] PROGMEM = "</table><button id=\"net\">Apply Network Settings</button>";
const char string_12[] PROGMEM = "<p style=\"background-color:black;\"><br></p><center><h2>Zone Settings</h2></center>";
const char string_13[] PROGMEM = "Zone 1:<input type=\"radio\" data-zone=\"0\" value=\"1\" name=\"z_0\">On<input type=\"radio\" data-zone=\"0\" value=\"0\" name=\"z_0\" checked>Off<br>";
const char string_14[] PROGMEM = "Zone 2:<input type=\"radio\" data-zone=\"1\" value=\"1\" name=\"z_1\">On<input type=\"radio\" data-zone=\"1\" value=\"0\" name=\"z_1\" checked>Off<br>";
const char string_15[] PROGMEM = "Zone 3:<input type=\"radio\" data-zone=\"2\" value=\"1\" name=\"z_2\">On<input type=\"radio\" data-zone=\"2\" value=\"0\" name=\"z_2\" checked>Off<br>";
const char string_16[] PROGMEM = "Zone 4:<input type=\"radio\" data-zone=\"3\" value=\"1\" name=\"z_3\">On<input type=\"radio\" data-zone=\"3\" value=\"0\" name=\"z_3\" checked>Off<br>";
const char string_17[] PROGMEM = "Zone 5:<input type=\"radio\" data-zone=\"4\" value=\"1\" name=\"z_4\">On<input type=\"radio\" data-zone=\"4\" value=\"0\" name=\"z_4\" checked>Off<br>";
const char string_18[] PROGMEM = "Zone 6:<input type=\"radio\" data-zone=\"5\" value=\"1\" name=\"z_5\">On<input type=\"radio\" data-zone=\"5\" value=\"0\" name=\"z_5\" checked>Off<br>";
const char string_19[] PROGMEM = "Zone 7:<input type=\"radio\" data-zone=\"6\" value=\"1\" name=\"z_6\">On<input type=\"radio\" data-zone=\"6\" value=\"0\" name=\"z_6\" checked>Off<br>";
const char string_20[] PROGMEM = "Zone 8:<input type=\"radio\" data-zone=\"7\" value=\"1\" name=\"z_7\">On<input type=\"radio\" data-zone=\"7\" value=\"0\" name=\"z_7\" checked>Off<br></center>";
const char string_21[] PROGMEM = "</body></html>";

const char multicast_response[] PROGMEM = "HTTP/1.1 200 OK\r\nMX: 5\r\nST: upnp:rootdevice\r\nMAN: \"sddp:discover\"\r\nUser-Agent: UPnP/1.0 DLNADOC/1.50 Platinum/1.0.4.11\r\nConnection: close\r\nHost: 239.225.225.250:1900\r\n\r\n\r\n";

const char *config_page[] = {
  string_0,
  string_1,
  string_2,
  string_3,
  string_4,
  string_5,
  string_6,
  string_7,
  string_8,
  string_9,
  string_10,
  string_11,
  string_12,
  string_13,
  string_14,
  string_15,
  string_16,
  string_17,
  string_18,
  string_19,
  string_20,
  string_21
};

byte mac[] = {
  0,0,0,0,0,0};
byte ip[] = {
  192, 168, 0, 5};
byte zones[] = {
  1, 2, 4, 8, 16, 32, 64, 128};
byte zStatus[] = {
  false, false, false, false, false, false, false, false};
char buffer[200];
char cBuffer[5];
char cmdBuffer[50];
byte currentState = 0;
EthernetClient client;
EthernetUDP udp;
EthernetServer server(80);
byte multi_ip[] = {239, 225, 225, 250};
const unsigned int multi_port = 1900;
boolean isStatic = false;
const int net_config = 0;
const int mac_addy = 1;
const int ip_addy = 7;

char packetBuffer[UDP_TX_PACKET_MAX_SIZE];
byte remoteIp[4];
unsigned int remote_port;

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
  delay(3000);
  Serial.println(Ethernet.localIP());
  udp.beginMulti(multi_ip, multi_port);
}

void loop(){
  client = server.available();
  int packetSize = udp.parsePacket();
  if(packetSize)
  {
    //query
    //todo: need to look to see if the multicast was meant for us
     IPAddress remote = udp.remoteIP();
     udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
     Serial.println("packet:");
     Serial.println(packetBuffer);
     
     //response
     udp.beginPacket(remote, udp.remotePort());
     udp.write(strcpy_P(buffer, (char*)pgm_read_word(&(multicast_response))));
     udp.endPacket();
   }
  
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
            sendMain(client);
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

//Ex: {"cmd":"zone", "zone":"1","status":"1"}
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

void sendMain(EthernetClient client){
  client.print("<html>");
  client.println("<body>");
  client.println("<form name=""net_input"" action=""/");
  client.println(""" method='post'>");
  for(int i = 0; i < 21; i++)
  {
    strcpy_P(buffer, (char*)pgm_read_word(&(config_page[i])));
    client.println( buffer );
  }
  delay(5);
  client.stop();
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














