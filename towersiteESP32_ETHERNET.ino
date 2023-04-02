#include <SPI.h>
#include <Ethernet.h>
#include <SSLClient.h>
#include <PubSubClient.h>
#include "AWS_Root_CA.h" // This file is created using AmazonRootCA1.pem from https://www.amazontrust.com/repository/AmazonRootCA1.pem
#include <ArduinoJson.h>
#include "SENSOR_INPUT.h"
#include "secrets.h"

void messageHandler(char* topic, byte* payload, unsigned int length);

EthernetClient ethClient;
SSLClient ethClientSSL(ethClient, TAs, (size_t)TAs_NUM, A5);
PubSubClient client(mqttServer, 8883, messageHandler, ethClientSSL);
byte mac[] = {0x00, 0xAA, 0xBB, 0xCC, 0xDE, 0x02};

#include "MQTTrequest.h"


void connectAWS()
{ Ethernet.init(5);
  ethClientSSL.setMutualAuthParams(mTLS);
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    }
    else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
 return;
  }

  Serial.print("My IP address: ");
  Serial.println(Ethernet.localIP()); 
  Serial.print("Connecting to AWS IOT");
  while (!client.connect(THINGNAME)) {
    Serial.print(".");
    delay(100);
  }
  if (!client.connected()) {
    Serial.println("AWS IoT Timeout!");
    return;
  }
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
  Serial.println("AWS IoT Connected!");
  Publishinstruction("Condat","Connected");
}

void publishMessage()
{
  char jsonBuffer[512];
  StaticJsonDocument<300> doc;
  doc["ID"] = ID;
  copyArray(environment, doc["ENV"].to<JsonArray>());
  copyArray(electricalparameters, doc["ELE"].to<JsonArray>());
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  doc.clear();
  doc["ID"] = ID;
  copyArray(PowerConsumption, doc["POW"].to<JsonArray>());
  copyArray(GeneratorSpecs, doc["GES"].to<JsonArray>());
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  doc.clear();
  doc["ID"] = ID;
  copyArray(baterybackup, doc["BBS"].to<JsonArray>());
  copyArray(humaninteraction, doc["HMI"].to<JsonArray>());
  copyArray(switches, doc["SWC"].to<JsonArray>());
  serializeJson(doc, jsonBuffer); // print to client
  client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
}



void setup() {

  Serial.begin(115200);
  configureIOpins();
  modbusinit();
  connectAWS();

}
int timecounter = 0;
static uint32_t lastMillis = 0;
void loop() {

  if (millis() - lastMillis > 150) {
    lastMillis = millis();
    if (timecounter == 0)readGenerator();
    if (timecounter == 60)DHT21sen();
    if (timecounter % 18 == 0) digitalINPUTS();
    //if(timecounter ==90)readPMvoltage();
    if (timecounter == 120)readPMpower();
    if (timecounter == 150)readPMfrequncy();
    if (timecounter == 180)readPMpowerrfactor();
    if (timecounter % 30 == 0) readADC();
    
    if (timecounter == 40){ipCheckcounter++; 
                            if(ipCheckcounter>30){ipCheckcounter=0;
                                              {Publishinstruction("IPadd",getIp());}                            
                            } }      
    timecounter++;
  }

  if (timecounter > 220) {
    ID++; if (ID > limitID)ID = 1;
    timecounter = 0;
if (!client.connected())connectAWS();
if (client.connected())publishMessage();
    printvalues();
  environment[2] =false;environment[3]=false;humaninteraction[0]=false;
  humaninteraction[1]=false;humaninteraction[2]=false;     
}
  client.loop();
 if(modbusMQTTflag==1){Publishinstruction("MODBUSread",String(modbusMQTTdata,2));modbusMQTTflag=0;}

  
}
