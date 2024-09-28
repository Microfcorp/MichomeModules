#ifndef ExternalUnits_h
#define ExternalUnits_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define Enable24Cxx
#define EnableDS3231

#define AT24CAddr 0x50
#define DS3231Addr 0x68

// Set I2C bus to use: Wire, Wire1, etc.
#define WIRE Wire

#ifdef WIRE
	#include <Wire.h>
	#ifdef Enable24Cxx
		#include <AT24Cxx.h>
	#endif
	#ifdef EnableDS3231
		#include <DS3231.h>
	#endif
#endif
#include <LinkedList.h>

class ExternalUnits
{
        public:
                ExternalUnits()
				{
					
				}
				#ifdef WIRE
					void init(){
						WIRE.begin();
					}
					uint8_t WIREScan(){
						foundWIRE.clear();
						uint8_t error, address = 1, nDevices = 0;
						for(address = 1; address < 127; address++) 
						{
							WIRE.beginTransmission(address);
							error = WIRE.endTransmission();
							if (error == 0)
							{
							  foundWIRE.add(address);
							  nDevices++;
							}   
						}
						return nDevices;
					}
					
					bool isWIREFound(uint8_t deviceAddr){
						for(uint8_t i = 0; i < foundWIRE.size(); i++){
							if(foundWIRE.get(i) == deviceAddr)
								return true;
						}
						WIRE.beginTransmission(deviceAddr);
						uint8_t error = WIRE.endTransmission();
						if (error == 0){
							foundWIRE.add(deviceAddr);
							return true;
						}
						return false;
					}

					
					bool isAT24CXX(){
						#if defined(Enable24Cxx)
							return isWIREFound(AT24CAddr);
						#else
							return false;
						#endif
					}
					#ifdef Enable24Cxx
						AT24Cxx& GetAT24CXX(){
							return at24;
						}
					#endif
					
					bool isDS3231(){
						#if defined(EnableDS3231)
							return isWIREFound(DS3231Addr);
						#else
							return false;
						#endif
					}
					#ifdef EnableDS3231
						DS3231& GetDS3231(){
							return ds32;
						}
					#endif
					LinkedList<uint8_t> foundWIRE = LinkedList<uint8_t>();
				#endif				
        private:
			#ifdef WIRE			
				#ifdef Enable24Cxx
					AT24Cxx at24 = AT24Cxx(AT24CAddr);
				#endif
				#ifdef EnableDS3231
					DS3231 ds32 = DS3231(DS3231Addr);
				#endif
			#endif
};

#endif // #ifndef ExternalUnits_h