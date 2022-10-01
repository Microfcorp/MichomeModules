#ifndef Michom_h
#define Michom_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define ToCharptr(str) (const_cast<char *>(str.c_str()))
#define ToCharArray(str) ((const char *)str.c_str())
#define GetHost ((String)MainConfig.Geteway + MichomePHPPath)
#define IsStr(str, str2) ((str.indexOf(str2) != -1))

#define FVer 2.12

#define BuiltLED 2
#define CountOptionFirmware 10
#define CountWIFI 3

#define WaitConnectWIFI 10000
#define PasswordAPWIFi "a12345678"

#define DNS_PORT 53
#define MichomePHPPath "/michome/getpost.php"

#define WIFIMode WIFI_AP_STA //WIFI_AP_STA

#define UTC3 10800 //Utc+3
#define NTPServer "pool.ntp.org"
#define NTPServerMaxLenght 50

#include "config.h"
#include "GetewayData.h"
#include <math.h>
#include <ModuleTypes.h>
/* extern "C" {
#include "user_interface.h"
} */
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266SSDP.h>
#include <ESP8266LLMNR.h>
#include <DNSServer.h>
#include <Logger.h>
#include <NTPClient.h>
#include <RTOS.h>
#include <RKeyboard.h>
#include <Hash.h>
#ifndef NoFS
    #include <FS.h>
	#include <LittleFS.h>
    #include <FSLoging.h>
    #include <FSFiles.h>
#endif
#include <WebPages.h>
#include <Telnet.h>
#include <MichomUDP.h>

typedef enum OptionsFirmware{LightModule, TimerLightModules, UDPTrigger, WST28, Infrometers, Termometers, MSInfoos};

typedef struct WIFINetwork //Настройки WIFI части
{
    char SSID[WL_SSID_MAX_LENGTH]; //SSID WIFI сети
    char Password[WL_WPA_KEY_MAX_LENGTH]; //Пароль WIFI сети
};

typedef struct WIFIConfig //Настройки WIFI части
{
    WIFINetwork WIFI[CountWIFI];
    char Geteway[WL_SSID_MAX_LENGTH]; //Адрес шлюза
    bool UseGeteway; //Использовать ли шлюз
};

typedef std::function<void(void)> THandlerFunction;

class Michome
{
        public:
                #ifdef ADCV
                    ADC_MODE(ADC_VCC);
                #endif
                //Объявление класса
                Michome(){VoidRefreshData = [&](){DefaultRefresh();}; SetOptionFirmware(UDPTrigger, true);};
                //Объявление класса
                Michome(const char* _ssid, const char* _password, const char* _id, const char* _type, const char* _host1, double FirmwareVersion);
                #ifndef NoFS
                //Объявление класса
                    Michome(const char* _id, const char* _type, double FirmwareVersion);
                #endif
                //Отправить GET запрос
                String SendDataGET(String gateway, String host, int Port);
                //Отправить POST запрос
				String SendDataPOST(const char* gateway, String host, int Port, String Data);
				String SendToGateway(String data){
					return SendDataGET(data, GetGatewayHost(), GetGatewayPort());
				}
                //Отправить данные на сервер
                void SendData(String Data);
                //Отправить стандартные данные на сервер
                void SendData();
                //Разделение строки по разделителю
                String Split(String data, char separator, int index);
                //Получить настройку с именем
                String GetSetting(String name);
                //Получить числовую настройку с именем
                int GetSettingToInt(String name){return GetSettingToInt(name, DefaultSettingInt);}
				int GetSettingToInt(String name, int defaults);
                //Получить все настройки
                String GetSetting();
				//
				//THandlerFunctionString GetDataFunction;
                //Если Файловая система разрешена
                #ifndef NoFS
                    //Получить настройки WIFI
                    WIFIConfig ReadWIFIConfig();
                    //Записать SSID и пароль
                    //void WriteSSIDAndPassword(String ssid, String password, String Geteway, bool UseGeteway);
                    //Записать SSID и пароль
                    //void WriteSSIDAndPassword(String txt);
					//Записать настройки WIFI
					void WriteWIFIConfig(WIFIConfig conf);
					//Записать текущие настройки
					void WriteWIFIConfig(){
						WriteWIFIConfig(MainConfig);
					}
                #endif
                //Получить класс логгера
                Logger GetLogger();
                //Инициализация модуля с отправкой данных
                void init(bool senddata);
                //Инициализация модуля
                void init();
				//
				void preInit(void);
                //Основной цикл
                void running(void);
                //Выполнение все критических операций
                void yieldM(void);
				//Выполнение стековых операций
                void yieldWarn(void);				
                //Моргнуть светодиодом на плате
                void StrobeBuildLed(byte timeout);
                //Моргнуть светодиодом на плате информацию об ошибки
                void StrobeBuildLedError(int counterror, int statusled);
                //Получить WEB сервер
                ESP8266WebServer& GetServer();
				//Получить Telnet сервер
                Telnet& GetTelnet();
				//Получить UDP сервер
                MichomeUDP& GetUDP();
                //Парсинг JSON данных
                String ParseJson(String type, String data); 
                String ParseJson(String data) {return ParseJson(GetModule(0), data);} 
                //Установить формат настроек
                void SetFormatSettings(int count);
                //Считать ли настройки
                bool GetSettingRead();
				//Проверить соединение с шлюзом
				bool CheckConnectToGateway();
				//Задает опцию прошивки
                void SetOptionFirmware(byte id, bool value){
					if(id < 0 || id > CountOptionFirmware) return;
					OptionsFirmware[id] = value;
				}
				//Получает опцию прошивки
                bool GetOptionFirmware(byte id){
					if(id < 0 || id > CountOptionFirmware) return false;
					return OptionsFirmware[id];
				}
				//Получить конфигуратор
				void GetConfigerator(){
					WebConfigurator(&GetServer(), (String)MainConfig.WIFI[0].SSID, (String)MainConfig.WIFI[0].Password,(String)MainConfig.WIFI[1].SSID, (String)MainConfig.WIFI[1].Password,(String)MainConfig.WIFI[2].SSID, (String)MainConfig.WIFI[2].Password, (String)MainConfig.Geteway, MainConfig.UseGeteway);
				}
				//Найден ли данные ssid в списке
				bool IsFoundSSID(String ssid){
					for(int i = 0; i < CountWIFI; i++){
						if(ssid == (String)MainConfig.WIFI[i].SSID) return true;
					}
					return false;
				}
                //Получены ли настройки
                bool IsSettingRead;
                //Таймаут сервера
                int TimeoutConnection = 5000;
                //Стандартное значение при поиске настройки
                int DefaultSettingInt = 1;
                //Отконфигурирован ди модуль
                bool IsConfigured = false;
                //Получает информацию о модуле по коду num
                String GetModule(byte num);
                //Время работы модуля
                long GetRunningTime(){return millis();};
                //Моргать при обновении прошивки через OTA
                bool IsBlinkOTA = true;
				//Включено ли использование шлюза
				bool IsUsingGateway(){
					return MainConfig.UseGeteway;
				}
				//Устанавливает метод для функции отправки данных
				void SetRefreshData(THandlerFunction voids){VoidRefreshData = voids;};
				//Производит сбор данных и их отправку на шлюз
				void Refresh(){VoidRefreshData();};
				void PortPrint(String text, bool newline = false){
					#if defined(UsePortPrint)
						if(newline){
							Serial.println(text);
							telnetmodule.println(text);
						}
						else{
							Serial.print(text);
							telnetmodule.print(text);
						}
					#else
						if(newline){
							Serial.println(text);
						}
						else{
							Serial.print(text);
						}
					#endif
				}
				void PortPrintln(String text){PortPrint(text, true);}
                //Если FS разрешена
                #ifndef NoFS
                    //Объект логов файловой системы
                    FSLoging FSLoger;
					void AddLogFile(String Log, bool printserial = true){
						if(printserial)
							Serial.println("LOG: " + timeClient.getFormattedDateTime() + " - " + Log);
						FSLoger.AddLogFile(timeClient.getFormattedDateTime() + " - " + Log);
						#ifdef PrintLogToTelnet
							telnetmodule.println("LOG: " + Log);
						#endif
					}
                #endif
                #if defined(WriteDataToFile) && !defined(NoFS)
                    //Объект логов данных
                    FSFiles DataFile = FSFiles("/datalog.txt");
                #endif
				//Режим востановления
				bool IsSaveMode = false;
				//Функции даты и времени
				int GetDay() const{return timeClient.getDay();}
				int GetHours() const{return timeClient.getHours();}
				int GetMinutes() const{return timeClient.getMinutes();}
				int GetSeconds() const{return timeClient.getSeconds();}
				String GetFormattedTime() const{return timeClient.getFormattedTime();}
				String GetFormattedDate() const{return timeClient.getFormattedDate();}
				String GetFormattedDateTime() const{return timeClient.getFormattedDateTime();}
        private:
			ESP8266WiFiMulti wifiMulti;
            WIFIConfig MainConfig;
			Telnet telnetmodule = Telnet(23, id);
			MichomeUDP udpmodule = MichomeUDP(id, type);
            DNSServer dnsServer;
            const char* id; const char* type; const char* host1;           
            MDNSResponder mdns;
            void _init(void);
            String settings;  
            int countsetting = 1;
            void CreateAP();
            bool IsNeedReadConfig = false;
            long wifi_check;
			//Установить режим работы WIFI
			void ChangeWiFiMode(void);
			void GetMainWeb(void);
			bool OptionsFirmware[CountOptionFirmware]; //Модуль освещения - Модуль таймеров - UPD тригеры
			bool IsPreInt = false; //Пре инициализация
			RTOS NTPUpdate = RTOS(240000);//На обновление времени
			RTOS SaveModeReboot = RTOS(300000);//Попытка перезапуска из сейф мода
			double FirVersion = 1.0; //Версия ПО модуля
			THandlerFunction VoidRefreshData;
			
			WiFiUDP ntpUDP;
			FSFiles TimeSettings = FSFiles("/tset.txt");
            NTPClient timeClient = NTPClient(ntpUDP, NTPServer, UTC3);
            char* _NTPServer = NTPServer;
            int _utcoffset = UTC3;
			void SetNTP(){
                timeClient.setPoolServerName(_NTPServer);
                timeClient.setTimeOffset(_utcoffset);
                timeClient.forceUpdate();
            }
			void LoadNTP(){
                String rs = TimeSettings.ReadFile();
				if(TimeSettings.ErrorOpen){
					TimeSettings.WriteFile((String)NTPServer + (String)";" + (String)UTC3);
				}
                String ntps = Split(rs, ';', 0);
                int utctime = Split(rs, ';', 1).toInt();
                if(ntps == "") ntps = NTPServer;
                if(utctime == 0) utctime = UTC3;
                ntps.toCharArray(_NTPServer, NTPServerMaxLenght);
                _utcoffset = utctime;
                SetNTP();
            }
			int GetGatewayPort(){
			    int httpPort = 80;
			    if(IsStr(String(MainConfig.Geteway), ":"))
					httpPort = Split(MainConfig.Geteway, ':', 1).toInt();
				return httpPort;
			}
			String GetGatewayHost(){
			    return Split(MainConfig.Geteway, ':', 0);
			}
			void DefaultRefresh(){
				SendData();
			}
};
#endif // #ifndef Michom_h