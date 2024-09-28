#ifndef Michom_h
#define Michom_h
#include "config.h"

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

/*#ifdef BaseConfigH
	#pragma message "Default config.h incuded"
#endif*/
#ifdef NoCompilation
	#error "This configuration is not compilation. Please, check config.h files"
#endif

#define ToCharptr(str) (const_cast<char *>(str.c_str()))
#define ToCharArray(str) ((const char *)str.c_str())
#define GetHost ((String)MainConfig.Geteway + MichomePHPPath)
#define IsStr(str, str2) ((str.indexOf(str2) != -1))
#define AddErrorChucked(m, v) (m->AddError(F(__FILE__), v, F("Client chucked error")))

#if defined(ADCV)
	#define InitADC ADC_MODE(ADC_VCC)
#else
	#define InitADC
#endif

#define CreateMichome InitADC

#define FVer 2.26 //17-07-2024
#define DVer "17-07-2024"

#define BuiltLED 2
#define CountOptionFirmware 10
#define CountWIFI 3

#define GatewayResponceTimeout 10000
#define SizeSendBuffer 45
#define MAX_SENDDATA_LENGTH 150

#define WaitConnectWIFI 10000
#define PasswordAPWIFi "a12345678"
#define LocalIPAP "192.168.4.1"

#define DNS_PORT 53
#define MichomePHPPath "/michome/getpost.php"

#define WIFIMode WIFI_AP_STA //WIFI_AP_STA
#define DefaultWIFIStandart WIFI_PHY_MODE_11G //WIFI_PHY_MODE_11B WIFI_PHY_MODE_11N
//#define SerialSpeed 921600
#define SerialSpeed 115200
#define SAFEMODERELOADTIMEOUT 300000

#define UTC3 10800 //Utc+3
#define NTPServer "pool.ntp.org"
#define NTPServerMaxLenght 50

#define ARDUINOJSON_DECODE_UNICODE 1

#include "GetewayData.h"
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval
#include <math.h>
#include <ModuleTypes.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#ifdef FlashOTA
	#include <ArduinoOTA.h>
#endif
#include <ssdp.h>
#include <ESP8266LLMNR.h>
#include <DNSServer.h>
#include <Hash.h>
#include <LinkedList.h>
#ifndef NoFS  
    #include <FSLoging.h>
    #include <FSFiles.h>
#endif
#include <ArduinoJson.h>
#include <RTOS.h>
#include <Module.h>
#ifdef EnableExternalUnits
	#include <ExternalUnits.h>
#endif
#include <Logger.h>
#include <NTPClient.h>
#include <RKeyboard.h>
#include <Telnet.h>
#include <MichomUDP.h>
#include <PinWatcher.h>

const char JSONSendData[] PROGMEM = "{\
\"ip\":\"%s\",\
\"firmware\":\"%f\",\
\"rssi\":\"%d\",\
\"secretkey\":\"10cdb4e914f7d9d4f5d9cd9aa8da13d834ae6364\",\
\"secret\":\"MICHoMeMoDuLeORIGINALFIRMWARE\",\
\"mac\":\"%s\",\
\"type\":\"%s\",\
\"data\":%s\}\0";

const char HTTPHeader[] PROGMEM = "POST %s HTTP/1.1\r\n\
Host: %s\r\n\
Module-Serial: %d\r\n\
Module-MAC: %s\r\n\
Module-Type: %s\r\n\
Module-IP: %s\r\n\
Module-Room: %s\r\n\
Module-Network: %s\r\n\
Module-Time: %d\r\n\
Module-RealTime: %d\r\n\
Module-ID: %s\r\n\
User-Agent: MichomeModule_ESP8266\r\n\
Content-Type: application/json\r\n\
Accept: application/json\r\n\
Cache-Control: no-cache\r\n\
Content-Length: %d\r\n\
Connection: close\r\n\r\n\0";

/* #ifdef ESP8266
extern "C" {
#include "user_interface.h"
}
#endif
 */
typedef enum OptionsFirmware{LightModule, TimerLightModules, UDPTrigger, WST28, Infrometers, Termometers, MSInfoos, MeteoStations, OLEDModules, PowerMonito};

typedef enum WIFISignal{NoSignal, LowSignal, MidSSignal, MidHSignal, HighSignal};

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
	WiFiPhyMode_t WIFIType; //Стандарт сети WiFi
	char IDModule[WL_SSID_MAX_LENGTH]; //ID модуля
	char MNetwork[WL_SSID_MAX_LENGTH]; //Имя сети Michome
	char MRoom[WL_SSID_MAX_LENGTH]; //Имя комнаты в сети
};

typedef struct sendBufferChunk //Элемент буфера на отправку
{
    unsigned long unixTime; //Значение в unix времени
    unsigned long mUptime; //Аптайм модуля в секундах
    int Timeout; //Таймаут подключения к шлюзу
	char Type[WL_SSID_MAX_LENGTH]; //Тип данных на отправку
	char Data[MAX_SENDDATA_LENGTH]; //Значение данных на отправку
};

typedef struct michomeError //Структура ошибок модуля
{
    char file[WL_SSID_MAX_LENGTH]; //Файл с ошибкой
    char source[WL_SSID_MAX_LENGTH]; //Источник ошибки
	char message[100]; //Описание ошибки
	unsigned long moduleUptime; //Аптайм модуля
};

#include <WebPages.h>

typedef std::function<void(void)> THandlerFunction;

class Michome
{
        public:                
                //Объявление класса
                Michome(){VoidRefreshData = [&](){DefaultRefresh();}; SetOptionFirmware(UDPTrigger, true);};
                //Объявление класса
                Michome(const char* _ssid, const char* _password, char* _id, const char* _type, const char* _host1, const double FirmwareVersion);
                #ifndef NoFS
                //Объявление класса
                    Michome(char* _id, const char* _type, const double FirmwareVersion);
                #endif
                //Отправить GET запрос
                String SendDataGET(String gateway, String host, int Port);
                //Отправить POST запрос
				String SendDataPOST(const char* gateway, String host, int Port, String Data);
				String SendToGateway(String data){
					return SendDataGET(data, GetGatewayHost(), GetGatewayPort());
				}				
				
				//Помещает в очередь отправку данных на шлюз
				bool SendGateway(sendBufferChunk Chunk); //Непосредственное указание чанка
				bool SendGateway(String type, String Data); //Данные с указанием типа данных и самих данных
				bool SendGateway(); //Данные по умолчанию
				
                //Разделение строки по разделителю
                String Split(String data, char separator, int index);
				//Получить все настройки
                String GetSetting();
                //Получить настройку с именем
                String GetSetting(String name);
                //Получить числовую настройку с именем
                int GetSettingToInt(String name){return GetSettingToInt(name, DefaultSettingInt);}
				int GetSettingToInt(String name, int defaults);
				void SetSettings(String strSett);
				//
				//THandlerFunctionString GetDataFunction;
                //Если Файловая система разрешена
                #ifndef NoFS
                    //Получить настройки WIFI
                    WIFIConfig ReadWIFIConfig();
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
                void init(bool sendGateway);
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
				//Возвращает ссылку на объект доп оборудования модуля
				#ifdef EnableExternalUnits
					ExternalUnits& GetExternalUnits();
				#endif               
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
				void GetConfigerator();
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
                unsigned long GetRunningTime(){return millis();};
                //Моргать при обновении прошивки через OTA
                //bool IsBlinkOTA = true;
				//Включено ли использование шлюза
				bool IsUsingGateway(){
					return MainConfig.UseGeteway;
				}
				//Устанавливает метод для функции отправки данных
				void SetRefreshData(THandlerFunction voids){VoidRefreshData = voids;};
				//Производит сбор данных и их отправку на шлюз
				void Refresh(){VoidRefreshData();};
				void PortPrint(String text, bool newline = false){				
					if(newline){
						Serial.println(text);
						#ifdef UsePortPrint
							telnetmodule.println(text);
						#endif
					}
					else{
						Serial.print(text);
						#ifdef UsePortPrint
							telnetmodule.print(text);
						#endif
					}
				}
				void PortPrintln(String text){PortPrint(text, true);}
                //Если FS разрешена
                #ifndef NoFS
                    //Объект логов файловой системы
                    FSLoging FSLoger;
					void AddLogFile(String Log, bool printserial = true){
						FSLoger.AddLogFile(timeClient.getFormattedDateTime() + " - " + Log);
						if(printserial)
							Serial.println("LOG: " + timeClient.getFormattedDateTime() + " - " + Log);						
						#ifdef PrintLogToTelnet
							telnetmodule.println("LOG: " + timeClient.getFormattedDateTime() + " - " + Log);
						#endif
					}
                #endif
                #if defined(WriteDataToFile) && !defined(NoFS)
                    //Объект логов данных
                    FSFiles DataFile = FSFiles("/datalog.txt");
                #endif
				//Записывает ошибку в список
				void AddError(michomeError err){
					mErrors.add(err);
				}
				void AddError(const char *file, const char *script, String message){
					michomeError err;
					memcpy(err.file, file, WL_SSID_MAX_LENGTH);
					memcpy(err.source, script, WL_SSID_MAX_LENGTH);
					message.toCharArray(err.message, 100);
					err.moduleUptime = GetRunningTime();
					AddError(err);
				}
				void AddError(const char *file, const char *script, const char *message){
					michomeError err;
					memcpy(err.file, file, WL_SSID_MAX_LENGTH);
					memcpy(err.source, script, WL_SSID_MAX_LENGTH);
					memcpy(err.message, message, 100);
					err.moduleUptime = GetRunningTime();
					AddError(err);
				}
				void AddError(const __FlashStringHelper *file, const __FlashStringHelper *script, const char *message){
					michomeError err;
					memcpy_P(err.file, file, WL_SSID_MAX_LENGTH);
					memcpy_P(err.source, script, WL_SSID_MAX_LENGTH);
					memcpy(err.message, message, 100);
					err.moduleUptime = GetRunningTime();
					AddError(err);
				}
				void AddError(const __FlashStringHelper *file, const __FlashStringHelper *script, const __FlashStringHelper *message){
					michomeError err;
					memcpy_P(err.file, file, WL_SSID_MAX_LENGTH);
					memcpy_P(err.source, script, WL_SSID_MAX_LENGTH);
					memcpy_P(err.message, message, 100);
					err.moduleUptime = GetRunningTime();
					AddError(err);
				}
				void AddError(const char *file, const char *message){
					AddError(file, file, message);
				}
				//Возвращает уровень сигнала для значения RSSI
				WIFISignal GetSignal(int rssi){
					//NoSignal, LowSignal, MidSSignal, MidHSignal, HighSignal
					if(rssi >= -65) return HighSignal;
					else if(rssi >= -75) return MidHSignal;
					else if(rssi >= -85) return MidSSignal;
					else if(rssi >= -100) return LowSignal;
					else return NoSignal;
				}
				//Возвращает уровень сигнала для текущей WIFI сети
				WIFISignal GetSignal(){return GetSignal(WiFi.RSSI());}
				//Режим востановления
				bool IsSaveMode = false;
				//Устанавливает обработчик первой синхронизации времени
				void OnFirstTimeSync(TimeHandlerFunction voids){timeClient.OnFirstSyncTime(voids);};
				//Функции даты и времени
				int GetDay() const{return timeClient.getDay();}
				int GetHours() const{return timeClient.getHours();}
				int GetMinutes() const{return timeClient.getMinutes();}
				int GetSeconds() const{return timeClient.getSeconds();}
				String GetFormattedTime() const{return timeClient.getFormattedTime();}
				String GetFormattedDate() const{return timeClient.getFormattedDate();}
				String GetFormattedDateTime() const{return timeClient.getFormattedDateTime();}
				DateTime GetDateTime() const{return timeClient.getDateTime();}
        private:
			ESP8266WiFiMulti wifiMulti;
            WIFIConfig MainConfig;
			Telnet telnetmodule = Telnet(STANDARTTELNETPORT, id);
			MichomeUDP udpmodule = MichomeUDP(id, type);
			PinWatcher pinWatch = PinWatcher(&telnetmodule);
			#ifdef EnableExternalUnits
				ExternalUnits exUnit = ExternalUnits();
			#endif
            DNSServer dnsServer;
            char* id; const char* type;           
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
			void UpdateSendChunk(void); //Функция обработки и отправки чанков на шлюз
            bool SendData(sendBufferChunk Chunk); //Немедленно отправить данные на шлюз (внутренняя)		
            void JSONData(char *buffData, char *type, char *data); //Парсинг JSON данных
			void parseGatewayResponce(String resp, bool &isOK); //Парсит ответ шлюза
			bool OptionsFirmware[CountOptionFirmware]; //Модуль освещения - Модуль таймеров - UPD тригеры
			bool IsPreInt = false; //Пре инициализация
			RTOS NTPUpdate = RTOS(RTOS10M);//На обновление времени в DS32
			RTOS SaveModeReboot = RTOS(SAFEMODERELOADTIMEOUT);//Попытка перезапуска из сейф мода
			RTOS SendChunk = RTOS(RTOS1S);//Отправка чанков на шлюз
			double FirVersion = 1.0; //Версия ПО модуля
			THandlerFunction VoidRefreshData;
			LinkedList<sendBufferChunk> sendBuffer = LinkedList<sendBufferChunk>(); //Буфер на отправку данных
			LinkedList<michomeError> mErrors = LinkedList<michomeError>(); //Список с ошибками в модуле
			
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
			    if(strcmp(MainConfig.Geteway, ":") > 0) //Чекнуть
					httpPort = Split(MainConfig.Geteway, ':', 1).toInt();
				return httpPort;
			}
			String GetGatewayHost(){
			    return Split(MainConfig.Geteway, ':', 0);
			}
			void DefaultRefresh(){
				SendGateway();
			}
			time_t CurrentTimeCallback() {
				return ((time_t)timeClient.getEpochTime()); // UNIX timestamp
			}
			#ifdef ADCV
				ADC_MODE(ADC_VCC);
			#endif
};
#endif // #ifndef Michom_h