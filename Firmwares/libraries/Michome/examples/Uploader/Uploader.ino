#include <Michom.h>

#pragma message This is only uploader michome core script

char* id = "Uploader";
const char* type = "Log";
double VersionUploader = 1.1;
/////////настройки//////////////

Michome michome(id, type, VersionUploader);

ESP8266WebServer& server1 = michome.GetServer();

CreateMichome;

void setup(void) 
{      
    server1.on("/logstatus", [](){ 
        server1.send(200, "text/html", String("OK"));    
    });    
    michome.init(true);
}

void loop(void)
{
    michome.running();
}