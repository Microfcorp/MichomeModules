#ifndef FSLoging_h
#define FSLoging_h
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 
#include "config.h"
#include <ESP8266WebServer.h>

#define LogFileName "/logfile.txt"

#if defined(FLASHMODE_DOUT)
	#define MaxLogFileBytes 204800
#else
	#define MaxLogFileBytes 1048576
#endif

#ifndef NoFS
class FSLoging
{
        public:
            void AddLogFile(String textadd){
                File f = SFS.open(LogFileName, "a");
                if (!f) {
                    Serial.println(F("LOG open failed"));  //  "открыть файл не удалось"
                }
                else{
					if(f.size() >= MaxLogFileBytes){
						ClearLogFile();
						f.print(F("Log file is full <br /> "));
						f.print(textadd + " <br /> ");
					}
                    if (!f.print(textadd + " <br /> ")){
						Serial.println(F("LOG append failed"));
					}
					f.close();
                }
            }
            String ReadLogFile(){      
                File f = SFS.open(LogFileName, "r");
                if (!f) {
                    Serial.println(F("LOG file open failed"));  //  "открыть файл не удалось"
                    return "";
                }
                else{
                    String cfg = f.readString();
                    f.close();//денициализация фс                        
                    return cfg;
                }    
            }
			void ReadLogFileToServer(ESP8266WebServer *server){				
				File f = SFS.open(LogFileName, "r");
				if ((*server).streamFile(f, F("text/html")) != f.size()) {
				  Serial.println(F("Sent less data than expected!"));
				}
				f.close();								    
            }
            void WriteLogFile(String text){
                File f = SFS.open(LogFileName, "w");
                if (!f) {
                    Serial.println(F("LOG file open failed"));  //  "открыть файл не удалось"
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