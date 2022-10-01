#include <Adafruit_BMP085.h>
#include <Michom.h>
#include <DHT.h>
#include <MSInfoo.h>

#define DHTPIN 14     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11

/////////настройки//////////////

const char* id = "sborinfo_tv";
const char* type = "msinfoo";
/////////настройки//////////////

Michome michome(id, type, host, host1);
MSInfoo msi(&michome);

DHT dht(DHTPIN, DHTTYPE, 15);
Adafruit_BMP085 bmp;


void setup() {

  msi.SetInitMSInfooHandler([&](){
    dht.begin();
    if (!bmp.begin()) {
      #ifndef NoSerial
      Serial.println("Could not find a valid BMPx85 sensor, check wiring!");
      #endif
      return false;
    }
    return true;    
  });
  
  msi.SetResetMSInfooHandler([&](){
    return true;    
  });  

  msi.SetMSInfooHandler([&](){
    MSInfooData data;
    data.temp1 = dht.readTemperature();
    data.temp2 = bmp.readTemperature();
    data.humm = dht.readHumidity();
    data.alt = bmp.readAltitude();
    data.press = bmp.readPressure();
    
    return data;    
  });  

  msi.SetMSInfooInfoHandler([&](){
    return "Используются: BMP185 и DHT11";    
  });  

  server1.on("/refresh", [](){ 
      server1.send(200, "text/html", "OK");
      SendData();
  });  

    michome.SetRefreshData([](){terms.SendData();});

   
  michome.init(false);  
  
  SendData();
}
void loop() {
  michome.running();

  if(michome.GetSettingRead()){
    rtos.ChangeTime(michome.GetSetting("update").toInt());
  }

  if (rtos.IsTick()) {
    SendData();
  }
}

void SendData(){
  michome.SendData(michome.ParseJson(String(type), String(bmp.readPressure()/133.332)+";"+String(bmp.readTemperature())+";"+String(bmp.readAltitude())+";"+String(dht.readTemperature())+";"+String(dht.readHumidity())));
}
