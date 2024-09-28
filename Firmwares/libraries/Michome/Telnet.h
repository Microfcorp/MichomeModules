#ifndef Telnet_h
#define Telnet_h

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 

#define MAX_SRV_CLIENTS 4
#define NullString "NullStringT"
#define STANDARTTELNETPORT 23

// ansi stuff, could always use printf instead of concat
#define ansiPRE  "\033 " // escape code
#define ansiHOME "\033[H " // cursor home
#define ansiESC  "\033[2J " // esc
#define ansiCLC  "\033[?25l " // invisible cursor

#define ansiEND  "\033[0m"   // closing tag for styles
#define ansiBOLD "\033[1m"

#define ansiSTRB "\033[5m"   //мигающий
#define ansiCORS "\033[4m"   //подчеркнутый

#define ansiRED  "\033[41m " // red background
#define ansiGRN  "\033[42m " // green background
#define ansiBLU  "\033[44m " // blue background

#define ansiREDF "\033[31m " // red foreground
#define ansiBLUF "\033[34m " // blue foreground
#define ansiGRNF "\033[32m " // green foreground
#define BELL     "\a "

#define ansiClearScreen (ansiHOME+ansiCLC)

#define InputArea(str) (str.print((String)ansiBOLD + ansiGRNF + (String)id + "> " + ansiEND))

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <LinkedList.h>
#include <Module.h>

typedef std::function<void(void)> THandlerFunction;

typedef struct OnData
{
	String name;
	String descreption;
	THandlerFunction func;	
};

class Telnet
{
        public:
                //Объявление класса
                Telnet(int port, String ID);
				~Telnet(){};
                //
                String GetData();
                //
                bool IsDataAvalible();
                //
                void Running();
                //
                void print(String text);
                //
                void println(String text);
				void println(){println("");};
				//
                void printlnNL(String text);
				//
                void printlnNIA(String text);
				//		
				//String Split(String data, char separator, int index);
				//
				void on(const String &linq, const String &descreption, THandlerFunction handler);
				//
				void on(const String &linq, const String &descreption){on(linq, descreption, [](){});};
				//
				void printSucess(String text){
					print(F("\n"));
					println(ansiGRNF + text + ansiEND);
				}
				void printError(String text){
					print(F("\n"));
					println(ansiRED + text + ansiEND);
				}
                //
                String read(){
                    return GetData();
                }
                //
                void Init();
				//
				void SetID(String ID){
					id = ID;
				}
				//
				bool IsCorrect = false;
                
        private:
			WiFiServer servT = WiFiServer(23);
            bool IsReadConfig = false;
			String id;
            WiFiClient serverClients[MAX_SRV_CLIENTS];
			LinkedList<OnData> Ons = LinkedList<OnData>();
            String RD = NullString;
            
};
#endif // #ifndef Telnet_h