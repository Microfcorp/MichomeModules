#ifndef DS3231_H
#define DS3231_H

#include <Arduino.h>
#include <time.h>                       // time() ctime()
#include <sys/time.h>                   // struct timeval

class DS3231 {
public:
	DS3231(uint8_t address){
		i2c_address = address;
	}
	void setTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t
				dayOfMonth, uint8_t month, uint8_t year){
		// передаем данные о времени и дате на DS3231:
		Wire.beginTransmission(i2c_address);
		// выставляем регистр указателей DS3231 на регистр «00h»,
		// т.е. на регистр, в котором хранятся данные о секундах:
		Wire.write(0);
		Wire.write(decToBcd(second)); // задаем секунды
		Wire.write(decToBcd(minute)); // задаем минуты
		Wire.write(decToBcd(hour)); // задаем часы
		Wire.write(decToBcd(dayOfWeek));
		// задаем день недели (1 – это воскресенье, 7 – это суббота)
		Wire.write(decToBcd(dayOfMonth)); // задаем дату (от 1 до 31)
		Wire.write(decToBcd(month)); // задаем месяц
		Wire.write(decToBcd(year)); // задаем год (от 0 до 99)
		Wire.endTransmission();
	}
	void readTime(uint8_t *second,
				uint8_t *minute,
				uint8_t *hour,
				uint8_t *dayOfWeek,
				uint8_t *dayOfMonth,
				uint8_t *month,
				uint8_t *year){
		Wire.beginTransmission(i2c_address);
		// выставляем регистр указателей DS3231 на регистр «00h»:
		Wire.write(0);
		Wire.endTransmission();
		Wire.requestFrom(i2c_address, (uint8_t)7);
		// запрашиваем у DS3231 семь байтов начиная с регистра «00h»:
		*second = bcdToDec(Wire.read() & 0x7f);
		*minute = bcdToDec(Wire.read());
		*hour = bcdToDec(Wire.read() & 0x3f);
		*dayOfWeek = bcdToDec(Wire.read());
		*dayOfMonth = bcdToDec(Wire.read());
		*month = bcdToDec(Wire.read());
		*year = bcdToDec(Wire.read());
	}
	void readTime(){
		readTime(&second,
				&minute,
				&hour,
				&dayOfWeek,
				&dayOfMonth,
				&month,
				&year);
	}
	void setTime(){
		setTime(second,
				minute,
				hour,
				dayOfWeek,
				dayOfMonth,
				month,
				year);
	}
	
	unsigned long UNIXTime(){
		struct tm* timeinfo;

		readTime();

		/* now modify the timeinfo to the given date: */
		timeinfo->tm_year   = (2000 + year) - 1900;
		timeinfo->tm_mon    = month - 1;    //months since January - [0,11]
		timeinfo->tm_mday   = dayOfMonth;          //day of the month - [1,31] 
		timeinfo->tm_hour   = hour;         //hours since midnight - [0,23]
		timeinfo->tm_min    = minute;          //minutes after the hour - [0,59]
		timeinfo->tm_sec    = second;          //seconds after the minute - [0,59]

		/* call mktime: create unix time stamp from timeinfo struct */
		return mktime(timeinfo);
	}
	
	uint8_t second = 15; 
	uint8_t minute = 39; 
	uint8_t hour = 1; 
	uint8_t dayOfWeek = 4; 
	uint8_t dayOfMonth = 19; 
	uint8_t month = 07; 
	uint8_t year = 23;
	
protected:
	uint8_t decToBcd(uint8_t val){
		return( (val/10*16) + (val%10) );
	}
	// конвертируем двоично-десятичный код в обычные десятичные значения: 
	uint8_t bcdToDec(uint8_t val){
		return( (val/16*10) + (val%16) );
	}
	uint8_t i2c_address;
};

#endif /* DS3231_H */

