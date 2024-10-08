/**
 * The MIT License (MIT)
 * Copyright (c) 2015 by Fabrice Weinberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "NTPClient.h"

NTPClient::NTPClient(UDP& udp) {
  this->_udp            = &udp;
}

NTPClient::NTPClient(UDP& udp, long timeOffset) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName) {
  this->_udp            = &udp;
  this->_poolServerName = poolServerName;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName, long timeOffset) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerName = poolServerName;
}

NTPClient::NTPClient(UDP& udp, const char* poolServerName, long timeOffset, unsigned long updateInterval) {
  this->_udp            = &udp;
  this->_timeOffset     = timeOffset;
  this->_poolServerName = poolServerName;
  this->_updateInterval = updateInterval;
}

void NTPClient::begin() {
  this->begin(NTP_DEFAULT_LOCAL_PORT);
}

void NTPClient::begin(int port) {
  this->_port = port;

  this->_udp->begin(this->_port);

  this->_udpSetup = true;
  updater.Start();
  
  //
  setTZ(TZ_Europe_Moscow);
}

bool NTPClient::forceUpdate() {
  #ifdef DEBUG_NTPClient
    Serial.println("Update from NTP Server");
  #endif

  this->sendNTPPacket();

  // Wait till data is there or timeout...
  byte timeout = 0;
  int cb = 0;
  do {
    delay ( 10 );
    cb = this->_udp->parsePacket();
    if (timeout > 100) return false; // timeout after 1000 ms
    timeout++;
  } while (cb == 0);

  this->_lastUpdate = millis() - (10 * (timeout + 1)); // Account for delay in reading the time

  this->_udp->read(this->_packetBuffer, NTP_PACKET_SIZE);

  unsigned long highWord = word(this->_packetBuffer[40], this->_packetBuffer[41]);
  unsigned long lowWord = word(this->_packetBuffer[42], this->_packetBuffer[43]);
  // combine the four bytes (two words) into a long integer
  // this is NTP time (seconds since Jan 1 1900):
  unsigned long secsSince1900 = highWord << 16 | lowWord;

  this->_currentEpoc = secsSince1900 - SEVENZYYEARS;

  return true;
}

bool NTPClient::update() {
  bool trt = false;
  if ((millis() - this->_lastUpdate >= this->_updateInterval)     // Update after _updateInterval
    || this->_lastUpdate == 0) {                                  // Update if there was no update yet.
    if (!this->_udpSetup) this->begin();                          // setup the UDP client if needed
    trt = this->forceUpdate();
	if(trt){
		time_t epoch_t = this->getEpochTime();
		// set the system time to UTC
		timeval tv = { epoch_t, 0 };
		settimeofday(&tv, nullptr);
		if(!_IsSync){
			_IsSync = true;
			for(uint8_t i = 0; i < _FirstSync.size(); i++){
				_FirstSync.get(i)(epoch_t);
			}
		}
	}
  }
  if(updater.IsTick()){
	  updateDT();
  }
  trt = true;
  return trt;
}

unsigned long NTPClient::getEpochTime() const {
  return this->_timeOffset + // User offset
         this->_currentEpoc + // Epoc returned by the NTP server
         ((millis() - this->_lastUpdate) / 1000); // Time since last update
}

int NTPClient::getDayWeek() const {
  return (((this->getEpochTime() / 86400L) + 4 ) % 7); //0 is Sunday
}
int NTPClient::getDay() const {
	unsigned long rawTime = this->getEpochTime() / 86400L;  // in days
	unsigned long days = 0, year = 1970;
	uint8_t month;
	static const uint8_t monthDays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

	while ((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
		year++;
	rawTime -= days - (LEAP_YEAR(year) ? 366 : 365); // now it is days in this year, starting at 0
	days = 0;
	for (month = 0; month < 12; month++) {
		uint8_t monthLength;
		if (month == 1) { // february
			monthLength = LEAP_YEAR(year) ? 29 : 28;
		}
		else {
			monthLength = monthDays[month];
		}
		if (rawTime < monthLength) break;
		rawTime -= monthLength;
	}
	//String monthStr = ++month < 10 ? "0" + String(month) : String(month); // jan is month 1  
	//String dayStr = ++rawTime < 10 ? "0" + String(rawTime) : String(rawTime); // day of month  
	return ++rawTime;
}
int NTPClient::getMounth() const {
	unsigned long rawTime = this->getEpochTime() / 86400L;  // in days
	unsigned long days = 0, year = 1970;
	uint8_t month;
	static const uint8_t monthDays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

	while ((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
		year++;
	rawTime -= days - (LEAP_YEAR(year) ? 366 : 365); // now it is days in this year, starting at 0
	days = 0;
	for (month = 0; month < 12; month++) {
		uint8_t monthLength;
		if (month == 1) { // february
			monthLength = LEAP_YEAR(year) ? 29 : 28;
		}
		else {
			monthLength = monthDays[month];
		}
		if (rawTime < monthLength) break;
		rawTime -= monthLength;
	}
	return ++month;
}
int NTPClient::getHours() const {
  return ((this->getEpochTime() % 86400L) / 3600);
}
int NTPClient::getMinutes() const {
  return ((this->getEpochTime() % 3600) / 60);
}
int NTPClient::getSeconds() const {
  return (this->getEpochTime() % 60);
}
int NTPClient::getYear() const {
    unsigned long rawTime = this->getEpochTime() / 86400L;  // in days
    unsigned long days = 0, year = 1970;
    while ((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
        year++;
    return year;
}

String NTPClient::getFormattedTime() const {
  unsigned long rawTime = this->getEpochTime();
  unsigned long hours = (rawTime % 86400L) / 3600;
  String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);

  unsigned long minutes = (rawTime % 3600) / 60;
  String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

  unsigned long seconds = rawTime % 60;
  String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

  return hoursStr + ":" + minuteStr + ":" + secondStr;
}

// Based on https://github.com/PaulStoffregen/Time/blob/master/Time.cpp
// currently assumes UTC timezone, instead of using this->_timeOffset
String NTPClient::getFormattedDate() const{
    unsigned long rawTime = this->getEpochTime() / 86400L;  // in days
    unsigned long days = 0, year = 1970;
    uint8_t month;
    static const uint8_t monthDays[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };

    while ((days += (LEAP_YEAR(year) ? 366 : 365)) <= rawTime)
        year++;
    rawTime -= days - (LEAP_YEAR(year) ? 366 : 365); // now it is days in this year, starting at 0
    days = 0;
    for (month = 0; month < 12; month++) {
        uint8_t monthLength;
        if (month == 1) { // february
            monthLength = LEAP_YEAR(year) ? 29 : 28;
        }
        else {
            monthLength = monthDays[month];
        }
        if (rawTime < monthLength) break;
        rawTime -= monthLength;
    }
    String monthStr = ++month < 10 ? "0" + String(month) : String(month); // jan is month 1  
    String dayStr = ++rawTime < 10 ? "0" + String(rawTime) : String(rawTime); // day of month  
    return String(year) + "-" + monthStr + "-" + dayStr;
}

String NTPClient::getFormattedDateTime() const {
    return getFormattedDate() + " " + getFormattedTime();
}

DateTime NTPClient::getDateTime() const {
	return cDT;
}

void NTPClient::updateDT() {
    DateTime DT;
	DT.Hour = getHours();
	DT.Minutes = getMinutes();
	DT.Seconds = getSeconds();
	DT.Day = getDay();
	DT.Mounth = getMounth();
	DT.Year = getYear();
	cDT = DT;
}

void NTPClient::end() {
  this->_udp->stop();

  this->_udpSetup = false;
}

void NTPClient::setTimeOffset(int timeOffset) {
  this->_timeOffset     = timeOffset;
}

void NTPClient::setUpdateInterval(unsigned long updateInterval) {
  this->_updateInterval = updateInterval;
}

void NTPClient::setPoolServerName(const char* poolServerName) {
  this->_poolServerName = poolServerName;
}

void NTPClient::sendNTPPacket() {
  // set all bytes in the buffer to 0
  memset(this->_packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  this->_packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  this->_packetBuffer[1] = 0;     // Stratum, or type of clock
  this->_packetBuffer[2] = 6;     // Polling Interval
  this->_packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  this->_packetBuffer[12]  = 49;
  this->_packetBuffer[13]  = 0x4E;
  this->_packetBuffer[14]  = 49;
  this->_packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  this->_udp->beginPacket(this->_poolServerName, 123); //NTP requests are to port 123
  this->_udp->write(this->_packetBuffer, NTP_PACKET_SIZE);
  this->_udp->endPacket();
}
