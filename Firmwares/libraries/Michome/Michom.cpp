#include "Michom.h"
#include "config.h"

#ifndef config_h
    #error "Error Read Config Files";     
#endif

#ifndef GetewayData_h
    #error "Error Read Geteway Data File";     
#endif

ESP8266WebServer server(80);

//
// конструктор - вызывается всегда при создании экземпляра класса Michome
//
Michome::Michome(const char* _ssid, const char* _password, const char* _id, const char* _type, const char* _host1, double FirmwareVersion)
{
	strncpy(MainConfig.WIFI[0].SSID, _ssid, WL_SSID_MAX_LENGTH);
	strncpy(MainConfig.WIFI[0].Password, _password, WL_WPA_KEY_MAX_LENGTH);
    id = _id;
    type = _type;
	FirVersion = FirmwareVersion;
	strncpy(MainConfig.Geteway, _host1, sizeof(MainConfig.Geteway));
    mdns = MDNSResponder();
    //this.server = server;
    IsNeedReadConfig = false;	
	VoidRefreshData = [&](){DefaultRefresh();};
	SetOptionFirmware(UDPTrigger, true);
	udpmodule = MichomeUDP(id, type);
}

#ifndef NoFS
//
// конструктор - вызывается всегда при создании экземпляра класса Michome
//
Michome::Michome(const char* _id, const char* _type, double FirmwareVersion)
{   
    //ssid = "";
    //password = "";
    //String("").toCharArray(ssid, WL_SSID_MAX_LENGTH);
    //String("").toCharArray(password, WL_WPA_KEY_MAX_LENGTH);
    id = _id;
    type = _type;
	FirVersion = FirmwareVersion;
    mdns = MDNSResponder();
    //this.server = server;
    IsNeedReadConfig = true;
	VoidRefreshData = [&](){DefaultRefresh();};
	SetOptionFirmware(UDPTrigger, true);
	udpmodule = MichomeUDP(id, type);
}
#endif

ESP8266WebServer& Michome::GetServer(){
    return server;
}

Telnet& Michome::GetTelnet(){
    return telnetmodule;
}

MichomeUDP& Michome::GetUDP(){
    return udpmodule;
}

//
// Инициализация в Setup
//
void Michome::init(bool senddata)
{
      _init();     
      
      int t1 = TimeoutConnection;
      TimeoutConnection = 1500;
      SendData(ParseJson(Initialization, ""));
      TimeoutConnection = t1;
      
      if(senddata){
        SendData();
      }
}
//
// Инициализация в Setup
//
void Michome::init()
{
      init(false);     
}
//
// Инициализация в Setup
//
void Michome::preInit(void)
{
	if(!IsPreInt){
		#ifndef NoFS
			if (!LittleFS.begin()) {
				Serial.println("LittleFS mount failed");
			}
		#endif   
		IsPreInt = true;
	}
}
//
// Инициализация в Setup
//
void Michome::_init(void)
{
    pinMode(BuiltLED, OUTPUT);
    #ifdef StartLED
        digitalWrite(BuiltLED, LOW);
    #endif
	
	if(ESP.getResetReason() == "Exception" || ESP.getResetReason() == "Hardware Watchdog"){
		IsSaveMode = true;
		SaveModeReboot.Start();
	}
	
	preInit();
	
	#ifndef NoSerial
        Serial.begin ( 115200 );
        Serial.setDebugOutput(false); //Куча логов. Путти нормально их не ест
    #endif 
	
    #ifndef NoFS
		if (!LittleFS.begin()) {
			Serial.println("LittleFS mount failed");
			return;
		}
        AddLogFile("Start CPU OK", false);
		if(IsSaveMode){
			AddLogFile("Safe Mode Enable", false);
			Serial.println("Run on Safe Mode");
		}
    #endif  
      
      Serial.println("");      
      
      ArduinoOTA.setHostname(id);     
      ArduinoOTA.onStart([this]() {
        Serial.println("Start");
        #ifndef NoFS
            AddLogFile("Update OTA Start", false);
        #endif
      });     
      ArduinoOTA.onEnd([this]() {
        Serial.println("\nEnd");
        #ifndef NoFS
            AddLogFile("Update OTA End", false);
        #endif
      });
      ArduinoOTA.onProgress([this](unsigned int progress, unsigned int total) {
        if(IsBlinkOTA){
            StrobeBuildLed(10);
        }
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      });
      ArduinoOTA.onError([this](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
          Serial.println("Auth Failed");
          #ifndef NoFS
              AddLogFile("Error OTA Auth Failed", false);
          #endif
        } else if (error == OTA_BEGIN_ERROR) {
          Serial.println("Begin Failed");
          #ifndef NoFS
              AddLogFile("Error OTA Begin Failed", false);
          #endif
        } else if (error == OTA_CONNECT_ERROR) {
          Serial.println("Connect Failed");
          #ifndef NoFS
              AddLogFile("Error OTA Connect Failed", false);
          #endif
        } else if (error == OTA_RECEIVE_ERROR) {
          Serial.println("Receive Failed");
          #ifndef NoFS
              AddLogFile("Error OTA Recive Failed", false);
          #endif
        } else if (error == OTA_END_ERROR) {
          Serial.println("End Failed");
          #ifndef NoFS
              AddLogFile("Error OTA End Failed", false);
          #endif
        }
      });
      ArduinoOTA.begin();      
      
	  #if defined(DebugConnection)
        Serial.println("----------Module Connection is starting----------");
      #endif
	  
      if(IsNeedReadConfig){
           MainConfig = ReadWIFIConfig();         
                               
           #if defined(DebugConnection)
		    Serial.println("Setting readed from " + (String)CountWIFI + " points");
		    for(int i = 0; i < CountWIFI; i++){
				if(MainConfig.WIFI[i].SSID[0] != '\0')
					Serial.println("Point "+(String)i+". SSID: "+(String)MainConfig.WIFI[i].SSID +" Password: "+(String)MainConfig.WIFI[i].Password);
				else
					Serial.println("None Point "+(String)i);
			}
           #endif
      }             
      
      #if defined(UsingFastStart)
		if(!IsFoundSSID(WiFi.SSID()))
			WiFi.disconnect(true);
		
        wl_status_t status = WiFi.status();
		if(status == WL_DISCONNECTED || status == WL_NO_SSID_AVAIL || status == WL_IDLE_STATUS || status == WL_CONNECT_FAILED){
            ChangeWiFiMode();
        }
		#if defined(DebugConnection)
			Serial.println("Fast Start is enable");
		#endif
      #else
        WiFi.disconnect(true);     
		ChangeWiFiMode();
		#if defined(DebugConnection)
			Serial.println("Fast Start is disable");
		#endif
      #endif

      WiFi.hostname(id);
      
	  for(int i = 0; i < CountWIFI; i++){
		  if(MainConfig.WIFI[i].SSID[0] != '\0'){
			  wifiMulti.addAP(MainConfig.WIFI[i].SSID, MainConfig.WIFI[i].Password);
			  IsConfigured = true;
		  }		  
	  }
	  
	  wifiMulti.run();	      
	  #if defined(NoWaitConnectFromStart)
		  CreateAP();
	  #else		
		  if(IsConfigured){
			  #if defined(DebugConnection)
				Serial.println("Connect to WIFI, please waiting...");
			  #endif
	  
			  long wifi_try = millis();      
			  // Wait for connection
			  while (wifiMulti.run() != WL_CONNECTED) {		
				delay(500);
				#if defined(DebugConnection)
					Serial.print("."); 
				#endif
				if (millis() - wifi_try > WaitConnectWIFI) break;
			  }
			  #if defined(DebugConnection)
				Serial.println("");
			  #endif
		  }	  
      
		  if(wifiMulti.run() != WL_CONNECTED || !IsConfigured){
			  #ifndef NoFS
				AddLogFile("Error Connecting to "+String(WiFi.SSID())+" - WIFI", false);
				AddLogFile("Start AP", false);
			  #endif
			  #if defined(DebugConnection)
				Serial.println("Error Connecting to "+String(WiFi.SSID())+" - WIFI");
				Serial.println("Start AP");
			  #endif			  
			  StrobeBuildLedError(2, LOW);
			  CreateAP();
			  IsConfigured = false;
		  }
		  else{
			  #if defined(DebugConnection)
				Serial.println("Sucess connect to "+String(WiFi.SSID()));
				Serial.println("IP is "+WiFi.localIP().toString());
			  #endif
		  }
      #endif
	  
      #if defined(DebugConnection)
        Serial.println("----------Module Connection is finished----------");
      #endif
      
      String locals = (String)id;
      locals.replace('_', '-');
      locals.toLowerCase();
      char mdsnname[locals.length()];
      (locals).toCharArray(mdsnname, locals.length());
      if ( mdns.begin ( mdsnname, WiFi.localIP() ) ) {
        #if defined(DebugConnection)
            Serial.println("MDNS Starting");
        #endif
        mdns.addService("http", "tcp", 80);
        mdns.addService("telnet", "tcp", 23);
      }
	  else{
		#if defined(DebugConnection)
            Serial.println("MDNS Start error");
        #endif  
	  }
      LLMNR.begin(mdsnname);

	  server.on("/updatemanager", HTTP_GET, [this](){
          const char* serverIndex = "Please, select *.bin file firmware: <form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>";
          server.sendHeader("Connection", "close");
          server.sendHeader("Access-Control-Allow-Origin", "*");
          server.send(200, "text/html", serverIndex);
        });
      server.on("/update", HTTP_POST, [this](){
          server.sendHeader("Connection", "close");
          server.sendHeader("Access-Control-Allow-Origin", "*");
          server.send(200, "text/plain", (Update.hasError())?"FAIL":"OK");
		  AddLogFile((Update.hasError())?"HTTP Update Fail":"HTTP Update OK");
          ESP.restart();
        },[this](){
          HTTPUpload& upload = server.upload();
          if(upload.status == UPLOAD_FILE_START){
            Serial.setDebugOutput(true);
            WiFiUDP::stopAll();
            Serial.printf("Update: %s\n", upload.filename.c_str());
			AddLogFile("HTTP Update Start");
            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if(!Update.begin(maxSketchSpace)){//start with max available size
			  AddLogFile("HTTP Update Fail");
              Update.printError(Serial);
            }
          } else if(upload.status == UPLOAD_FILE_WRITE){
            if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
			  AddLogFile("HTTP Update Fail");
              Update.printError(Serial);
            }
          } else if(upload.status == UPLOAD_FILE_END){
            if(Update.end(true)){ //true to set the size to the current progress
              Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
		      AddLogFile("HTTP Update Fail");
              Update.printError(Serial);
            }
            Serial.setDebugOutput(false);
          }
          yield();
        });
	  
	  
      server.on("/", [this](){
		  if(!IsSaveMode)
		  {
		   #if !defined(NoFs)
			if(!IsConfigured){
				GetConfigerator();
			}
			else{ 
				GetMainWeb();	
			}                        	   
		   #else
			GetMainWeb();
		   #endif
		  }
		  else{
			server.send(200, "text/html", "Module is Safe Mode. Please reboot module or update firmware on /updatemanager");  
		  }
      });
	  
	  server.on("/restart", [this](){ 
        server.send(200, "text/html", "OK");
        #ifndef NoFs
			AddLogFile("Restart from WEB");
        #endif
        ESP.reset();
      });  
	  
	  if(IsSaveMode)
	  {
		  server.begin();
		  return;
	  }
      
      server.on("/generate_204", [this](){  //Android captive portal. Maybe not needed. Might be handled by notFound handler.  
		#if defined(DebugConnection)
            Serial.println("Android Captive-portal request");
        #endif
		GetMainWeb();
      });
      server.on("/fwlink", [this](){  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.   
		#if defined(DebugConnection)
            Serial.println("Microsoft Captive-portal request");
        #endif	  
		GetMainWeb();
      });        

	  server.on("/refresh", [this](){ 
        server.send(200, "text/html", "OK");
		VoidRefreshData();
      });  	  

	  server.on("/allclear", [this](){
		LittleFS.format();
        server.send(200, "text/html", "LittleFS formating");
      });
      
      server.on("/getmoduleinfo", [this](){
		bool isgetawaytype = !server.hasArg("t");
        String tmpe = (String)Michome::id + (isgetawaytype ? "/n" : "<br />") + (String)Michome::type + (isgetawaytype ? "/n" : "<br />") + String(WiFi.RSSI()) + (isgetawaytype ? "/n" : "<br />") + String(WiFi.localIP().toString()) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(ESP.getFlashChipRealSize()) + (isgetawaytype ? "/n" : "<br />");
        #if defined(ADCV)
            tmpe += String(ESP.getVcc()) + (isgetawaytype ? "/n" : "<br />");
        #else
            tmpe += String("null") + (isgetawaytype ? "/n" : "<br />");
        #endif
		tmpe += String(ESP.getFreeHeap()) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(ESP.getFreeSketchSpace()) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(ESP.getResetReason()) + (isgetawaytype ? "/n" : "<br />");
        server.send(200, "text/html", tmpe);
      });
      
      #ifdef ADCV
           server.on("/getvcc", [this](){
            server.send(200, "text/html", String(ESP.getVcc()));
          });
      #endif
      
      server.on("/setsettings", [this](){         
        String settinga = "";
            for(int i=1; i < server.args(); i++) settinga += server.argName(i) + "=" + server.arg(i) + ";";
        Michome::countsetting = Split(Split(settinga, ';', 0), '=', 1).toInt();
        Michome::settings = settinga;
        IsSettingRead = true;
        Serial.println("Setting read OK");
        #ifndef NoFs
        AddLogFile("Setting read OK", false);
        #endif
        server.send(200, "text/html", settinga);
      });     
      
      server.on("/getsettings", [this](){
        server.send(200, "text/html", GetSetting());
      });
      
      server.on("/getdigital", [this](){
        int pin = server.arg("p").toInt();
        server.send(200, "text/html", String(digitalRead(pin)));
      });
      
      server.on("/getanalog", [this](){
        server.send(200, "text/html", String(analogRead(A0)));
      });    
      
    #ifndef NoFS
      
      server.on("/getconfig", [this](){
		String tmp = "<meta charset=\"UTF-8\">";
		for(int i = 0; i < CountWIFI; i++){
			tmp += "Point "+(String)i+". SSID: "+(String)MainConfig.WIFI[i].SSID +" Password:"+(String)MainConfig.WIFI[i].Password + "<br />";
		}
        server.send(200, "text/html", tmp + "<br />" + (String)MainConfig.Geteway + "<br />" + (String)(MainConfig.UseGeteway ? "true" : "false"));
      });
      
      server.on("/setconfig", [this](){		
        String ss0 = server.arg("ssid0");
        String pw0 = server.arg("password0");
		String ss1 = server.arg("ssid1");
        String pw1 = server.arg("password1");
		String ss2 = server.arg("ssid2");
        String pw2 = server.arg("password2");
		
        String gt = server.arg("geteway");
        bool ug = server.hasArg("usegetaway") ? (server.arg("usegetaway") == "on") : false;
		
		ss0.toCharArray(MainConfig.WIFI[0].SSID, WL_SSID_MAX_LENGTH);
		pw0.toCharArray(MainConfig.WIFI[0].Password, WL_WPA_KEY_MAX_LENGTH);	
		
		ss1.toCharArray(MainConfig.WIFI[1].SSID, WL_SSID_MAX_LENGTH);
		pw1.toCharArray(MainConfig.WIFI[1].Password, WL_WPA_KEY_MAX_LENGTH);
		
		ss2.toCharArray(MainConfig.WIFI[2].SSID, WL_SSID_MAX_LENGTH);
		pw2.toCharArray(MainConfig.WIFI[2].Password, WL_WPA_KEY_MAX_LENGTH);
		
		gt.toCharArray(MainConfig.Geteway, WL_SSID_MAX_LENGTH);		
		MainConfig.UseGeteway = ug;
		
        WriteWIFIConfig();
        
        if(ss0 == "" && pw0 == "")
            IsConfigured = false;
        else
            IsConfigured = true;
        
        AddLogFile("Config Saved");
        server.send(200, "text/html", "Config Saved. Module restarting");
        yieldM();
        delay(200);
        ESP.reset();
      });
      
      server.on("/resetconfig", [this](){
		WIFIConfig cf;		
		
        WriteWIFIConfig(cf);        
        IsConfigured = false;
        
        AddLogFile("Config Reset");
        server.send(200, "text/html", "OK");
        ESP.reset();
      });
	  
	  server.on("/getoptionfirmware", [this](){
		  String tmp = "";
		  for(byte i = 0; i < CountOptionFirmware; i++){
			  tmp += (String)i + " = " + (GetOptionFirmware(i) ? "true" : "false") + "<br />";
		  }
        server.send(200, "text/html", tmp);    
      });
      
      server.on("/configurator", [this](){
		GetConfigerator();    
      });

      server.on("/getlogs", [this](){
        //server.send(200, "text/html", RussianHead("Просмотр системных логов") + "<p><a href='/clearlogs'>Отчистить логи</a></p><br />" + FSLoger.ReadLogFile()); 
		FSLoger.ReadLogFileToServer(&server);
      });  
      
      server.on("/addlog", [this](){
        String setting = server.arg("log");
        AddLogFile(setting);
        server.send(200, "text/html", RussianHead("Добавление лога", MetaRefresh("/getlogs")));    
      });

      server.on("/clearlogs", [this](){
        FSLoger.ClearLogFile();
        server.send(200, "text/html", RussianHead("Отчистка логов..", MetaRefresh()) + "OK");    
      });
	  
	  server.on("/fs", [this](){
		String type = server.arg("type");
		String file = server.arg("file");
		if(type == "read"){
			File f = LittleFS.open(file, "r");
			if (server.streamFile(f, F("text/html")) != f.size()) {
				AddLogFile("Sent less data than expected!");
			}
			f.close();	
		}
		else if(type == "delete"){
			LittleFS.remove(file);	
		}
        server.send(200, "text/html", RussianHead("Отчистка логов..", MetaRefresh()) + "OK");    
      });
      
      #ifdef WriteDataToFile
        server.on("/getdatalogs", [this](){
            //server.send(200, "text/html", RussianHead("Просмотр логов переданных на шлюз данных") + "<p><a href='/resetdatalogs'>Отчистить логи</a></p><br />" +  DataFile.ReadFile());    
			DataFile.ReadFileToServer(&server);
		});
        
        server.on("/cleardatalogs", [this](){
            DataFile.ClearFile();
            server.send(200, "text/html", RussianHead("Отчистка логов..", MetaRefresh()) + "OK");    
        });
      #endif
	  
	    server.on("/saveqsettings", [this](){
			String ntps = server.arg("ntpserver");
			int utco = server.arg("utco").toInt()*60*60;
			TimeSettings.WriteFile(ntps + ";" + (String)utco);
			LoadNTP();
			server.send(200, "text/html", "<meta http-equiv='refresh' content='1;URL=/' />OK");
		});
		server.on("/qsettings", [this](){
			String tmp = RussianHead("Настройка системы таймеров");
			tmp += (String)"<form action='saveqsettings'><table><tr><td>NTP Сервер точного времени:<td><td><input type='text' name='ntpserver' value='"+_NTPServer+"' /></td></tr><tr><td>Часовой пояс:<td><td><input type='number' name='utco' value='"+_utcoffset/60/60+"' /></td></tr></table><input type='submit' value='Сохранить' /></form><br /><a href='/'>Главная</a>";
			server.send(200, "text/html", tmp);
		});
		server.on("/timemodule", [this](){
			server.send(200, "text/html", timeClient.getFormattedDateTime());
		});

    #endif      
      
      server.on("/description.xml", HTTP_GET, [this](){
        SSDP.schema(server.client());
      });     
      
      server.onNotFound([this](){
        server.send(404, "text/html", "Not found from Michome module");
      });
	  
	  udpmodule.on(GetModule(0), [&](IPAddress from, String cmd){
		 if(Split(cmd, '-', 1) == "Gettype"){
			udpmodule.SendMulticast(udpmodule.GetData_MyType());
		 }
		 else if(Split(cmd, '-', 1) == "rssi"){
			udpmodule.SendMulticast((String)WiFi.RSSI());
		 } 
	  });
	  
	  udpmodule.EventSendGateway([&](IPAddress from, String cmd)
	  {
		SendData(ParseJson("UDPData", cmd));
	  });
	  
	  udpmodule.EventSendURL([&](IPAddress from, String cmd)
	  {
		//SendData(ParseJson("UDPData", cmd));
	  });
	  
	  udpmodule.init(server);
      
      server.begin();
      dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
      dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
      
      SSDP.setDeviceType("upnp:rootdevice");
      SSDP.setSchemaURL("description.xml");
      SSDP.setHTTPPort(80);
      SSDP.setName(String(id));
      SSDP.setSerialNumber((String)ESP.getFlashChipId());
      SSDP.setURL("/");
      SSDP.setModelName(String("MichomModule-")+Michome::type);
      SSDP.setModelNumber("000000000001");
      SSDP.setModelURL("http://www.github.com/microfcorp/michome");
      SSDP.setManufacturer("Microf-Corp");
      SSDP.setManufacturerURL("http://www.github.com/microfcorp/michome");
      SSDP.begin();
	  
	  timeClient.begin();
	  LoadNTP();
	  
	  telnetmodule.SetID(id);
	  telnetmodule.Init();	
      
	  WiFi.scanNetworks(true);
	  
      StrobeBuildLed(70);
      
      #ifdef UsingWDT
		  #if defined(DebugConnection)
		  	Serial.println("WDT Enable");
		  #endif
          ESP.wdtDisable();
          ESP.wdtEnable(WDTO_8S);
      #endif
	  
	  #if defined(DebugConnection)
		Serial.println("Module is full load");
		Serial.println("");
      #endif
	  
	  NTPUpdate.Start();
}

void Michome::CreateAP(void){
    WiFi.softAP(Michome::id, PasswordAPWIFi);
    
    IPAddress myIP = WiFi.softAPIP(); //192.168.4.1 is default
	Serial.print("AP IP address: ");
	Serial.println(myIP);    
}

//
// Цикл в Loop
//
void Michome::running(void)
{
	#ifdef UsingWDT
		ESP.wdtFeed();   // покормить пёсика
	#endif
    	
	#if defined(CheckWIFI)
		if((wifiMulti.run() != WL_CONNECTED) && (millis() - wifi_check > 10000) && IsConfigured){
			wifi_check = millis();			
			Serial.println("WIFI connection from "+(String)WiFi.SSID()+" error");			
			#ifndef NoFS
				  AddLogFile("WIFI connection from "+(String)WiFi.SSID()+" error");
			#endif
		}
	#else
		wifiMulti.run();
	#endif

	yieldWarn();
}

void Michome::StrobeBuildLed(byte timeout){
    digitalWrite(BuiltLED, LOW);
    delay(timeout);
    digitalWrite(BuiltLED, HIGH);
}

void Michome::GetMainWeb(){
    WebMain(&server, type, id, timeClient.getFormattedDateTime(), GetOptionFirmware(TimerLightModules), GetOptionFirmware(UDPTrigger), MainConfig.UseGeteway, FirVersion, CheckConnectToGateway());
}

void Michome::ChangeWiFiMode(){
	if (WiFi.getMode() != WIFIMode)
    {
        WiFi.mode(WIFIMode);
        wifi_set_sleep_type(NONE_SLEEP_T); //LIGHT_SLEEP_T and MODE_SLEEP_T
        delay(10);
    }
}

void Michome::StrobeBuildLedError(int counterror, int statusled){
    for(int i = 0; i < counterror; i++)
        StrobeBuildLed(30);
    digitalWrite(BuiltLED, statusled);
}

#ifndef NoFS

#pragma NoFS

WIFIConfig Michome::ReadWIFIConfig(){      
    WIFIConfig cf;
    WIFINetwork nf;
    
    File f = LittleFS.open("/config.bin", "r");
    if (!f) {
        Serial.println("Module configuration file not found");  //  "открыть файл не удалось"
		strncpy(nf.SSID, "", sizeof(nf.SSID));
		strncpy(nf.Password, "", sizeof(nf.Password));
		cf.WIFI[0] = nf;
		cf.WIFI[1] = nf;
		cf.WIFI[2] = nf;
		strncpy(cf.Geteway, "", sizeof(cf.Geteway));
        cf.UseGeteway = false;
        return cf;
    }
    else{
        f.read((byte *)&cf, sizeof(cf));
		f.close();              
        return cf;
    }    
}

void Michome::WriteWIFIConfig(WIFIConfig conf){  
    File f = LittleFS.open("/config.bin", "w");
    if (!f) {
        Serial.println("Error open config.bin for write");  //  "открыть файл не удалось"
        return;
    }
    else{
        f.write((byte *)&conf, sizeof(conf));
		f.close();                    
        return;
    }    
}

#endif

String Michome::GetSetting(String name){
    //update=6000;log=1    
    for(int i = 0; i < countsetting; i++){
        if(Split(Split(settings, ';', i), '=', 0) == name){
            return Split(Split(settings, ';', i), '=', 1);
        }
    }
    return String("");
}

int Michome::GetSettingToInt(String name, int defaults){
    //update=6000;log=1    
    for(int i = 0; i < countsetting; i++){
        if(Split(Split(settings, ';', i), '=', 0) == name){
            return Split(Split(settings, ';', i), '=', 1).toInt();
        }
    }
    return defaults;
}

String Michome::GetSetting(){
    return settings;
}

void Michome::SetFormatSettings(int count){
    countsetting = count;
}

bool Michome::GetSettingRead()
{
    if(IsSettingRead){
        IsSettingRead = false;
        return true;
    }
    return false;
}

bool Michome::CheckConnectToGateway(){
    if((WiFi.status() != WL_CONNECTED))
	    return false;
  
    if(!MainConfig.UseGeteway){
	    return false;
    }
	
	return SendDataGET(MichomePHPPath, GetGatewayHost(), GetGatewayPort()) != "";
}
  
//
// 
//
String Michome::SendDataGET(String gateway, String host, int Port)
{ 
          if((WiFi.status() != WL_CONNECTED))
              return "";          
		  
		  WiFiClient client;
		  HTTPClient http;
		  http.setTimeout(TimeoutConnection);
		  PortPrintln((String)"Start GET connect to " + host + ":" + String(Port));
		  if (http.begin(client, host, Port, gateway)) {  // HTTP
		    int httpCode = http.GET();
			String payload = http.getString();
			http.end();
			return payload;
		  }
		  return "";		  		  		  
}
String Michome::SendDataPOST(const char* gateway, String host, int Port, String Data)
{ 
          if((WiFi.status() != WL_CONNECTED))
              return "";
          
		  WiFiClient client;
		  HTTPClient http;
		  http.setTimeout(TimeoutConnection);
		  PortPrintln((String)"Start POST connect to " + host + ":" + String(Port));
		  if (http.begin(client, host, Port, gateway)) {  // HTTP
		    int httpCode = http.POST(Data);
			String payload = http.getString();
			http.end();
			return payload;
		  }
		  return "";
}
void Michome::SendData()
{
    SendData(ParseJson(String(type), ""));
}
void Michome::SendData(String Data)
{  
          #ifdef DurationLog
            unsigned long starttime = millis();
          #endif
          
          if((WiFi.status() != WL_CONNECTED))
              return;
		  
		  if(!MainConfig.UseGeteway){
			  Serial.println("Geteway is off");
			  return;
		  }
       
          #if !defined(NoFS) && !defined(NoAddLogSendData) && !defined(NoDataAddLogSendData)
              AddLogFile("Sending data: " + Data, false);             
          #endif         
          
		  SendDataPOST(MichomePHPPath, GetGatewayHost(), GetGatewayPort(), Data);
          
          #if !defined(NoFS) && !defined(NoAddLogSendData)
			AddLogFile("Send data OK");
          #endif

          #ifdef DurationLog
              unsigned long endtime = millis();                  
              AddLogFile("Time Sending: " + String(endtime - starttime));
          #endif
          yield();
}
String Michome::GetModule(byte num){
    if(num == 0) return id;        
    else if(num == 1) return type;
    else return "";
}
void Michome::yieldM(void){
    yield();
    running();
    yield();
}
void Michome::yieldWarn(void){
    //mdns.update();
	server.handleClient();
	ArduinoOTA.handle();
	if(IsSaveMode) return;
	dnsServer.processNextRequest();
	telnetmodule.Running();
	udpmodule.running();
	#ifndef NoFS
	if(!timeClient.update() && IsConfigured && NTPUpdate.IsTick()){					
		AddLogFile("NTP server time read error", true);		
	}
	#endif
	if(SaveModeReboot.IsTick() && IsSaveMode){
		AddLogFile("Attempt reboot for safe mode..", true);
		ESP.restart();
	}
}
Logger Michome::GetLogger()
{
      return Logger(GetHost, MainConfig.Geteway);
}
String Michome::Split(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
String Michome::ParseJson(String type, String data){
      #if defined(WriteDataToFile) && !defined(NoFS)
        DataFile.AddTextToFile("Sending data ("+type+"): " + (data == "" ? "none data for parsing" : data));
      #endif
              
      String temp = "";
      temp += "{";
      temp += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
      temp += "\"rssi\":\"" + String(FirVersion) + "\",";
      temp += "\"firmware\":\"" + String(WiFi.RSSI()) + "\",";
      temp += "\"secretkey\":\"" + sha1(String("MICHoMeMoDuLe")) + "\",";
      temp += "\"secret\":\"" + String("MICHoMeMoDuLeORIGINALFIRMWARE") + "\",";
	  #ifdef TimeSending
            temp += "\"runningtime\":\"" + String(millis()) + "\",";
      #endif     
      temp += "\"type\":";
      temp += "\"" + type + "\",";
      if(IsStr(type, StudioLight) || IsStr(type, "LightStudio")){    
          temp += "\"data\":{";
          temp += "\"status\": \"" + String("OK") + "\"}";
      }
      else if(IsStr(type, Informetr)){    
          temp += "\"data\":{";
          temp += "\"data\": \"" + String("GetData") + "\"}";
      }
      else if(IsStr(type, Logs)){    
          temp += "\"data\":{";
          temp += "\"log\": \"" + String("On Running") + "\"}";
      }
      else if(IsStr(type, HDC1080md)){    
          temp += "\"data\":{";
          temp += "\"temper\": \"" + Split(data, ';', 0) + "\",";
          temp += "\"humm\": \"" + Split(data, ';', 1) + "\"}";
      }
      else if(IsStr(type, Termometr)){    
          temp += "\"data\":{";
          temp += "\"temper\": \"" + data + "\"}";
      }
      else if(IsStr(type, Msinfoo)){    
          temp += "\"data\":{";
          temp += "\"davlen\": \"" + Split(data, ';', 0) + "\",";
          temp += "\"temperbmp\": \"" + Split(data, ';', 1) + "\",";
          temp += "\"visot\": \"" + Split(data, ';', 2) + "\",";
          temp += "\"temper\": \"" + Split(data, ';', 3) + "\",";
          temp += "\"humm\": \"" + Split(data, ';', 4) + "\" }";
      }
      else if(IsStr(type, Initialization)){    
          temp += "\"data\":{";
          temp += "\"type\": \"" + String(Michome::type) + "\",";
          temp += "\"id\": \"" + String(Michome::id) + "\" }";
      }
      else if(IsStr(type, ButtonData)){    
          temp += "\"data\":{";
          temp += "\"status\": \"" + data + "\" }";
      }
      else{
          temp += "\"data\":\"" + data + "\"";
      }
      temp += "}         \r\n";
      return temp;
}