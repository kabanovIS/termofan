#define BLYNK_PRINT Serial
#define TERMO_PIN D7
#define RELE_POWER D5 //включение и отключение вентилятора. 0 включено (по умолчанию) 1 выключено
#define RELE_MODE D6  //переключение режима. 0 максимальные обороты 1 средние обороты
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
//Sensors
#include <OneWire.h>
#include <DallasTemperature.h>
//RTC Time
#include <TimeLib.h>
#include <WidgetRTC.h>


DeviceAddress Term01 = {0x28, 0x61, 0x64, 0x11, 0x8C, 0x6E, 0x5F, 0x96};
DeviceAddress Term02 = {0x28, 0x61, 0x64, 0x12, 0x3F, 0xE3, 0x93, 0x89};
DeviceAddress Term03 = {0x28, 0x61, 0x64, 0x11, 0x94, 0x68, 0x2A, 0x94};   
DeviceAddress Term04 = {0x28, 0x61, 0x64, 0x11, 0x95, 0x9E, 0xFD, 0xD8};
DeviceAddress Term05 = {0x28, 0x61, 0x64, 0x11, 0x95, 0x9F, 0x43, 0x31};  
DeviceAddress TermMotor = {0x28, 0xFF, 0xA1, 0x33, 0x91, 0x16, 0x04, 0xD2};
DeviceAddress TermOUT = {0x28, 0xFF, 0xBA, 0xFC, 0x01, 0x17, 0x04, 0x91};
//new sensors




OneWire oneWire(TERMO_PIN);
DallasTemperature sensors(&oneWire);
char auth[] = "c94dd10289fb4c449af5973cda8b784c";
char ssid[] = "www";
char pass[] = "11111111";
char host[] = "ap.kashkan.org";
int port = 9998;

float temp0; //Средняя температура
float MIN_TEMP = 18;
float MAX_TEMP = 40; //температура переключения на максимум
int DELTA_TEMP = 10;  //разница для возврата 
int  count_sensors = 4;
float sum_temp = 0;
int TEMP_LOW = 28;
int TEMP_HIGH = 38;
float avg_temper;
int fanmode = 1; //средние обороты
bool automode = true; 
float temp01;
int uptime;
bool wifilost_flag =  false;
int wifilost_timer_start;
int wifilost_timer_max = 60; // 60 sec timeout for reset if WiFi connection lost
int days, hours, minutes, seconds;
String currentTime;
String currentDate;

BlynkTimer timer;
WidgetRTC rtc;

bool connectBlynk(){
  _blynkWifiClient.stop();
  return _blynkWifiClient.connect(host, port);
}


void setup()
{
pinMode(RELE_POWER,OUTPUT);
pinMode(RELE_MODE,OUTPUT);
digitalWrite(RELE_POWER, HIGH);
digitalWrite(RELE_MODE, HIGH);
Serial.begin(115200);
Blynk.begin(auth, ssid, pass, host, port);
sensors.begin();
  
  timer.setInterval(1000L, sendUptime);
 
  ESP.wdtDisable();
}



void sendUptime(){

  uptime = millis() / 1000;
  

  
  if (Blynk.connected()){
  Blynk.virtualWrite(V99, uptime);
 
  }
  
}




int SetMode (float avg_temp ,float out_temp ,int fanmode )
{      Serial.println("\nПараматры средней температруты ");
       Serial.println("\nСредняя температура ");
           Serial.println(avg_temp);
       Serial.println("\nТемпература улицы ");
           Serial.println(out_temp);    
       Serial.print("\nРежим ");
           Serial.println(fanmode);  
       Serial.println("\nДельта ");
           Serial.println(DELTA_TEMP); 
       Serial.println("\nMIN ");
           Serial.println(MIN_TEMP); 
       Serial.println("\nMAX ");
           Serial.println(MAX_TEMP);  
              
 if (avg_temp<MIN_TEMP and fanmode != 0) 
   {
     Serial.println("!!! 0 !!!!");  
    Blynk.virtualWrite (V20,"Температура меньше минимальной. Выключаем!");
    return 0; 
   }
 else if (avg_temp > MAX_TEMP and fanmode !=2) 
  { Serial.println("!!! 1 !!!!"); 
   Blynk.virtualWrite (V20,"Температура больше максимальной. Включаем на максимум!");
   return 2;   
  }
 else if (avg_temp > TEMP_HIGH and fanmode !=2) 
  {  Serial.println("!!! 2 !!!!"); 
    Blynk.virtualWrite (V20,"Температура включения полных оборотов!");
    return 2; 
  }
 else if (fanmode == 2 and avg_temp < out_temp + DELTA_TEMP) 
 {  Serial.println("!!! 3 !!!!"); 
  Blynk.virtualWrite (V20,"Температура отключения полных оборотов!");
  return 1; // убавляем обороты
 }
 else if (fanmode == 1 and avg_temp < out_temp + DELTA_TEMP) 
 {  Serial.println("!!! 4 !!!!"); 
  Blynk.virtualWrite (V20,"Средние обороты согнали температуру. Выключаем!");
  return 0;   // вырубаем
 }
 else if (fanmode == 0 and avg_temp > TEMP_LOW ) 
  {
     Blynk.virtualWrite (V20,"Температура больше включения средних оборотов. включаем");
    return 1;
  }
 else return fanmode;
 }

BLYNK_CONNECTED()
{Blynk.syncVirtual(V5);
 Blynk.syncVirtual(V6);
 Blynk.syncVirtual(V15);
 Blynk.syncVirtual(V16);
 Blynk.syncVirtual(V40);
 Blynk.syncVirtual(V41);
 Blynk.syncVirtual(V30);
// Blynk.syncVirtual(V31); 
 Blynk.syncVirtual(V32);
 Blynk.syncVirtual(V33); 
 Blynk.syncVirtual(V34);
 rtc.begin();
 }

 BLYNK_WRITE(V30){
  MIN_TEMP = param.asInt();
    Blynk.virtualWrite (V20,"Минимальная темпертура: "); 
    Blynk.virtualWrite (V20,MIN_TEMP); 
  }
  
//  BLYNK_WRITE(V31){
//  MAX_TEMP = param.asInt();
//    Blynk.virtualWrite (V20,"Максимальная темпертура: "); 
//    Blynk.virtualWrite (V20,MAX_TEMP); 
//  }
 BLYNK_WRITE(V32){
    TEMP_HIGH = param.asInt();
    Blynk.virtualWrite (V20,"Включение максимум: "); 
    Blynk.virtualWrite (V20,TEMP_HIGH); 
  }
 BLYNK_WRITE(V33){
    DELTA_TEMP = param.asInt();
    Blynk.virtualWrite (V20,"Дельта : "); 
    Blynk.virtualWrite (V20,DELTA_TEMP); 
  }
  BLYNK_WRITE(V34){
    TEMP_LOW = param.asInt();
    Blynk.virtualWrite (V20,"Включение средние: "); 
    Blynk.virtualWrite (V20,TEMP_LOW); 
  }
// BLYNK_WRITE(V35){
//    DELTA_TEMP_LOW = param.asInt();
//    Blynk.virtualWrite (V20,"Дельта с улицей средние: "); 
//    Blynk.virtualWrite (V20,DELTA_TEMP_LOW); 
//  } 

BLYNK_WRITE(V5)
{
  if(automode == false)
  {
if(param.asInt()==1)
{
   digitalWrite(RELE_POWER, LOW);
   Blynk.virtualWrite(V15,0);
   Blynk.virtualWrite (V20,"Питание отключено");
}
else
{
   digitalWrite(RELE_POWER, HIGH);
   Blynk.virtualWrite(V15,255);
   Blynk.virtualWrite (V20,"Питание включено");
}
  }
}

BLYNK_WRITE(V6)
  {
    if(automode == false)
    {
      if(param.asInt()==1)
      {
   digitalWrite(RELE_MODE, LOW);
   Blynk.virtualWrite(V16, 0);
   Blynk.virtualWrite (V20,"Режим ограниченных оборотов");
      }
      else
      {
   digitalWrite(RELE_MODE, HIGH);
   Blynk.virtualWrite(V16, 255);
   Blynk.virtualWrite(V20,"Режим полных оборотов");
      }
    }
  }

BLYNK_WRITE(V40) //авторежим
{ 
  
if(param.asInt()==1)
   { 
   automode =true;
   Blynk.virtualWrite(V41, 254);
   Blynk.virtualWrite (V20,"Автоматический режим включен");
   } else
   {
   automode =false;
   Blynk.virtualWrite(V41, 0);
   Blynk.virtualWrite (V20,"Автоматический режим выключен");
    }
  
}


float avg_temp() {

count_sensors = 4;
sum_temp =0;
sensors.requestTemperatures();

temp01 = sensors.getTempC(Term01);
Serial.println (temp01);
Blynk.virtualWrite(V1,temp01);
if (temp01 < -40 ||temp01 > 60 ) 
  {
//Blynk.virtualWrite(V20,"!!датчик 1 неисправен!!");
count_sensors --;
  } 
else 
  {
    sum_temp = sum_temp + temp01;   
  };

float temp02 = sensors.getTempC(Term02);
Serial.println (temp02);
Blynk.virtualWrite(V2,temp02);

if (temp02 < -40 ||temp02 > 60 ) 
  {
 Blynk.virtualWrite(V20,"!!датчик 2 неисправен!!");
count_sensors --;
  } 
else 
  {
    sum_temp = sum_temp + temp02;   
  };

float temp03 = sensors.getTempC(Term03);
Serial.println (temp03);
Blynk.virtualWrite(V3,temp03);
if (temp03 < -40 ||temp03 > 60 ) 
  {
 Blynk.virtualWrite(V20,"!!датчик 3 неисправен!!");
count_sensors --;
  } 
else 
  {
    sum_temp = sum_temp + temp03;   
  };
float temp04 = sensors.getTempC(Term04);
Serial.println (temp04);

Blynk.virtualWrite(V4,temp04);

if (temp04 < -40 ||temp04 > 60 ) 
  {
 Blynk.virtualWrite(V20,"!!датчик 4 неисправен!!");
count_sensors --;
  } 
else 
  {
    sum_temp = sum_temp + temp04;   
  };


 float temp05 = sensors.getTempC(Term05);
 
Blynk.virtualWrite(V7,temp05);
Serial.println (temp05);
if (temp05 < -40 ||temp05 > 60 ) 
  {
 Blynk.virtualWrite(V20,"!!датчик чердака неисправен!!");

  } 

 float tempmotor = sensors.getTempC(TermMotor);
 
Blynk.virtualWrite(V8,tempmotor);
Serial.println (tempmotor);
if (tempmotor < -40 ||tempmotor > 60 ) 
  {
 Blynk.virtualWrite(V20,"!!датчик мотора неисправен!!");

  } 
 
Serial.println ("Количество датчиков ");
Serial.println (count_sensors);
Serial.println ("Сумма температур ");
Serial.println (sum_temp);
if (count_sensors <=0) {return 0;} else {return sum_temp/count_sensors;};
}

void loop()
{

  
  if (WiFi.status() == WL_CONNECTED){
    wifilost_flag = false;

      
      if (Blynk.connected() && _blynkWifiClient.connected()){
        Blynk.run();
      }
      else{
        Serial.print("\n\rReconnecting to blynk.. ");
        Serial.print(Blynk.connected());
       if (!_blynkWifiClient.connected()){
           connectBlynk();
           return;
        }
        
        //FIXME: add exit after n-unsuccesfull tries.
        Blynk.connect(4000);
        Serial.print(Blynk.connected());
      }
    }

 




  
sensors.requestTemperatures();
temp0 = sensors.getTempC(TermOUT); 
Blynk.virtualWrite(V0,temp0);
avg_temper = avg_temp(); 
Serial.println ("Средняя температура ");
Serial.println (avg_temper);
Blynk.virtualWrite(V10,avg_temper);
if (automode == true ) 

{
Serial.println ("Автоматический режим");  

fanmode = SetMode (avg_temper,temp0,fanmode);
Serial.println ("Выбранный режим ");
Serial.println (fanmode);
switch (fanmode) {
  case 0:
    digitalWrite(RELE_POWER, LOW); //Выключить
    digitalWrite(RELE_MODE, LOW); //Средние обороты
    Blynk.virtualWrite(V15,0);
    Blynk.virtualWrite(V16,0);
    break;
  case 1:
    digitalWrite(RELE_POWER, HIGH); //Включить
    digitalWrite(RELE_MODE, LOW); //Средние обороты
    Blynk.virtualWrite(V15,1);
    Blynk.virtualWrite(V16,0);
    break;
  case 2:
    digitalWrite(RELE_POWER, HIGH); //Включить
    digitalWrite(RELE_MODE, HIGH); //Средние обороты
    Blynk.virtualWrite(V15,1);
    Blynk.virtualWrite(V16,1);
    break;
  break;
}
}
delay(1000);
  if (WiFi.status() != WL_CONNECTED){  
     Serial.print("\n\rWiFi connection lost");
    if (!wifilost_flag){
      wifilost_timer_start = uptime;
      wifilost_flag = true;
    }
    if (((uptime -  wifilost_timer_start) > wifilost_timer_max) && wifilost_flag){
      Serial.print("\n\rWiFi connection lost, restarting..");
      wifilost_flag = false;
      ESP.restart();
    }
  }

  timer.run();  
  ESP.wdtFeed();


}
