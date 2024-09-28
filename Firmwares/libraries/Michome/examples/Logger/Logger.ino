#include <Michom.h>

char* id = "Logger";
const char* type = "Log";
double VersionUploader = 1.1;
/////////настройки//////////////

Michome michome(id, type, VersionUploader);

Logger debuging = michome.GetLogger();

ESP8266WebServer& server1 = michome.GetServer();

CreateMichome;

void setup(void) 
{      
    server1.on("/printlog", [](){
        debuging.Log("On printlog");        
        server1.send(200, "text/html", String("OK"));    
    });    
    michome.init(true);
}

void loop(void)
{
    michome.running();
}