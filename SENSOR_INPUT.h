////////////////////DHTsensor//////////////////////////////
#include "DHT.h"

#define DHTPIN 15//LABLE (+3.3v/PT1+ ,DHTDATA/PT1- ,GND/GND) 

#define DHTTYPE DHT21   // DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
////////////////////DHTsensor//////////////////////////////
////////////////////modbus credencials///////////////////
#include <esp32ModbusRTU.h>
esp32ModbusRTU modbus(&Serial2, 4);


#define GENREADINGS 100
#define PMvoltage 101
#define PMpower 102
#define PMfrequncy 103
#define PMpowerrfactor 104
#define MODBUSMQTTrequest 105

#define motion 14 //LABLE DIN-7
#define door1 26//LABLE DIN-5
#define door2 25//LABLE DIN-6
#define flood 32//LABLE DIN-8
#define Smoke 27//LABLE DIN-4

#define Batterycurrent 35//LABLE (PT2/5v ,GNG/GND ,PT3+/ADCin)
#define BatteryVoltage  34//LABLE (PT3-)
#define BatteryEXT 39//LABLE (ADC/GND*1)
#define BatteryEXT2 36//LABLE (ADC/GND*2)

#define EXTIN 12   //LABLE 
#define switch1 33 //LABLE 

#define IRLED 22   //LABLE DIN7
#define LED 21     //LABLE DIN7
#define POWRESET 2 //LABLE DIN7
#define EXT2 13    //LABLE DIN7 

#define sensorerror 1212.12;

byte modbusMQTTflag=0;
float modbusMQTTdata=0;
byte modbusREQ = 0;
/////////////////////////modbus credencials/////////////////////////
long ID = 0, limitID = 2102400;
bool switches[] = {0, 0, 0, 0, 0, 0}; // PosSelect,EGonoff,AC12,BuALonoff,lightonoff;
bool humaninteraction[] = {1, 0, 0}; //door1,door2,PIR;
float environment[] = {29.6, 78.2, 0, 0}; //temperature,humidity,smoke,flood;
float electricalparameters[] = {232.6, 0, 0, 0, 0, 0}; //gridvoltage , generatorvoltage , frequncy , PF1, PF2,PF3
float PowerConsumption[] = {2.6,.3, 0, 0}; //Rectifire , AC , other ,total
float GeneratorSpecs[] = {75, 350, 100, 12.8, 0}; //Fuellevel,fuelcapacity,Batterylevel,outputvoltage
float baterybackup[] = {100,52.9, 0, 0}; //batery level,batteryvoltage,chargecurrent,drawcurrent


float roundTodecimal(float xx){
  int yy= xx*10;
  xx=yy/10.0;
  return xx;
  
  }


float Arraytofloat(byte b[],byte point){
union { float f; byte b[4];} u;
for(byte kk=0;kk<4;kk++)u.b[3-kk] = b[kk+(point*4)];
if(u.f==0)return (-0);
return roundTodecimal(u.f);
}
/////////////////////////modbus INITreturn////////////////////////////
void modbusinit() {
  Serial2.begin(9600);
  modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint16_t address, uint8_t* data, size_t length) {
    if (modbusREQ == GENREADINGS) {
      int kk;
  kk=(data[4] << 8) | data[5]; electricalparameters[0]=roundTodecimal(kk/25.0);
      kk = (data[6] << 8) | data[7];electricalparameters[1]=roundTodecimal(kk/26.0);
        kk=(data[2] << 8) | data[3]; GeneratorSpecs[3]=roundTodecimal(kk/27.00);//fuel level
         kk=(data[0] << 8) | data[1]; GeneratorSpecs[0]=roundTodecimal(kk/276.00);//12v battery        
     // for (size_t i = 0; i < length; ++i) {Serial.printf("%02x", data[i]);}
      // Serial.println(electricalparameters[0]);Serial.println(electricalparameters[1]);Serial.println(GeneratorSpecs[1]);Serial.println(GeneratorSpecs[4]);
    }
if (modbusREQ == PMvoltage){electricalparameters[0]=Arraytofloat(data,0);}
if (modbusREQ == PMpower){PowerConsumption[0]=Arraytofloat(data,0);                           
                          PowerConsumption[1]=Arraytofloat(data,1); 
                          PowerConsumption[2]=Arraytofloat(data,2); 
                          PowerConsumption[3]=Arraytofloat(data,3);} 
if (modbusREQ == PMfrequncy) {electricalparameters[2]=Arraytofloat(data,0);}
if (modbusREQ == PMpowerrfactor) {electricalparameters[3]=Arraytofloat(data,0); 
                                   if(electricalparameters[3]<-2||electricalparameters[3]>2)electricalparameters[3]=0;
                                  electricalparameters[4]=Arraytofloat(data,1); 
                                  if(electricalparameters[4]<-2||electricalparameters[4]>2)electricalparameters[4]=0;
                                  electricalparameters[5]=Arraytofloat(data,2);
                                  if(electricalparameters[5]<-2||electricalparameters[5]>2)electricalparameters[5]=0;
                                  }
if (modbusREQ == MODBUSMQTTrequest) {modbusMQTTflag=1;modbusMQTTdata=Arraytofloat(data,0);}                            
 modbusREQ = 0;});
  
  modbus.onError([](esp32Modbus::Error error) {
    Serial.printf("error: 0x%02x\n\n", static_cast<uint8_t>(error)); modbusREQ = 0;
if (modbusREQ == GENREADINGS) { electricalparameters[0]=sensorerror;electricalparameters[1]=sensorerror;
                                GeneratorSpecs[0]=sensorerror;GeneratorSpecs[2]=sensorerror;}    
if (modbusREQ == PMvoltage){electricalparameters[0]=sensorerror;}
if (modbusREQ == PMpower){PowerConsumption[0]=sensorerror;                           
                          PowerConsumption[1]=sensorerror; 
                          PowerConsumption[2]=sensorerror; 
                          PowerConsumption[3]=sensorerror;} 
if (modbusREQ == PMfrequncy) {electricalparameters[2]=sensorerror;}
if (modbusREQ == PMpowerrfactor) {electricalparameters[3]=sensorerror; 
                                  electricalparameters[4]=sensorerror; 
                                  electricalparameters[5]=sensorerror; }   
 });
  modbus.begin();
}
////////////////////modbus INITreturn///////////////////



void configureIOpins() {
  dht.begin();
  pinMode(Batterycurrent,INPUT);
  pinMode(BatteryVoltage,INPUT);
  pinMode(Smoke, INPUT_PULLUP);
  pinMode(flood, INPUT_PULLUP);
  pinMode(door1, INPUT_PULLUP);
  pinMode(door2, INPUT_PULLUP);
  pinMode(EXTIN, INPUT_PULLUP);
  pinMode(motion, INPUT_PULLUP);
  pinMode(switch1, INPUT_PULLUP);    
  pinMode(POWRESET  , OUTPUT);digitalWrite(POWRESET,false);
  pinMode(EXT2  , OUTPUT);digitalWrite(EXT2,false);
  pinMode(IRLED ,OUTPUT);digitalWrite(IRLED,false);
  pinMode(LED ,OUTPUT); digitalWrite(LED,false);  
}

void digitalINPUTS() {

  if(digitalRead(Smoke))environment[2] =true;
   if(!digitalRead(flood))environment[3]=true;
    if(digitalRead(door1))humaninteraction[0]=true;
     if(digitalRead(door2))humaninteraction[1]=true;
      if(!digitalRead(motion))humaninteraction[2]=true;
}

byte modbuscounter=0;
void readGenerator() {
  if(modbuscounter>3 ){modbuscounter=0;modbusREQ=0;}
  if(modbusREQ!=0){modbuscounter++;return;}
  modbusREQ = GENREADINGS;
  modbus.readHoldingRegisters(1, 0x9c4A, 4);
}


void readPMvoltage(){
  if(modbusREQ!=0)return;  
    modbusREQ = PMvoltage;
    modbus.readHoldingRegisters(9,0xBD3,2);   
}

void readPMpower(){
  if(modbusREQ!=0)return;
    modbusREQ = PMpower ;
    modbus.readHoldingRegisters(9,0xBFD,8);   
}
void readPMfrequncy(){
  if(modbusREQ!=0)return;
    modbusREQ = PMfrequncy ;
    modbus.readHoldingRegisters(9,0xC25,2);   
}
void readPMpowerrfactor(){
  if(modbusREQ!=0)return;
    modbusREQ = PMpowerrfactor;
    modbus.readHoldingRegisters(9,0xC05,6);   
}




void DHT21sen() {
float h,t,f;
  for(byte b=0;b<5;b++){
  h = dht.readHumidity(); // Read temperature as Celsius (the default)
  t = dht.readTemperature(); // Read temperature as Fahrenheit (isFahrenheit = true)
  f = dht.readTemperature(true); // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {    
   // Serial.println(F("Failed to read from DHT sensor!"));
   //environment[0] =sensorerror ; environment[1] = sensorerror;
   // return;
   delay(1000);
  }
  else { environment[0] = roundTodecimal(t);
         environment[1] = roundTodecimal(h);
         return;}
  }
  environment[0] =sensorerror ; environment[1] = sensorerror;
//  Serial.print(F("Humidity: ")); Serial.print(h); Serial.print(F("%  Temperature: ")); Serial.print(t); Serial.println(F("°C "));
}

void readADC() {
int tempCurrent=0,tempvolt=0;
for(byte i=0;i<16;i++){tempCurrent+=analogRead(Batterycurrent);tempvolt+=analogRead(BatteryVoltage);}
tempCurrent/=16.0;
if(tempCurrent>1330){baterybackup[2]=roundTodecimal((baterybackup[2]+(tempCurrent-1330)/16.0)/2);
                     baterybackup[3]=0;}                     
else if(tempCurrent<1320){baterybackup[3]=roundTodecimal((baterybackup[3]+(1320-tempCurrent)/16.0)/2);
                          baterybackup[2]=0;}
else {baterybackup[3]=0;baterybackup[2]=0;}
  baterybackup[1]=roundTodecimal((baterybackup[1]+tempvolt/812.00)/2);   
   
  if(baterybackup[1]>51.5)baterybackup[0]=(baterybackup[1]-44)/11.2*100;
  else baterybackup[0]=(baterybackup[1]-32)/19.2*100;
  
  if(baterybackup[0]>100)baterybackup[0]=100;
  if(baterybackup[0]<1)baterybackup[0]=0;
}


void printvalues(){
//  Serial.println("Humidity:Temperature ");Serial.print(environment[0]); Serial.print(":");Serial.println(environment[1]); 
//  
//   Serial.println("baterry backup");Serial.print(baterybackup[0]);Serial.print(baterybackup[1]);
//                                     Serial.print(baterybackup[2]);Serial.println(baterybackup[3]);
// if(environment[3])Serial.println("Smoke:detected"); else Serial.println("Smoke:NOT"); 
// if(environment[2])Serial.println("FLOOD:detected"); else Serial.println("FLOOD:NOT");
// if(humaninteraction[0]) Serial.println("Door1:detected");else Serial.println("Door1:NOT");
// if(humaninteraction[1])Serial.println("Door2:detected"); else Serial.println("Door2:NOT");
// if(humaninteraction[2])Serial.println("motion:detected");else Serial.println("motion:NOT");
// 
//
//Serial.println(electricalparameters[0]);
//Serial.println(electricalparameters[1]);
//Serial.println(electricalparameters[2]);
//Serial.println(electricalparameters[3]);
//Serial.println(electricalparameters[5]);
//Serial.println(electricalparameters[4]);
//Serial.println(PowerConsumption[3]); 
//Serial.println(PowerConsumption[2]);
//Serial.println(PowerConsumption[1]);



 char jsonBuffer[512];
  StaticJsonDocument<300> doc;
  doc["ID"] = ID;
  copyArray(environment, doc["ENV"].to<JsonArray>());
  copyArray(electricalparameters, doc["ELE"].to<JsonArray>());
  serializeJson(doc, jsonBuffer); // print to client  
Serial.println(jsonBuffer);
doc.clear();
  doc["ID"] = ID;
  copyArray(PowerConsumption, doc["POW"].to<JsonArray>());
  copyArray(GeneratorSpecs, doc["GES"].to<JsonArray>());
  serializeJson(doc, jsonBuffer); // print to client
  
Serial.println(jsonBuffer);
doc.clear();
   doc["ID"] = ID;
  copyArray(baterybackup, doc["BBS"].to<JsonArray>());
  copyArray(humaninteraction, doc["HMI"].to<JsonArray>());
  copyArray(switches, doc["SWC"].to<JsonArray>());
 serializeJson(doc, jsonBuffer); // print to client  
Serial.println(jsonBuffer);
  Serial.println("");
}
