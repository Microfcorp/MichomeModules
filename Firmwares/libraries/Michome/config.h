#ifndef config_h
#define config_h

//#define NoFS //Не использовать файловую систему
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

#endif