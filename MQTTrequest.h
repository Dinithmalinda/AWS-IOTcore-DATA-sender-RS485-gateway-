
void Publishinstruction(String func,String Ins){
  char jsonBuffer[200];
   StaticJsonDocument<200> doc; 
    doc["ID"] = "CON";
    doc["FUNC"]=func;
    doc["INST"]=Ins;    
    serializeJson(doc, jsonBuffer);
    client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);  
    Serial.println(jsonBuffer);
  }

byte ipCheckcounter=100;  
String getIp(){    
  String line="NOERROR";
  EthernetClient client;
byte kk;
for (kk=0;kk<5;kk++){  
  if (client.connect("api.ipify.org", 80)) { 
    client.println("GET / HTTP/1.0"); client.println("Host: api.ipify.org");client.println();
    break;}  }
if(kk>4){Serial.println("Connection to ipify.org failed");return line;}
  delay(1000);  
  while(client.available())
  { line = client.readStringUntil('\n');
    //Serial.println(line);
  }
  client.stop();
  return line;
}


void messageHandler(char* topic, byte* payload, unsigned int length) {
  char jsonBuffer[512];
  Serial.print("incoming: ");
  Serial.println(topic);
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["method"];  
  Serial.println(message);
if("reset"==doc["method"]) {Publishinstruction("SYSreset","Ok"); ESP.restart();}
if("IPAdd"==doc["method"]) { Publishinstruction("IPadd",getIp());}
if("LDPOINT"==doc["method"]) {long numb = doc["NUMB"];ID=numb ;Serial.println(numb);}
if("POWreset"==doc["method"]) {Publishinstruction("powersupply_reset","OK");digitalWrite(POWRESET,true);}    
if("MODBUSread"==doc["method"]) {long coil= doc["coil"];
                                 long address =doc["address"];
                                  modbusREQ = MODBUSMQTTrequest;
                                  modbus.readHoldingRegisters(coil,address,2); 
                                 }
}
