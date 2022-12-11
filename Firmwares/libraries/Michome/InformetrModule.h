#ifndef InformetrModules_h
#define InformetrModules_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define InformetrModuleTimeoutConnection 15000
#define DaysMaximum 6
#define HoursMaximum 24
#define MaxSelH 4
#define PathToPrognoz "/michome/api/getprognoz.php?type=json"

#define IsLCDI2C

#define ChangeLightForSettings //Управление подсветкой по настройкам или по данным со шлюза

#include <Michom.h>
#include <LinkedList.h>
#include <ArduinoJson.h>

#ifdef IsLCDI2C
	#include <LiquidCrystal_I2C.h>
	//#define IsLCD1602I2C
	#include <Wire.h>
	#include <lcd/symbols_inf.h>
	#include <lcd/symbols_russian.h>
#endif

typedef enum SymbolsBank { BIndoor, BOutDoor, Day, BHourly, Watcher };
typedef enum WorkingModes { Weather, Watch };
typedef struct LCDData
{	
    char DayTemp[6]; //Дневная температура
    char NightTemp[6]; //Ночная температура
    char Wind[6]; //Скорость ветра
	char Press[6]; //Давление
	char ToDate[16]; //На дату
	long UNIXDay; //На дату
	byte IconDay; //Иконка
	byte IconNight; //Иконка
};
typedef struct LCDDataPrint //для вывода на дисплей
{
    SymbolsBank Symbols; //Банк с иконками
    //byte IconsLine[2]; //Иконки по строкам
    LCDData Data; //Погодгые данные
};

typedef struct IndoorData
{		
    LCDData Data;
};

typedef struct OutdoorData
{
    LCDData Data[DaysMaximum];
};

typedef struct HourlyDataPoint
{
    char Temp[5]; //температура
	char Time[6]; //На время
	byte Icon; //Иконка
};
typedef struct HourlyData
{
	byte Count;
    HourlyDataPoint Data[HoursMaximum];
};

typedef struct ServerData //для хранения с сервера в ram
{
    IndoorData Indoor; //Домашние данные
    OutdoorData Outdoor; //Интернет данные
	HourlyData Hourly; //Интернет данные
    String ServerTime; //Серверное время
    bool LightEnable; //Управление подсветкой
};

typedef struct InformetrSetting //для хранения настроек информетра
{
    DateTime StartDisplay; //Домашние данные
    DateTime StopDisplay; //Интернет данные
	//HourlyData Hourly; //Интернет данные
    //String ServerTime; //Серверное время
    //bool LightEnable; //Управление подсветкой
};

class InformetrModule
{
        public:
                //Объявление класса
                InformetrModule(Michome *m);
                InformetrModule(){};
				//#ifdef IsLCDI2C
					void SetDisplaySize(byte adress, byte x, byte y);					
				//#endif
				void init();
				void running();
				void startUpdate();
				void printLCD(LCDDataPrint dt);
				bool BacklightEnable = false; //Включение подстветки
				bool PokazType = true; //тип экрана (прогноз (сменный)) или домашний (статичный)
				void InversePokazType(){if(!IsReadData || WorkMode == Watch) return; PokazType =! PokazType; if(!PokazType) i1(); else i2();}
				bool pause = false; // пауза обновления и отображения				
				bool IsAutoUpdate = true; //автоматически обновлять данные по таймеру
				void InverseHourlyDay(){if(WorkMode == Watch) return; IsHourly =! IsHourly; if(IsHourly){ LoadHourlyPrognoz(); i1();} else i2();} //Включает почасовой прогноз
                void InverseWorkMode(){if(WorkMode == Weather) {WorkMode = Watch; rtos1.Stop(); rtos2.Stop(); rtos3.Start();} else {WorkMode = Weather; rtos3.Stop(); rtos2.Start();} i1();}
				
        private:
            Michome *gtw;
			RTOS rtos = RTOS(600000);//Запрос данных
			
			RTOS rtos1 = RTOS(1200);//Время переключения экранов
			RTOS rtos2 = RTOS(5000);//Врея показа дня
			RTOS rtos3 = RTOS(1000);//Врея показа дня
			
			RTOS LightOff = RTOS(600000);//Время до отключения подсветки
			RTOS UpdateLight = RTOS(30);//Включение подсветки
			long pogr = 1800; //Отсчет до времени показа дня
			bool EtherFail = false; //Ошибка интернета
			long Attempted = 0; //попытка соеденения						
			bool IsRunLight = false; //Принудительное включение подсветки
			bool IsReadData = false; //Данные были прочитаны
			bool IsHourly = false; //Почасовой прогноз
			WorkingModes WorkMode = (WorkingModes)Weather; //Режим работы
			byte displayH, displayW;
			
			int selhours[MaxSelH];
			
			InformetrSetting Setting;
			ServerData Server;
			void Save();
			void Load();
			void parse(String datareads);
			void parseH(String datareads);
			void printpause();
			void i1();
			void i2();
			void i3();
			void nodat();
			void LoadHourlyPrognoz();
			void PrintETError();
			bool st = true;
			byte day = 0;
			byte maxday = 0;
			void plusday() {
				day++;
				if (day > maxday-1) 
					day = 0;
			}
			int getminusday() {
				int d = day;
				d--;
				if (d < 0) 
					return maxday-1;
				return d;
			}
			
			void PrintTextDayFromDate(String todate){
				int year = (*gtw).Split(todate, '-', 0).toInt();
				int mounth = (*gtw).Split(todate, '-', 1).toInt();
				int day = (*gtw).Split(todate, '-', 2).toInt();
				DateTime CD = (*gtw).GetDateTime();
				
				if(year == CD.Year && mounth == CD.Mounth && day == CD.Day){
					lcd.createChar(0, rus_G);
					lcd.createChar(1, rus_D);
					lcd.createChar(2, rus_YA);
					lcd.print("Ce");
					lcd.write(0);
					lcd.write('O');
					lcd.write(1);
					lcd.write('H');
					lcd.write(2);
				}
				else if((year == CD.Year && mounth == CD.Mounth && day == CD.Day + 1) || (year == CD.Year && mounth == CD.Mounth && day == 1 && (CD.Day == 30 || CD.Day == 31))){
					lcd.createChar(0, rus_Z);
					lcd.write(0);
					lcd.print("ABTPA");				
				}
			}
			
			void GenerateSelHours(){
				if(Server.Hourly.Count <= MaxSelH)
					for(int i = 0; i < MaxSelH; i++)
						selhours[i] = i;
					
				else if(Server.Hourly.Count > MaxSelH){
					int pr = ((int)Server.Hourly.Count) / MaxSelH;
					for(int i = 0; i < MaxSelH; i++){
						//if(i == 0) continue;
						selhours[i] = (i * pr);
					}
				}						
			}
			#if defined(ChangeLightForSettings)
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
			#endif
			
			//#ifdef IsLCDI2C
				void ChangeLight(){
					#if defined(ChangeLightForSettings)
					if(BacklightEnable || IsRunLight || IsOnLight())
					#else
					if(BacklightEnable || IsRunLight || Server.LightEnable)
					#endif
						lcd.backlight();
					else lcd.noBacklight(); //Выключаем свет
				}
				void ToHomes(){
				  lcd.createChar(0, homes);
				  lcd.createChar(1, watchh);
				  lcd.createChar(2, groza);
				  lcd.createChar(3, soln);
				  lcd.createChar(4, sneg);
				  lcd.createChar(5, gradus);
				}

				void ToPrognoz(){
				  lcd.createChar(0, Dozd);
				  lcd.createChar(1, oblazn);
				  lcd.createChar(2, groza);
				  lcd.createChar(3, soln);
				  lcd.createChar(4, sneg);
				  lcd.createChar(5, gradus);
				  lcd.createChar(6, LittleDozd);
				  lcd.createChar(7, clockI); //WINDSTRELA
				}
				void ToHourly(){
				  lcd.createChar(0, Dozd);
				  lcd.createChar(1, oblazn);
				  lcd.createChar(2, groza);
				  lcd.createChar(3, soln);
				  lcd.createChar(4, sneg);
				  lcd.createChar(5, Delimiters);
				  lcd.createChar(6, LittleDozd);
				  lcd.createChar(7, clockI);
				}
				void ToWatch(){
				  lcd.createChar(0, Dozd);
				  lcd.createChar(1, oblazn);
				  lcd.createChar(2, groza);
				  lcd.createChar(3, soln);
				  lcd.createChar(4, sneg);
				  lcd.createChar(5, Delimiters);
				  lcd.createChar(6, LittleDozd);
				  lcd.createChar(7, clockI);
				}
				LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 16, 2);
			//#endif
            
};
#endif // #ifndef InformetrModules_h