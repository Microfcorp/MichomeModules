#ifndef FSFiles_h
#define FSFiles_h
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif 
#include "config.h"
#include <ESP8266WebServer.h>

#ifndef NoFS
class FSFiles
{
        public:
            FSFiles(String path){
                FilePath = path;
            }
            
            void AddTextToFile(String textadd){
				ErrorOpen = false;
                File f = SFS.open(FilePath, "a");
                if (!f) {
                    Serial.println("file "+FilePath+" append-mode open failed");  //  "открыть файл не удалось"
					ErrorOpen = true;
                }
                else{
                    if (f.print(textadd + " <br /> ")) {} 
					else {
						Serial.println("Append to "+FilePath+" failed");
					}
					delay(500);
					f.close();
                }
            }
            String ReadFile(){
				ErrorOpen = false;
                File f = SFS.open(FilePath, "r");
                if (!f) {
                    Serial.println("file "+FilePath+" from read-mode open failed");  //  "открыть файл не удалось"
					ErrorOpen = true;
                    return "";
                }
                else{
                    String cfg = f.readString();
					f.close();
                    return cfg;
                }    
            }
			void ReadFileToServer(ESP8266WebServer *server){				
				File f = SFS.open(FilePath, "r");
				if ((*server).streamFile(f, F("text/html")) != f.size()) {
				  Serial.println("Sent less data than expected!");
				}
				f.close();								    
            }
            void WriteFile(String text){
				ErrorOpen = false;
                File f = SFS.open(FilePath, "w");
                if (!f) {
                    Serial.println("file "+FilePath+" write-mode open failed");  //  "открыть файл не удалось"
					ErrorOpen = true;
                    return;
                }
                else{
                    f.print(text); 
					delay(500); // Make sure the CREATE and LASTWRITE times are different					
					f.close();
                    return;
                }
            }
            void ClearFile(){
                WriteFile("");
            }
			
			bool Exist(){
				return SFS.exists(FilePath);
			}
			
			bool ErrorOpen = false;			
        private:
            String FilePath = "/file.txt";
};
#endif 
#endif // #ifndef Michom_h