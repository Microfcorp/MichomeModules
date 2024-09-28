#include "Michom.h"
#include "config.h"

#ifndef config_h
    #error "Error Read Config Files";     
#endif

#ifndef GetewayData_h
    #error "Error Read Geteway Data File";     
#endif

#ifdef EnableFSManager
	#include <fsbrowser.h>
#endif

ESP8266WebServer server(80);

//
// конструктор - вызывается всегда при создании экземпляра класса Michome
//
Michome::Michome(const char* _ssid, const char* _password, char* _id, const char* _type, const char* _host1, const double FirmwareVersion)
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

//
// конструктор - вызывается всегда при создании экземпляра класса Michome
//
Michome::Michome(char* _id, const char* _type, const double FirmwareVersion)
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

ESP8266WebServer& Michome::GetServer(){
    return server;
}

Telnet& Michome::GetTelnet(){
    return telnetmodule;
}

MichomeUDP& Michome::GetUDP(){
    return udpmodule;
}

#ifdef EnableExternalUnits
	ExternalUnits& Michome::GetExternalUnits(){
		return exUnit;
	}
#endif

//
// Инициализация в Setup
//
void Michome::init(bool sendGateway)
{
      _init();     
      
      int t1 = TimeoutConnection;
      TimeoutConnection = 1500;
      SendGateway(Initialization, "ok");
      TimeoutConnection = t1;
      
      if(sendGateway){
        SendGateway();
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
		#ifndef NoSerial
			Serial.begin(SerialSpeed);
			Serial.setDebugOutput(false); //Куча логов. Путти нормально их не ест
		#endif 
	
		#ifndef NoFS
			if (!SFS.begin()) {
				Serial.println(F("FS mount failed. Formating..."));
			}
			if(IsNeedReadConfig){
				MainConfig = ReadWIFIConfig();
				if(MainConfig.IDModule != "" && MainConfig.IDModule != "\0")
					id = MainConfig.IDModule;				
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
	
	#if defined(DebugConnection)
	    Serial.println(F("----------Module core is starting----------"));
    #endif				
	
    #ifndef NoFS
        AddLogFile(F("Start ESP OK"), false);
		if(IsSaveMode){
			AddLogFile(F("Safe Mode Enable"), false);
			Serial.println(F("Run on Safe Mode"));
		}
    #endif  
      
	Serial.println();

	#ifdef EnableExternalUnits
		exUnit.init();
		#ifdef EnableDS3231
		if(exUnit.isDS3231()){
			DS3231& ds32 = exUnit.GetDS3231();
			timeClient.setEpochTime(ds32.UNIXTime());
			AddLogFile(F("Load time from DS3231"), true);
		}
		#endif
	#endif
      
	  #if defined(FlashOTA)
      ArduinoOTA.setHostname(GetModule(0).c_str());     
      ArduinoOTA.onStart([&]() {
        Serial.println(F("Start"));
        #ifndef NoFS
            AddLogFile(F("Update OTA Start"), false);
        #endif
      });     
      ArduinoOTA.onEnd([&]() {
        Serial.println(F("\nEnd"));
        #ifndef NoFS
            AddLogFile(F("Update OTA Success"), false);
        #endif
      });
      ArduinoOTA.onProgress([&](unsigned int progress, unsigned int total) {
        #ifdef IsBlinkOTA
            StrobeBuildLed(10);
        #endif
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      });
      ArduinoOTA.onError([&](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
          Serial.println(F("Auth Failed"));
          #ifndef NoFS
              AddLogFile(F("Error OTA Auth Failed"), false);
          #endif
        } else if (error == OTA_BEGIN_ERROR) {
          Serial.println(F("Begin Failed"));
          #ifndef NoFS
              AddLogFile(F("Error OTA Begin Failed"), false);
          #endif
        } else if (error == OTA_CONNECT_ERROR) {
          Serial.println(F("Connect Failed"));
          #ifndef NoFS
              AddLogFile(F("Error OTA Connect Failed"), false);
          #endif
        } else if (error == OTA_RECEIVE_ERROR) {
          Serial.println(F("Receive Failed"));
          #ifndef NoFS
              AddLogFile(F("Error OTA Recive Failed"), false);
          #endif
        } else if (error == OTA_END_ERROR) {
          Serial.println(F("End Failed"));
          #ifndef NoFS
              AddLogFile(F("Error OTA End Failed"), false);
          #endif
        }
      });
      ArduinoOTA.begin(false);
	  #else
		  Serial.println(F("OTA is disable this firmware"));
      #endif	  
      
	  #if defined(DebugConnection)
        Serial.println(F("----------Module core is full loading----------"));
        Serial.println(F("----------Module michome is starting----------"));
      #endif
	  
      if(IsNeedReadConfig){                                        
           #if defined(DebugConnection)
		    Serial.println("Setting readed from " + (String)CountWIFI + " points");
		    for(int i = 0; i < CountWIFI; i++){
				if(MainConfig.WIFI[i].SSID[0] != '\0')
					Serial.println("Point "+(String)i+". SSID: "+(String)MainConfig.WIFI[i].SSID +" Password: ***"/*+(String)MainConfig.WIFI[i].Password*/);
				else
					Serial.println("None Point "+(String)i);
			}
           #endif
      }             
      
      #if defined(UsingFastStart)
		#if defined(DebugConnection)
			Serial.println(F("Fast Start is enable"));
		#endif
		if(!IsFoundSSID(WiFi.SSID())){
			WiFi.disconnect(true);
			#if defined(DebugConnection)
				Serial.println(F("Disconected from Fast Start"));
			#endif
		}
		
        wl_status_t status = WiFi.status();
		if(status == WL_DISCONNECTED || status == WL_NO_SSID_AVAIL || status == WL_IDLE_STATUS || status == WL_CONNECT_FAILED){
            ChangeWiFiMode();
        }		
      #else
        WiFi.disconnect(true);     
		ChangeWiFiMode();
		#if defined(DebugConnection)
			Serial.println(F("Fast Start is disable"));
		#endif
      #endif

      WiFi.hostname(GetModule(0).c_str());
      
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
				Serial.println(F("Connect to WIFI, please waiting..."));
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
				Serial.println();
			  #endif
		  }	  
      
		  if(wifiMulti.run() != WL_CONNECTED || !IsConfigured){
			  #ifndef NoFS
				AddLogFile(F("Error Connecting to all - WIFI"), false);
			  #endif
			  #if defined(DebugConnection)
				Serial.println(F("Error Connecting to all - WIFI"));
			  #endif			  
			  StrobeBuildLedError(8, LOW);
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
        Serial.println(F("----------Module Connection is finished----------"));
      #endif
      
      String locals = GetModule(0);
      //locals.replace('_', 'q');
      locals.toLowerCase();
      if (mdns.begin(locals.c_str(), WiFi.localIP()))
	  {
        #if defined(DebugConnection)
            Serial.println(F("MDNS Starting"));
        #endif
        mdns.addService("http", "tcp", 80);
        mdns.addService("telnet", "tcp", STANDARTTELNETPORT);
        mdns.addService("michomeudp", "udp", MICHOMEUDPPORT);
		#ifdef FlashOTA
			mdns.enableArduino(8266);
		#endif
      }
	  else{
		#if defined(DebugConnection)
            Serial.println(F("MDNS Start error"));
        #endif  
	  }
      LLMNR.begin(locals.c_str());

	  #if defined(FlashWEB)
	  server.on(F("/updatemanager"), HTTP_GET, [this](){
          server.sendHeader(F("Connection"), "close");
          server.sendHeader(F("Access-Control-Allow-Origin"), "*");
          server.send(200, F("text/html"), F("Please, select *.bin file firmware: <form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>"));
        });
      server.on(F("/update"), HTTP_POST, [this](){
          server.sendHeader(F("Connection"), "close");
          server.sendHeader(F("Access-Control-Allow-Origin"), "*");
          server.send(200, F("text/plain"), (Update.hasError())?"FAIL":"OK");
		  AddLogFile((Update.hasError())?"HTTP Update Fail":"HTTP Update OK");
          ESP.restart();
        },[this](){
          HTTPUpload& upload = server.upload();
          if(upload.status == UPLOAD_FILE_START){
            Serial.setDebugOutput(true);
            WiFiUDP::stopAll();
            Serial.printf("Update: %s\n", upload.filename.c_str());
			AddLogFile(F("HTTP Update Start"));
            uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
            if(!Update.begin(maxSketchSpace)){//start with max available size
			  AddLogFile(F("HTTP Update Fail"));
              Update.printError(Serial);
            }
          } else if(upload.status == UPLOAD_FILE_WRITE){
            if(Update.write(upload.buf, upload.currentSize) != upload.currentSize){
			  AddLogFile(F("HTTP Update Fail"));
              Update.printError(Serial);
            }
          } else if(upload.status == UPLOAD_FILE_END){
            if(Update.end(true)){ //true to set the size to the current progress
              Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
            } else {
		      AddLogFile(F("HTTP Update Fail"));
              Update.printError(Serial);
            }
            Serial.setDebugOutput(false);
          }
          yield();
        });
	  #else
		  Serial.println(F("HTTP Update is disable this firmware"));
	  #endif
	  
	  
      server.on(F("/"), [this](){
		  if(!IsSaveMode)
		  {
			if(!IsConfigured){
				GetConfigerator();
			}
			else{ 
				GetMainWeb();	
			}                        	   
		  }
		  else{
			#if defined(FlashWEB) && defined(FlashOTA)
				server.send(200, F("text/html"), F("Module is Safe Mode (<a href='/getlogs'>check log</a>). Please <a href='/restart'>/restart</a> module or update firmware on OTA or <a href='/updatemanager'>/updatemanager</a>"));  
			#elif defined(FlashOTA)
				server.send(200, F("text/html"), F("Module is Safe Mode (<a href='/getlogs'>check log</a>). Please <a href='/restart'>/restart</a> module or update firmware on OTA</a>"));
			#elif defined(FlashWEB)
				server.send(200, F("text/html"), F("Module is Safe Mode (<a href='/getlogs'>check log</a>). Please <a href='/restart'>/restart</a> module or update firmware on <a href='/updatemanager'>/updatemanager</a>"));	
			#else
				server.send(200, F("text/html"), F("Module is Safe Mode (<a href='/getlogs'>check log</a>). Please <a href='/restart'>/restart</a> module and connecting UART"));	
			#endif
		  }
      });
	  
	  server.on(F("/restart"), [this](){ 
        server.send(200, F("text/html"), F("OK"));
		AddLogFile(F("Restart from WEB"));
        ESP.reset();
      }); 

	  server.on(F("/getlogs"), [this](){ 
		FSLoger.ReadLogFileToServer(&server);
      });	

	  server.on(F("/clearlogs"), [this](){
        FSLoger.ClearLogFile();
        server.send(200, F("text/html"), RussianHead(F("Отчистка логов.."), MetaRefresh()) + "OK");    
      });	  
	  
	  if(IsSaveMode)
	  {
		  server.begin();
		  return;
	  }
      
      server.on(F("/generate_204"), [this](){  //Android captive portal. Maybe not needed. Might be handled by notFound handler.  
		#if defined(DebugConnection)
            Serial.println(F("Android Captive-portal request"));
        #endif
		GetMainWeb();
      });
      server.on(F("/fwlink"), [this](){  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.   
		#if defined(DebugConnection)
            Serial.println(F("Microsoft Captive-portal request"));
        #endif	  
		GetMainWeb();
      });        

	  server.on(F("/refresh"), [this](){ 
        server.send(200, F("text/html"), F("OK"));
		VoidRefreshData();
      });  	  

	  server.on(F("/formatingfs"), [this](){
		SFS.format();
        server.send(200, F("text/html"), F("SFS formating"));
      });
      
      server.on(F("/getmoduleinfo"), [this](){
		bool isgetawaytype = !server.hasArg("t");
        String tmpe = GetModule(0) + (isgetawaytype ? "/n" : "<br />") + GetModule(1) + (isgetawaytype ? "/n" : "<br />") + String(WiFi.RSSI()) + (isgetawaytype ? "/n" : "<br />") + String(WiFi.localIP().toString()) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(ESP.getFlashChipRealSize()) + (isgetawaytype ? "/n" : "<br />");
        #if defined(ADCV)
            tmpe += String((float)ESP.getVcc()/1024.0f) + (isgetawaytype ? "/n" : "<br />");
        #else
            tmpe += String("null") + (isgetawaytype ? "/n" : "<br />");
        #endif
		tmpe += String(ESP.getFreeHeap()) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(ESP.getFreeSketchSpace()) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(ESP.getResetReason()) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(FirVersion) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(FVer) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(__DATE__) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(__TIME__) + (isgetawaytype ? "/n" : "<br />");
        tmpe += String(__BASE_FILE__ ) + (isgetawaytype ? "/n" : "<br />");       
		#if defined(EnableFSManager) && defined(FSBrowser_h)
            tmpe += String("fsmanageron") + (isgetawaytype ? "/n" : "<br />");
        #else
            tmpe += String("fsmanagerof") + (isgetawaytype ? "/n" : "<br />");
        #endif
		tmpe += ESP.getSketchMD5() + (isgetawaytype ? "/n" : "<br />");
        server.send(200, F("text/html"), tmpe);
      });
      
      #ifdef ADCV
           server.on("/getvcc", [this](){
            server.send(200, F("text/html"), String((float)ESP.getVcc()/1024.0f));
          });
      #endif
      
      server.on(F("/setsettings"), [this](){         
        String settinga = "";
            for(int i=1; i < server.args(); i++) settinga += server.argName(i) + "=" + server.arg(i) + ";";
        SetSettings(settinga);
        server.send(200, F("text/html"), settinga);
      });     
      
      server.on(F("/getsettings"), [this](){
        server.send(200, F("text/html"), GetSetting());
      });
      
      server.on(F("/getdigital"), [this](){
        int pin = server.arg("p").toInt();
        server.send(200, F("text/html"), String(digitalRead(pin)));
      });
      
      server.on(F("/getanalog"), [this](){
        server.send(200, F("text/html"), String(analogRead(A0)));
      }); 

	  server.on(F("/favicon.ico"), [this](){
        server.send(200, "image/png", Favicon_png, Favicon_png_len);
      });	  
      
      
      server.on(F("/getconfig"), [this](){
		String tmp = F("<meta charset=\"UTF-8\">");
		for(int i = 0; i < CountWIFI; i++){
			tmp += "Point " + (String)i + ". SSID: " + (String)MainConfig.WIFI[i].SSID + " Password:" + (String)MainConfig.WIFI[i].Password + "<br />";
		}
        server.send(200, F("text/html"), tmp + "<br />Gateway: " + (String)MainConfig.Geteway + "<br />Use Gateway: " + (String)(MainConfig.UseGeteway ? "true" : "false") + "<br />ID Module: " + (String)MainConfig.IDModule + "<br />Network: " + (String)MainConfig.MNetwork + "<br />Room: " + (String)MainConfig.MRoom);
      });
      
      server.on(F("/setconfig"), [this](){
		for(uint8_t i = 0; i < CountWIFI; i++){
			String ss = server.arg("ssid"+(String)i);
			String pw = server.arg("password"+(String)i);
			memcpy(MainConfig.WIFI[i].SSID, ss.c_str(), WL_SSID_MAX_LENGTH);
			memcpy(MainConfig.WIFI[i].Password, pw.c_str(), WL_WPA_KEY_MAX_LENGTH);
		}

        byte WiFIS = server.arg("wifistandart").toInt();
        String mid = server.arg("moduleid");
        String mn = server.arg("mnetwork");
        String mr = server.arg("mroom");
        String gt = server.arg("geteway");
        bool ug = server.hasArg("usegetaway") ? (server.arg("usegetaway") == "on") : false;
		
		if(mid != String(MainConfig.IDModule)){
			AddLogFile(F("Changing module ID"));
		}
		
		mn.toCharArray(MainConfig.MNetwork, WL_SSID_MAX_LENGTH);		
		mid.toCharArray(MainConfig.IDModule, WL_SSID_MAX_LENGTH);		
		gt.toCharArray(MainConfig.Geteway, WL_SSID_MAX_LENGTH);		
		mr.toCharArray(MainConfig.MRoom, WL_SSID_MAX_LENGTH);		
		MainConfig.UseGeteway = ug;
		MainConfig.WIFIType = (WiFiPhyMode_t)WiFIS;
		
        WriteWIFIConfig();    
        
        AddLogFile(F("Config module saved"));
        server.send(200, F("text/html"), RussianHead(F("Конфигурация модуля"), MetaRefresh("/")) + "Config Saved. Module restarting..");
        yieldM();
        delay(200);
        ESP.reset();
      });
      
      server.on(F("/resetconfig"), [this](){
		WIFIConfig cf;		
		
        WriteWIFIConfig(cf);        
        IsConfigured = false;
        
        AddLogFile(F("Config Reset"));
        server.send(200, F("text/html"), RussianHead(F("Сброс настроек"), MetaRefresh("/")));
        ESP.reset();
      });
	  
	  server.on(F("/getoptionfirmware"), [this](){
		  String tmp = "";
		  for(byte i = 0; i < CountOptionFirmware; i++){
			  tmp += (String)i + " = " + (GetOptionFirmware(i) ? "true" : "false") + "<br />";
		  }
        server.send(200, F("text/html"), tmp);    
      });
      
      server.on(F("/configurator"), [this](){
		GetConfigerator();    
      });  
      
      server.on(F("/addlog"), [this](){
        String setting = server.arg("log");
        AddLogFile(setting);
        server.send(200, F("text/html"), RussianHead(F("Добавление лога"), MetaRefresh("/getlogs")));    
      });     
	  
	  #ifdef FSBrowser_h
		InitFSBrowser(&server);
	  #endif
      
      #ifdef WriteDataToFile
        server.on(F("/getdatalogs"), [this](){    
			DataFile.ReadFileToServer(&server);
		});
        
        server.on(F("/cleardatalogs"), [this](){
            DataFile.ClearFile();
            server.send(200, F("text/html"), RussianHead(F("Отчистка логов.."), MetaRefresh()) + "OK");    
        });
      #endif
	  
	  server.on(F("/timemodule"), [this](){
		if(server.arg("type") == "get" || !server.hasArg("type")){
			server.send(200, F("text/html"), timeClient.getFormattedDateTime());
		}
		else if(server.arg("type") == F("update")){
			timeClient.forceUpdate();
			server.send(200, F("text/html"), F("<meta http-equiv='refresh' content='1;URL=/' />OK"));
		}
		else if(server.arg("type") == F("setting")){
			String tmp = RussianHead(F("Настройка подсистемы времени"));
			tmp += (String)"<form action='timemodule'><input type='hidden' name='type' value='save' /><table><tr><td>NTP сервер точного времени:<td><td><input type='text' name='ntpserver' value='"+_NTPServer+"' /></td></tr><tr><td>Часовой пояс (GMT+):<td><td><input type='number' name='utco' value='"+_utcoffset/60/60+"' /></td></tr></table><input type='submit' value='Сохранить' /></form><br /><a href='/timemodule?type=update'>Запросить время</a><br /><a href='/'>Главная</a>";
			server.send(200, F("text/html"), tmp);
		}
		else if(server.arg("type") == F("save")){
			String ntps = server.arg(F("ntpserver"));
			int utco = server.arg(F("utco")).toInt()*60*60;
			TimeSettings.WriteFile(ntps + ";" + (String)utco);
			LoadNTP();
			timeClient.forceUpdate();
			server.send(200, F("text/html"), F("<meta http-equiv='refresh' content='1;URL=/' />OK"));
		}
	  });
      
      
      server.on(F("/description.xml"), HTTP_GET, [this](){
        SSDPMichome.schema(server.client());
      });
	  
	  /*server.on(F("/cds.xml"), HTTP_GET, [this](){
        server.send(200, F("text/xml"), F("<?xml version=\"1.0\"?> <scpd xmlns=\"urn:schemas-upnp-org:service-1-0\">   <specVersion>     <major>1</major>     <minor>0</minor>   </specVersion>   <actionList>     <action>     <name>SetTarget</name>       <argumentList>         <argument>           <name>newTargetValue</name>           <relatedStateVariable>Target</relatedStateVariable>           <direction>in</direction>         </argument>       </argumentList>     </action>     <action>     <name>GetTarget</name>       <argumentList>         <argument>           <name>RetTargetValue</name>           <relatedStateVariable>Target</relatedStateVariable>           <direction>out</direction>         </argument>       </argumentList>     </action>     <action>     <name>GetStatus</name>       <argumentList>         <argument>           <name>ResultStatus</name>           <relatedStateVariable>Status</relatedStateVariable>           <direction>out</direction>         </argument>       </argumentList>     </action>     </actionList>   <serviceStateTable>     <stateVariable sendEvents=\"no\">       <name>Target</name>       <dataType>boolean</dataType>       <defaultValue>0</defaultValue>     </stateVariable>     <stateVariable sendEvents=\"yes\">       <name>Status</name>       <dataType>boolean</dataType>       <defaultValue>0</defaultValue>     </stateVariable> </serviceStateTable> </scpd >"));
      });*/     
      
      server.onNotFound([this](){
        server.send(404, F("text/html"), F("Not found on Michome module"));
      });
	  
	  udpmodule.on(GetModule(0), [&](IPAddress from, String cmd){
		 if(Split(cmd, '-', 1) == F("Gettype")){
			udpmodule.SendMulticast(udpmodule.GetData_MyType());
		 }
		 else if(Split(cmd, '-', 1) == F("rssi")){
			udpmodule.SendMulticast((String)WiFi.RSSI());
		 } 
	  });
	  
	  udpmodule.EventSendGateway([&](IPAddress from, String cmd)
	  {
		SendGateway(F("UDPData"), cmd);
	  });
	  
	  udpmodule.EventSendURL([&](IPAddress from, String cmd)
	  {
		//SendGateway("UDPData", cmd);
	  });
	  
	  udpmodule.on(F("SetGatewayIP"), [&](IPAddress from, String cmd)
	  {
		String GATEWAYaddress = Split(cmd, '-', 1);
		GATEWAYaddress.toCharArray(MainConfig.Geteway, WL_SSID_MAX_LENGTH);	
		WriteWIFIConfig();
		AddLogFile(F("Config Saved from change Gateway IP for UDP"));
        ESP.reset();
	  });
	  udpmodule.on(F("SetGatewayState"), [&](IPAddress from, String cmd)
	  {
		bool GATEWAYstate = IsStr(Split(cmd, '-', 1), "on");
		MainConfig.UseGeteway = GATEWAYstate;
		WriteWIFIConfig();
		AddLogFile(F("Config Saved from change Gateway state for UDP"));
        ESP.reset();
	  });
	  
	  udpmodule.init(server);
      
      server.begin();
      dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
      dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
      
      SSDPMichome.setDeviceType(F("upnp:rootdevice"));
      SSDPMichome.setDeviceTypeExtern(F("urn:schemas-upnp-org:device:HVAC_System:1")); //HVAC_System
      SSDPMichome.setSchemaURL(F("description.xml"));
      SSDPMichome.setHTTPPort(80);
      SSDPMichome.setName(GetModule(0));
      SSDPMichome.setSerialNumber((String)ESP.getFlashChipId());
      SSDPMichome.setURL(F("/"));
      SSDPMichome.setModelName(String("MichomModule-")+GetModule(1));
      SSDPMichome.setModelNumber(GetModule(1));
      SSDPMichome.setModelURL(F("http://www.github.com/microfcorp/michome"));
      SSDPMichome.setManufacturer(F("Lexap-Dev"));
      SSDPMichome.setManufacturerURL(F("http://www.github.com/microfcorp/michome"));
      SSDPMichome.begin();
	  
	  timeClient.begin();
	  LoadNTP();	  
	  
	  telnetmodule.on((F("geterror")), (F("Listing on all modules error")), [&]() {
		  telnetmodule.printlnNIA((String)ansiEND + "Found " + String(mErrors.size()) + " errors:");
		  for(uint8_t i = 0; i < mErrors.size(); i++){
			  michomeError err = mErrors.get(i);
			  telnetmodule.printlnNIA((String)ansiREDF + err.source + " (" + err.file + ", " + err.moduleUptime + ") - " + err.message + ansiEND);
		  }
		  telnetmodule.println();
	  });
	  
	  telnetmodule.on((F("sendbuffers")), (F("Listing on gateway send buffer")), [&]() {
		  telnetmodule.printlnNIA((String)ansiEND + "Found " + String(sendBuffer.size()) + " chunks:");
		  for(uint8_t i = 0; i < sendBuffer.size(); i++){
			  sendBufferChunk chunk = sendBuffer.get(i);
			  telnetmodule.printlnNIA((String)ansiREDF + "ID: " + i + ansiEND + " Type: " + chunk.Type + " (unixTime: " + chunk.unixTime + ", upTime:" + chunk.mUptime + ", Timeout: " + chunk.Timeout + ") - " + (strlen(chunk.Data) == 0 ? (String)ansiREDF + "NOT DATA" + ansiEND : chunk.Data) + ansiEND);
		  }
		  telnetmodule.println();
	  });
	  
	  telnetmodule.on((F("sendchunk")), (F("Send selected chunk")), [&]() {
		  uint16_t id = Split(telnetmodule.read(), ';', 1).toInt();
		  if(id >= sendBuffer.size() || id < 0){
			  telnetmodule.printlnNIA((String)ansiREDF + "Selected id ("+id+") is not valid" + ansiEND);
			  telnetmodule.println();
			  return;
		  }
		  sendBufferChunk chunk = sendBuffer.get(id);
		  telnetmodule.printlnNIA((String)ansiREDF + "Selected ID: " + id + ansiEND + " Type: " + chunk.Type + " (unixTime: " + chunk.unixTime + ", upTime:" + chunk.mUptime + ", Timeout: " + chunk.Timeout + ") - " + (strlen(chunk.Data) == 0 ? (String)ansiREDF + "NOT DATA" + ansiEND : chunk.Data) + ansiEND);
		  telnetmodule.printlnNIA((String)ansiEND + "Sending to gateway...");
		  if(SendData(chunk)){
			  telnetmodule.printlnNIA((String)ansiGRNF + "Sending OK" + ansiEND);
			  sendBuffer.remove(id);
		  }
		  else{
			  telnetmodule.printlnNIA((String)ansiREDF + "Sending Error" + ansiEND);
		  }
		  telnetmodule.println();
	  });
	  
	  telnetmodule.on((F("refresh")), (F("Refresh module data")), [&]() {
		  telnetmodule.printlnNIA((String)ansiEND + "Refreshing...");
		  VoidRefreshData();
		  telnetmodule.printlnNIA((String)ansiGRNF + "OK" + ansiEND);
		  telnetmodule.println();
	  });
	  
	  #ifdef EnableExternalUnits
		  telnetmodule.on((F("scanwire")), (F("Scaning I2C interfaces")), [&]() {
			  telnetmodule.printlnNIA((String)ansiEND + "Found " + String(exUnit.WIREScan()) + " devices:");
			  for(uint8_t i = 0; i < exUnit.foundWIRE.size(); i++){
				  telnetmodule.printlnNIA((String)ansiGRNF + "Device "+String(i)+" - 0x" + String(exUnit.foundWIRE.get(i), HEX) + ansiEND);
			  }
			  telnetmodule.println();
		  });
		  
		  #ifdef Enable24Cxx
		  if(exUnit.isAT24CXX()){
			  telnetmodule.on((F("read24")), (F("Reading byte from AT24xx {read24;1}")), [&]() {
				  uint16_t addr = Split(telnetmodule.read(), ';', 1).toInt();
				  uint8_t data = exUnit.GetAT24CXX().read(addr);
				  telnetmodule.printlnNIA((String)ansiEND + String(addr, HEX) + " = " + String(data, HEX));
				  telnetmodule.println();
			  });
			  telnetmodule.on((F("write24")), (F("Wryting byte from AT24xx {write;1;5}")), [&]() {
				  String rd = telnetmodule.read();
				  uint16_t addr = Split(rd, ';', 1).toInt();
				  uint8_t data = Split(rd, ';', 2).toInt();
				  exUnit.GetAT24CXX().write(addr, data);
				  telnetmodule.printlnNIA((String)ansiEND + String(addr, HEX) + " = " + String(data, HEX));
				  telnetmodule.println();
			  });
		  }
		  #endif
	   #endif
	  
	  telnetmodule.SetID(GetModule(0));
	  telnetmodule.Init();	
      
	  WiFi.scanNetworks(true);
	  
	  pinWatch.SetPinWatcherHandler([&](uint8_t pin, bool state){
		  SendGateway(F(BUTTONPRESS), String(pin) + "=1");
	  });
	  pinWatch.init();
	  
      StrobeBuildLed(70);
      
      #ifdef UsingWDT
		  #if defined(DebugConnection)
		  	Serial.println(F("WDT Enable"));
		  #endif
          ESP.wdtDisable();
          ESP.wdtEnable(WDTO_8S);
      #endif
	  
	  #if defined(DebugConnection)
		Serial.println(F("Module is full load"));
		Serial.println();
      #endif
	  
	  NTPUpdate.Start();
	  SendChunk.Start();
}

void Michome::CreateAP(void){
	AddLogFile(F("Starting AP"), false);
    WiFi.softAP(GetModule(0), PasswordAPWIFi);
    
    IPAddress myIP = WiFi.softAPIP(); //192.168.4.1 is default
	Serial.print(F("AP IP address: "));
	Serial.println(myIP);    
}

//
// Цикл в Loop
//
void Michome::running(void)
{
	#ifdef UsingWDT
		ESP.wdtFeed();   // покормить пёсика. ХА-ХА, фраза Гайвера. Приветик)
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
		if(wifiMulti.run() == WL_CONNECTED){
			if(!IsConfigured)
				IsConfigured = true;
		}
	#endif

	yieldWarn();
}

void Michome::yieldWarn(void){
    mdns.update();
	server.handleClient();
	#ifdef FlashOTA
		ArduinoOTA.handle();
	#endif
	if(SaveModeReboot.IsTick() && IsSaveMode){
		AddLogFile(F("Attempt reboot for safe mode.."), true);
		ESP.restart();
	}
	if(IsSaveMode) return;
	dnsServer.processNextRequest();
	telnetmodule.Running();
	udpmodule.running();
	pinWatch.running();
	if(!timeClient.update() && IsConfigured){					
		AddLogFile(F("NTP server time read error"), true);		
	}
	#if defined(EnableExternalUnits) && defined(EnableDS3231)
		if(NTPUpdate.IsTick()){
			DS3231& ds32 = exUnit.GetDS3231();
			ds32.setTime(timeClient.getSeconds(),
				timeClient.getMinutes(),
				timeClient.getHours(),
				timeClient.getDayWeek(),
				timeClient.getDay(),
				timeClient.getMounth(),
				timeClient.getYear());
		}
	#endif
	if(SendChunk.IsTick()){
		UpdateSendChunk();
	}
}

void Michome::StrobeBuildLed(byte timeout){
    digitalWrite(BuiltLED, LOW);
    delay(timeout);
    digitalWrite(BuiltLED, HIGH);
}

void Michome::GetMainWeb(){
	#if defined(EnableExternalUnits)
		bool isds32 = exUnit.isDS3231();
		bool isEXUnit = true;
	#else
		bool isds32 = false;
		bool isEXUnit = false;
	#endif
	if(server.hasArg("params") && server.arg("params") == "get") //ajax request
	{
		if (!server.chunkedResponseModeStart(200, F("text/html"))) {
			server.send(505, F("text/html"), F("HTTP1.1 required"));
			AddErrorChucked(this, F("Michome::GetMainWeb(ajax)"));
			return;
		}
		server.sendContent(F("{\"type\":\"MainWeb\","));
		
		server.sendContent(F("\"typeModule\":\""));
		server.sendContent(GetModule(1));
		server.sendContent(F("\","));
		
		server.sendContent(F("\"idModule\":\""));
		server.sendContent(GetModule(0));
		server.sendContent(F("\","));
		
		server.sendContent(F("\"isDS32\":\""));
		server.sendContent(isds32 ? F("true") : F("false"));
		server.sendContent(F("\","));
		
		server.sendContent(F("\"isEXUnit\":\""));
		server.sendContent(isEXUnit ? F("true") : F("false"));
		server.sendContent(F("\","));
		
		server.sendContent(F("\"version\":\""));
		server.sendContent(String(FirVersion));
		server.sendContent(F("\","));
		
		server.sendContent(F("\"useGateway\":\""));
		server.sendContent(MainConfig.UseGeteway ? F("true") : F("false"));
		server.sendContent(F("\","));
		
		server.sendContent(F("\"rssi\":\""));
		server.sendContent(String(WiFi.RSSI()));
		server.sendContent(F("\","));
		
		server.sendContent(F("\"time\":\""));
		server.sendContent(String(timeClient.getFormattedDateTime()));
		server.sendContent(F("\","));
		
		server.sendContent(F("\"uptime\":\""));
		server.sendContent(millisToTime(millis()));
		server.sendContent(F("\","));
		
		server.sendContent(F("\"ssidV\":\""));
		server.sendContent(WiFi.SSID());
		server.sendContent(F("\","));
		
		server.sendContent(F("\"ip\":\""));
		server.sendContent(WiFi.localIP().toString());
		server.sendContent(F("\","));
		
		server.sendContent(F("\"lip\":\""));
		server.sendContent(F(LocalIPAP));
		server.sendContent(F("\","));
		
		server.sendContent(F("\"wifiState\":\""));
		server.sendContent(WiFi.status() != WL_CONNECTED ? F("false") : F("true"));
		server.sendContent(F("\""));
		
		server.sendContent(F("}"));
		server.chunkedResponseFinalize();
	}
	else
		WebMain(&server, GetModule(1), GetOptionFirmware(TimerLightModules), MainConfig.UseGeteway, (WiFi.status() == WL_CONNECTED ? CheckConnectToGateway() : false), &mErrors);
}

void Michome::GetConfigerator(){
    WebConfigurator(&GetServer(), &MainConfig);
}

void Michome::ChangeWiFiMode(){
	if (WiFi.getMode() != WIFIMode)
    {
        WiFi.mode(WIFIMode);
        wifi_set_sleep_type(NONE_SLEEP_T); //LIGHT_SLEEP_T and MODE_SLEEP_T
        delay(10);
    }
	WiFi.setPhyMode(MainConfig.WIFIType);
	WiFi.setOutputPower(20.5);
}

void Michome::StrobeBuildLedError(int counterror, int statusled){
    for(int i = 0; i < counterror; i++)
        StrobeBuildLed(30);
    digitalWrite(BuiltLED, statusled);
}

#ifndef NoFS

#pragma message NoFS

WIFIConfig Michome::ReadWIFIConfig(){      
    WIFIConfig cf;
    WIFINetwork nf;
    
    File f = SFS.open("/config.bin", "r");
    if (!f) {
        Serial.println(F("Module configuration file not found"));  //  "открыть файл не удалось"
		strncpy(nf.SSID, "\0", sizeof(nf.SSID));
		strncpy(nf.Password, "\0", sizeof(nf.Password));
		for(uint8_t i = 0; i < CountWIFI; i++)
			cf.WIFI[i] = nf;
		strncpy(cf.Geteway, "\0", sizeof(cf.Geteway));
		strncpy(cf.MNetwork, "\0", sizeof(cf.MNetwork));
		strncpy(cf.MRoom, "\0", sizeof(cf.MRoom));
		strncpy(cf.IDModule, id, sizeof(cf.IDModule));
        cf.UseGeteway = false;
		cf.WIFIType = DefaultWIFIStandart;
		WriteWIFIConfig(cf);
        return cf;
    }
    else{
        f.read((byte *)&cf, sizeof(cf));
		f.close();
		Serial.println(F("Module configuration file success reading"));  //  "открыть файл не удалось"		
        return cf;
    }    
}

void Michome::WriteWIFIConfig(WIFIConfig conf){  
    File f = SFS.open("/config.bin", "w");
    if (!f) {
        Serial.println(F("Error open config.bin for write"));  //  "открыть файл не удалось"
		AddError(F(__FILE__), F("Michome::WriteWIFIConfig(WIFIConfig)"), F("Error open config.bin for write"));
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
	
	return SendDataGET((String)MichomePHPPath+"?ischeckconnect=1", GetGatewayHost(), GetGatewayPort()) != "";
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
	http.end();
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
	http.end();
	return "";
}
bool Michome::SendGateway()
{
	return SendGateway(GetModule(0), "");
}
bool Michome::SendGateway(String type, String data)
{
    sendBufferChunk chunk;
	chunk.unixTime = timeClient.getEpochTime();
	chunk.mUptime = millis();
	chunk.Timeout = TimeoutConnection;
	type.toCharArray(chunk.Type, WL_SSID_MAX_LENGTH);
	data.toCharArray(chunk.Data, MAX_SENDDATA_LENGTH);
    return SendGateway(chunk);
}
bool Michome::SendGateway(sendBufferChunk chunk)
{
	if(sendBuffer.size() >= SizeSendBuffer){
		#if defined(RemoveFirstBuffer)
			AddLogFile(F("Maximum sendBuffer size. Remove last element"));
			sendBuffer.shift();
			sendBuffer.add(chunk);
			return true;
		#else
			AddLogFile(F("Maximum sendBuffer size"));
		#endif
		return false;
	}
	sendBuffer.add(chunk);
	return true;
}
void Michome::UpdateSendChunk(void)
{
	#ifndef AutoSendBuffer
		return;
	#endif
	for(uint8_t i = 0; i < sendBuffer.size(); i++)
	{
		if(SendData(sendBuffer.get(i))){
			sendBuffer.remove(i);
			i--;
		}
		else{
			//Не удалось отправить данный чанк :(
			PortPrintln((String)"Error sending chunk ID: " + i);
		}
	}
}
//Парсит в JSON и отправляет данные на сервер
bool Michome::SendData(sendBufferChunk Chunk)
{  
	#ifdef DurationLog && !defined(NoAddLogSendData)
		unsigned long starttime = millis();
	#endif
	
	bool isSend = false;

	if((WiFi.status() != WL_CONNECTED)) //Если нет wifi коннекта
		return false;

	if(!MainConfig.UseGeteway){ //Если выключена работа со шлюзом
		PortPrintln(F("Geteway is off"));
		return true;
	}

	char buffSend[550] = "\0";
	char buffData[150] = "\0";
	
	JSONData(buffData, Chunk.Type, Chunk.Data); 
	
	sprintf_P(buffSend, JSONSendData, 
		WiFi.localIP().toString().c_str(), 
		FirVersion,
		WiFi.RSSI(),
		WiFi.macAddress().c_str(),
		Chunk.Type,
		buffData);
	
	const char *b = buffSend;	

	WiFiClient client;
	
	#if defined(UseHTTPLibToGateway)
		HTTPClient http;
		http.setTimeout(Chunk.Timeout);	
		if (http.begin(client, GetGatewayHost(), GetGatewayPort(), MichomePHPPath)) {  // HTTP
			http.addHeader(F("Content-Type"), F("application/json"));
			http.setUserAgent(F("MichomeModule_ESP8266"));
			http.useHTTP10(true);
			http.setReuse(false);
			http.addHeader(F("Module-Serial"), (String)ESP.getFlashChipId());
			http.addHeader(F("Module-MAC"), WiFi.macAddress());
			http.addHeader(F("Module-Type"), GetModule(1));
			http.addHeader(F("Module-IP"), WiFi.localIP().toString());
			http.addHeader(F("Module-Room"), MainConfig.MRoom);
			http.addHeader(F("Module-Network"), MainConfig.MNetwork);
			http.addHeader(F("Module-Time"), String(Chunk.unixTime));
			http.addHeader(F("Module-RealTime"), String(CurrentTimeCallback()));
			http.addHeader(F("Module-ID"), GetModule(0));
			
			int httpCode = http.POST((const uint8_t*)b, strlen(buffSend));
			if(httpCode == HTTP_CODE_NOT_FOUND){
				AddLogFile(F("Gateway URL not found"));
			}
			else if(httpCode == -11){
				AddLogFile(F("Gateway Fucked error -11"));
				isSend = true;
			}
			else if(httpCode != HTTP_CODE_OK){
				AddLogFile("Gateway POST request error: " + String(httpCode));
			}
			else{
				isSend = true;
				const String& payload = http.getString();
				#if !defined(NoFS) && !defined(NoAddLogSendData)
					AddLogFile(F("Send data OK"));
				#endif
				parseGatewayResponce(payload, isSend);
			}
		}
		else{
			PortPrintln(F("Error initial HTTP request"));
		}
		http.end();	
	#else
		client.setTimeout(Chunk.Timeout);
		// Connect to the server
		if (!client.connect(GetGatewayHost(), GetGatewayPort())) {
			PortPrintln(F("Connection to server failed"));
			return false;
		}
		
		char HTTPReq[500] = "\0";
		strcpy_P(HTTPReq, HTTPHeader);

		// Send the POST request
		client.printf(HTTPReq,
				   MichomePHPPath,
				   GetGatewayHost().c_str(),
				   ESP.getFlashChipId(),
				   WiFi.macAddress().c_str(),
				   Michome::type,
				   WiFi.localIP().toString().c_str(),
				   MainConfig.MRoom,
				   MainConfig.MNetwork,
				   Chunk.unixTime,
				   CurrentTimeCallback(),
				   Michome::id,
				   strlen(buffSend));
				   
		client.print(b);

		unsigned long timeout = millis();
		while (!client.available()) {
			if (millis() - timeout > GatewayResponceTimeout) {
				AddLogFile(F("Gateway responce timeout"));
				client.stop();
				return false;
			}
		}

		timeout = millis();

		while(client.available() && millis() - timeout < 4000){
			String line = client.readStringUntil('\r');
			if(IsStr(line, F("HTTP/"))){ //http заголовок
				int httpCode = Split(line, ' ', 1).toInt();
				if(httpCode == HTTP_CODE_NOT_FOUND){
					AddLogFile(F("Gateway URL not found"));
				}
				else if(httpCode != HTTP_CODE_OK){
					AddLogFile("Gateway POST request error: " + String(httpCode));
				}
				else{
					isSend = true;
					#if !defined(NoFS) && !defined(NoAddLogSendData)
						AddLogFile(F("Send data OK"));
					#endif
				}
			}
			if(isSend && line == "\n"){
				line = client.readStringUntil('\r');
				parseGatewayResponce(line, isSend);
			}
		}

		client.stop();
	#endif

	#if defined(DurationLog) && !defined(NoAddLogSendData)
		unsigned long endtime = millis();                  
		AddLogFile("Time Sending: " + String(endtime - starttime) + " ms");
	#endif
	yield();
	return isSend;
}
void Michome::parseGatewayResponce(String resp, bool &isOK){
	//{"name":"getpost","type":"init","serverDate":4546454,"device":"192.168.0.175","data":{"settings":"SCount=2;update=600000"},"error":"none","errorCode":"0"}
	//{"name":"getpost","type":"meteostation","serverDate":4546454,"device":"192.168.0.175","data":[],"error":"none","errorCode":"0"}
	
	if(resp.length() < 60){
		AddLogFile(F("Gateway responce is small size"));
		isOK = false;
		return;
	}
    if(IsStr(resp, "php") || IsStr(resp, "PHP")){
		FSFiles phpERR = FSFiles("phpError.txt");
		phpERR.WriteFile(resp);
		#if defined(EnableFSManager)
			AddLogFile(F("PHP error on Gateway. See /fsedit?file=/phpError.txt"));
		#else
			AddLogFile(F("PHP error on Gateway (log: phpError.txt)"));
		#endif
		isOK = false;
		return;
	}	
	
	DynamicJsonDocument root(512);
	deserializeJson(root, resp);
	if(root["errorCode"].as<int>() != 0){
		AddLogFile("Gateway responce error: " + String(root["error"].as<const char*>()));
		return;
	}
	String TypeReq = String(root["type"].as<const char*>());
	if(root["data"].containsKey("settings")){
		String setting = String(root["data"]["settings"].as<const char*>());		
        SetSettings(setting);
	}
	time_t serverTime = root["serverDate"].as<time_t>();
	time_t currentTime = CurrentTimeCallback();
	if(abs(serverTime - (currentTime - timeClient.getTimeOffset())) > 3601){
		AddLogFile("Gateway and module time is not synchro. Range: " + String(abs(serverTime - currentTime)) + ". Synchronize");
		timeClient.setEpochTime(serverTime);
	}
}
void Michome::SetSettings(String strSett){
    Michome::countsetting = Split(Split(strSett, ';', 0), '=', 1).toInt();
	Michome::settings = strSett;
	IsSettingRead = true;
	AddLogFile(F("Setting read OK"));
}
String Michome::GetModule(byte num){
    if(num == 0) return String(MainConfig.IDModule);        
    else if(num == 1) return type;
    else return "";
}
void Michome::yieldM(void){
    yield();
    running();
    yield();
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
void Michome::JSONData(char *buffData, char *type, char *data){
      /*#if defined(WriteDataToFile) && !defined(NoFS)
        DataFile.AddTextToFile("Parsing data ("+type+"): " + (data == "" ? "none data for parsing" : data));
      #endif
              
      String temp = "";
      temp += "{";
      temp += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
      temp += "\"firmware\":\"" + String(FirVersion) + "\",";
      temp += "\"rssi\":\"" + String(WiFi.RSSI()) + "\",";
      temp += "\"mac\":\"" + String(WiFi.macAddress()) + "\",";
      temp += "\"secretkey\":\"" + sha1(String("MICHoMeMoDuLe")) + "\",";
      temp += "\"secret\":\"" + String("MICHoMeMoDuLeORIGINALFIRMWARE") + "\",";
	  #ifdef TimeSending
            temp += "\"runningtime\":\"" + String(millis()) + "\",";
      #endif     
      temp += "\"type\":";
      temp += "\"" + type + "\",";
      
      temp += "}         \r\n";
      return temp;*/
	  
	String temp = "";
	if(IsStr(type, StudioLight)){    
	  temp += "{";
	  temp += "\"status\":\"OK\"";
	  temp += "}";
	}
	else if(IsStr(type, Informetr)){    
	  temp += "{";
	  temp += "\"data\":\"GetData\"";
	  temp += "}";
	}
	else if(IsStr(type, Logs)){    
	  temp += "{";
	  temp += "\"log\":\"" + data + "\"";
	  temp += "}";
	}
	else if(IsStr(type, MeteoModule)){    
	  temp += "{";
	  temp += "\"meteo\":[" + data + "]";
	  temp += "}";
	}
	else if(IsStr(type, PowerMods)){    
	  temp += "{";
	  temp += "\"power\":[" + data + "]";
	  temp += "}";
	}
	else if(IsStr(type, Initialization)){    
	  temp += "{";
	  temp += "\"type\":\"" + String(Michome::type) + "\",";
	  temp += "\"id\":\"" + String(Michome::id) + "\",";
	  temp += "\"settings\":\"none\"";
	  temp += "}";
	}
	else if(IsStr(type, BUTTONPRESS)){    
	  temp += "{";
	  temp += "\"status\":\"" + data + "\"";
	  temp += "}";
	}
	else{
	  temp += "\"" + data + "\"";
	}
	temp.toCharArray(buffData, 150);
}