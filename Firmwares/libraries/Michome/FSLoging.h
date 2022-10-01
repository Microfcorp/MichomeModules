#ifndef FSLoging_h
#define FSLoging_h
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 
#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WebServer.h>

#define LogFileName "/logfile.txt"

#ifndef NoFS
class FSLoging
{
        public:
            void AddLogFile(String textadd){
                File f = LittleFS.open(LogFileName, "a");
                if (!f) {
                    Serial.println("LOG open failed");  //  "открыть файл не удалось"
                }
                else{
                    if (f.print(textadd + " <br /> ")) {} 
					else {
						Serial.println("LOG append failed");
					}
					f.close();
                }
            }
            String ReadLogFile(){      
                File f = LittleFS.open(LogFileName, "r");
                if (!f) {
                    Serial.println("LOG file open failed");  //  "открыть файл не удалось"
                    return "";
                }
                else{
                    String cfg = f.readString();
                    f.close();//денициализация фс                        
                    return cfg;
                }    
            }
			void ReadLogFileToServer(ESP8266WebServer *server){				
				File f = LittleFS.open(LogFileName, "r");
				if ((*server).streamFile(f, F("text/html")) != f.size()) {
				  Serial.println("Sent less data than expected!");
				}
				f.close();								    
            }
            void WriteLogFile(String text){
                File f = LittleFS.open(LogFileName, "w");
                if (!f) {
                    Serial.println("LOG file open failed");  //  "открыть файл не удалось"
                    return;
                }
                else{
                    f.print(text);
                    f.close();//денициализация фс                     
                    return;
                }
            }
            void ClearLogFile(){
                WriteLogFile("");
            }
};
#endif 
#endif // #ifndef Michom_h