#pragma once

#include "Arduino.h"

#include <Udp.h>
#include <RTOS.h>

#define SEVENZYYEARS 2208988800UL
#define NTP_PACKET_SIZE 48
#define NTP_DEFAULT_LOCAL_PORT 1337
#define LEAP_YEAR(Y)     ( (Y>0) && !(Y%4) && ( (Y%100) || !(Y%400) ) )

typedef struct DateTime //Текущее дата и время
{
    byte Hour; 
    byte Minutes; 
	byte Seconds; 
	byte Day; 
    byte Mounth; 
    byte Year; 
	
	int ToMinutes(){
		return (Hour*60) + Minutes;
	}
	int ToSeconds(){
		return (Hour*60*60) + (Minutes*60) + Seconds;
	}
};

/*typedef struct OnlyTime //Текущее время
{
    byte Hour; 
    byte Minutes; 
	byte Seconds; 
};*/

class NTPClient {
  private:
    UDP*          _udp;
    bool          _udpSetup       = false;

    const char*   _poolServerName = "pool.ntp.org"; // Default time server
    int           _port           = NTP_DEFAULT_LOCAL_PORT;
    long          _timeOffset     = 0;

    unsigned long _updateInterval = 3600000;  // In ms

    unsigned long _currentEpoc    = 0;      // In s
    unsigned long _lastUpdate     = 0;      // In ms

    byte          _packetBuffer[NTP_PACKET_SIZE];

    void          sendNTPPacket();
	
	RTOS 		  updater         = RTOS(998);
	DateTime 	  cDT;
	//OnlyTime 	  cT;
	void		  updateDT();

  public:
    NTPClient(UDP& udp);
    NTPClient(UDP& udp, long timeOffset);
    NTPClient(UDP& udp, const char* poolServerName);
    NTPClient(UDP& udp, const char* poolServerName, long timeOffset);
    NTPClient(UDP& udp, const char* poolServerName, long timeOffset, unsigned long updateInterval);

    /**
     * Set time server name
     *
     * @param poolServerName
     */
    void setPoolServerName(const char* poolServerName);

    /**
     * Starts the underlying UDP client with the default local port
     */
    void begin();

    /**
     * Starts the underlying UDP client with the specified local port
     */
    void begin(int port);

    /**
     * This should be called in the main loop of your application. By default an update from the NTP Server is only
     * made every 60 seconds. This can be configured in the NTPClient constructor.
     *
     * @return true on success, false on failure
     */
    bool update();

    /**
     * This will force the update from the NTP Server.
     *
     * @return true on success, false on failure
     */
    bool forceUpdate();

    int getDayWeek() const;
    int getDay() const;
    int getMounth() const;
    int getHours() const;
    int getMinutes() const;
    int getSeconds() const;
    int getYear() const;

    /**
     * Changes the time offset. Useful for changing timezones dynamically
     */
    void setTimeOffset(int timeOffset);

    /**
     * Set the update interval to another frequency. E.g. useful when the
     * timeOffset should not be set in the constructor
     */
    void setUpdateInterval(unsigned long updateInterval);

    /**
     * @return time formatted like `hh:mm:ss`
     */
    String getFormattedTime() const;

    /**
    * @return secs argument (or 0 for current date)
    * like `2004-02-12`
    */
    String getFormattedDate() const;

    /**
    * @return secs argument (or 0 for current date)
    * like `2004-02-12 15:25:45`
    */
    String getFormattedDateTime() const;

    /**
     * @return time in seconds since Jan. 1, 1970
     */
    unsigned long getEpochTime() const;
	
	DateTime getDateTime() const;

    /**
     * Stops the underlying UDP client
     */
    void end();
};
