#include <Adafruit_BMP085.h>
#include <Michom.h>
#include <DHT.h>
#include <MSInfoo.h>

#define DHTPIN 14     // what digital pin we're connected to
#define DHTTYPE DHT11   // DHT 11

/////////настройки//////////////

char* id = "sborinfo_tv";
const char* type = Msinfoo;
double VersionTermometr = 1.9;
/////////настройки//////////////

Michome michome(id, type, VersionTermometr);
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

  michome.SetRefreshData([](){msi.SendData();});
 
  michome.init(false);

  if(michome.IsSaveMode) return;
  msi.init();
  
  michome.Refresh();  
}
void loop() {
  michome.running();

  if(michome.IsSaveMode) return;
  msi.running();
  
  if(michome.GetSettingRead()){
    msi.ChangeTime(michome.GetSetting("update").toInt());
  }
}
