#ifndef config_h
#define config_h

//#define CheckWIFI //Проверять wifi соединение и писать об этом в лог и консоль
#define DebugConnection //Выводить в консоль к кому подключаться
//#define WriteDataToFile //Записывать все передаваемые на шлюз данные в отдельный файл
#define ADCV //Включить измерение напряжения
#define NoAddLogSendData //Не записываль в основной лог о том, что передаем информацию
//#define TimeSending //Отправлять на шлюз время работы модуля
//#define DurationLog //Записывать в лог время передачи пакета
//#define UsingWDT //Использовать сторожевой таймер
//#define NoScanWIFi //Не сканировать сети в конфигураторе
#define StartLED //Светить светодиодам при запуске
#define UsingFastStart //Использовать быструю загрузку - не отключаться от сети при перезапуске
//#define NoWaitConnectFromStart //Не ожидать соединение с WIFI сетью при старте
#define PrintLogToTelnet //Писать логи в telnet
#define UsePortPrint //Использовать абстракцию порт-принта, что бы писать в телнет и сериал, иначе писать только в сериал
#define IsBlinkOTA //Мигать светодиодом при обновлении по воздуху
#define EnableFSManager //Включить веб-менеджер файловой системы
#define CheckConnectGateway //Проверять соединение со шлюзом на главной странице
#define EnableExternalUnits //Включает поддержку дополнительного оборудования модуля
#define RemoveFirstBuffer   //Удаляет самый старый элемент из буфера отправки, когда в нем не хватает места
//#define UseHTTPLibToGateway //Использовать библиотеку http для отправки данных на шлюз, в противном случае использовать собственную реализацию http протокола
#define AutoSendBuffer      //Отправлять автоматически данные на шлюз из буфера

#if !defined(FLASHMODE_DOUT)
	#define FlashWEB //Обновление прошивки через HTTP
	#define FlashOTA //Обновление прошивки через OTA
	#define SmallROM //Используется прошивка для маленькой флешки
#endif

//#define USE_SPIFFS //Использовать SPIFFS файловую систему
#define USE_LITTLEFS //Использовать LITTLEFS файловую систему
//#define USE_SDFS   //Использовать SDFS файловую систему

#define BaseConfigH

#endif

#ifdef SmallROM
	//#undef UseHTTPLibToGateway
#endif

#ifndef config_helper_h
	#define config_helper_h
	
	#if !defined(SFS)
		#if defined USE_SPIFFS
			#include <FS.h>
			#define SFS SPIFFS
		#elif defined USE_LITTLEFS
			#include <LittleFS.h>
			#define SFS LittleFS
		#elif defined USE_SDFS
			#include <SDFS.h>
			#define SFS SDFS
		#else
			#error Please select a filesystem first by uncommenting one of the "#define USE_xxx"
		#endif
	#endif

	#if defined(NoFS)
		#pragma message "NoFS is not supported on current version"
		#define NoCompilation
	#endif
	
	#if defined(UsingWDT)
		#pragma message "WDT is none debug functionality"
	#endif
	
#endif
