#include <OneWire.h>
#include <DallasTemperature.h>
#include <EEPROM.h>

int R1= 1000;
int Ra=22;  //arduino Ra=25; eso32 Ra=22
int ECPin= 27;    //INPUT   
int ECGround=25;  //OUTPUT 
int ECPower =33;  //OUTPUT  
   
float PPMconversion=0.7;  //セル定数　[USA] PPMconverion:  0.5  [EU] PPMconversion:  0.64  [Australia] PPMconversion:  0.7
float TemperatureCoef = 0.019; //温度補正 0.02が多い。
float K=2.88; 
  

#define ONE_WIRE_BUS 32 
const int TempProbePossitive =26;  //ds18b20 VCC

#define INPUMP 5
#define OUTPUMP 15
   
#define EEPROM_SIZE 512
#define MOTOR_POWER 50

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
   
float Temperature;
float EC=0;
float EC25 =0;
int ppm =0; 
float raw= 0;
float Vin= 3.3;  //5;
float Vdrop= 0;
float Rc= 0;
float buffer=0;
  
void setup(){
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  pinMode(TempProbePossitive , OUTPUT );
  digitalWrite(TempProbePossitive , HIGH );
  pinMode(ECPin,INPUT);
  pinMode(ECPower,OUTPUT);
  pinMode(ECGround,OUTPUT);
  digitalWrite(ECGround,LOW);
  
  delay(100);
  sensors.begin();
  delay(100);
   R1=(R1+Ra); 

}
void loop(){
  // CleanBoard();

  // InWater(5000);
  GetEC();
  // OutWater();

  //int data;
  //EEPROM.get(0, data);
  
  Serial.print(EC25);
  //PumpOn(INPUMP);
  //analogWrite(INPUMP, 255);
  delay(1000);
    
}
void GetEC(){
  
  sensors.requestTemperatures();
  Temperature=sensors.getTempCByIndex(0);
    
  Temperature = 25;

  digitalWrite(ECPower,HIGH);
  raw= analogRead(ECPin);
  delay(2);              //esp32の計算速度が速いので2ミリ秒待機
  raw= analogRead(ECPin);//静電容量の影響を抑えて再度計測
  digitalWrite(ECPower,LOW);
    
  Vdrop=(Vin*raw)/4096.0; //arduinoでは4096→1024に変更する。
  Rc=(Vdrop*R1)/(Vin-Vdrop);
  Rc=Rc-Ra; //acounting for Digital Pin Resitance
  EC = 1000/(Rc*K);
    
  EC25  =  EC/ (1+ TemperatureCoef*(Temperature-25.0));
  ppm=(EC25)*(PPMconversion*1000);

  
}
  
void PrintReadings(){
  Serial.print("Rc: ");
  Serial.print(Rc);
  Serial.print(" EC: ");
  Serial.print(EC25);
  Serial.print(" Simens  ");
  Serial.print(ppm);
  Serial.print(" ppm  ");
  Serial.print(Temperature);
  Serial.println(" *C ");
}

void InWater(int msec){
  PumpOn(INPUMP);
  delay(msec);
  PumpOff(INPUMP);
}

void OutWater(){
  PumpOn(OUTPUMP);
  while (true)
  {
    if(1/*水位が0なら*/){
      PumpOff(OUTPUMP);
      break;
    }
    delay(100);
  }
  
}

void CleanBoard(){
  InWater(5000);
  OutWater();
}

void WriteData(){
  int eeAddress;
  EEPROM.get(0, eeAddress);

  EEPROM.put(eeAddress, EC25);
  EEPROM.commit();
}

void PumpOn(int pump){
  analogWrite(pump, 255 * (MOTOR_POWER/100));
}

void PumpOff(int pump){
  digitalWrite(pump, LOW);
}