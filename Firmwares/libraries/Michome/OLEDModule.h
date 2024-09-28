#ifndef OLEDModule_h
#define OLEDModule_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define OLEDModuleTimeoutConnection 15000
#define PathToPrognoz "/michome/modules/oledmodule/getoled.php?type=json"
#define MaximumPages 10

#define MaxNamePage 40
#define MaxPageValue 80

#define MaxNameMenu 30
#define MaxMenuValue 80

#include <Michom.h>
#include <LinkedList.h>
#define ARDUINOJSON_DECODE_UNICODE 1
#include <ArduinoJson.h>

#include "lcd/symbols_oled.h"

typedef struct PageData
{	
    char NamePage[MaxNamePage]; //Имя страницы
    char PageValue[MaxPageValue]; //Значение страницы
};

typedef struct MenuData
{	
	uint8_t MenuID; //ID страницы
	uint8_t ParentID; //ID родительской страницы
	
    char NameMenu[MaxNamePage]; //Имя пункта меню
    char MenuValue[MaxPageValue]; //Значение страницы
};

typedef struct OLEDSetting //для хранения настроек информетра
{
    DateTime StartDisplay; //Время включения дисплея
    DateTime StopDisplay; //Время выключения дисплея
	uint8_t DisplayBR; //Яркость дисплея
	
	//HourlyData Hourly; //Интернет данные
    //String ServerTime; //Серверное время
    //bool LightEnable; //Управление подсветкой
};

typedef std::function<void(PageData page)> PDHandlerFunction;
typedef std::function<void()> UPDHandlerFunction;
typedef std::function<void(bool state, uint8_t br)> CHHandlerFunction;

class OLEDModule
{
        public:
                //Объявление класса
                OLEDModule(Michome *m);
                OLEDModule(){};
				void init(); //Инициалзация
				void running(); //Цикл работы
				void startUpdate(); //Принудительное обновление данных с сервера
				bool BacklightEnable = false; //Принудительное включение подстветки
				uint8_t CurrentPage = 0; //Возвращает или задает текущую страницу
				bool pause = false; // пауза обновления и отображения				
				bool isMenu = false; // Используется ли сейчас режим меню				
				bool IsAutoUpdate = true; //автоматически обновлять данные по таймеру
				bool GetEtherFail() {return EtherFail;}; //Возвращает есть ли ошибка соединения
				long GetAttempted() {return Attempted;}; //Возвращает попытку соеденения
				long GetReadData() {return IsReadData;}; //Возвращает были ли прочитаны данные
				void ChangePage(){
					CurrentPage++;
					if(CurrentPage >= maxpages) CurrentPage = 0;
					UpdatePageData(Pages[CurrentPage]);
					UpdateData();
					rtos1.Zero();
				}
				
				void AddMenu(MenuData d){Menus.add(d);}
				
				PDHandlerFunction UpdatePageData; //Отображение страницы
				UPDHandlerFunction UpdateData; //Обновление данных на дисплее
				CHHandlerFunction UpdateLight; //Изменение состояния подсветки
				
        private:
            Michome *gtw;
			RTOS rtos = RTOS(RTOS5M);//Запрос данных
			
			RTOS rtos1 = RTOS(4000);//Время переключения экранов
			
			RTOS DisplayUpdate = RTOS(RTOS15FPS);//Обновление дисплея
			RTOS _UpdateLight = RTOS(RTOS30FPS);//Включение подсветки
			
			bool EtherFail = false; //Ошибка интернета
			long Attempted = 0; //попытка соеденения						
			bool IsReadData = false; //Данные были прочитаны
			OLEDSetting Setting;

			PageData Pages[MaximumPages];
			uint8_t maxpages = 0;
			
			LinkedList<MenuData> Menus = LinkedList<MenuData>();

			void Save();
			void Load();
			void parse(String datareads);
			
			bool IsOnLight(){
				int StartMinutes = Setting.StartDisplay.ToMinutes();
				int StopMinutes = Setting.StopDisplay.ToMinutes();
				int CurrentMinutes = (*gtw).GetDateTime().ToMinutes();
				// 120  150  210
				// 120  10   210
				if(CurrentMinutes < StartMinutes && CurrentMinutes >= StopMinutes)
					return false;
				else if(CurrentMinutes >= StartMinutes && CurrentMinutes < StopMinutes)
					return true;
				else return false;
			}
		
			void ChangeLight(){
				if(BacklightEnable || IsOnLight())
					UpdateLight(true, Setting.DisplayBR);
				else UpdateLight(false, Setting.DisplayBR); //Выключаем свет
			}
            
};
#endif // #ifndef OLEDModule_h