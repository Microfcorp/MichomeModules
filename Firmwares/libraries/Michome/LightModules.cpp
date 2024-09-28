#include "LightModules.h"
#ifdef EffectsON
#include "LMEffects/effects.h"
#endif
//
// конструктор - вызывается всегда при создании экземпляра класса LightModules
//
LightModules::LightModules(Michome *m)
{
    gtw = m;
	(*gtw).SetOptionFirmware(LightModule, true);
	telnLM = &(*gtw).GetTelnet();
	udpLM = &(*gtw).GetUDP();
	#ifdef EffectsON					
		InitEffects();
	#endif
}

void LightModules::AddPin(LightPin Pin){
	uint8_t NWP[NotWorkPinCount] = {NotWorkPin};
	for(uint8_t i = 0; i < NotWorkPinCount; i++){
		if(Pin.Pin == NWP[i]){
			(*gtw).AddLogFile("Pin " + (String)Pin.Pin + " is not work for LightModule");
			(*gtw).AddError(F(__FILE__), F("LightModules::AddPin"), ("Pin " + (String)Pin.Pin + " is not work for LightModule").c_str());
			return;
		}
	}
    Pins.add(Pin);
}

String LightModules::GetPins(){
    String tmp = "";
    for(int i = 0; i < Pins.size(); i++){
        tmp += (String)i + "(" + Pins.get(i).Pin + ") - " + String(Pins.get(i).Type == 0 ? "Relay" : "PWM") + " - " + String(Pins.get(i).value) + " - " + String(Pins.get(i).Reverse ? "Reverse" : "Normal") + "<br />";
    }
    return tmp;
}

/*void LightModules::RunData(LightData LD){
    
}*/

void LightModules::SetLight(LightPin &pin, int brith){
	
	pin.value = pin.ValidateValue(brith);
	
    if(pin.Type == PWM)
		#if defined(EnableCRT)			
			analogWrite(pin.Pin, getBrightCRT(pin.ConvertValue(pin.value)));
		#else
			analogWrite(pin.Pin, pin.ConvertValue(pin.value));
		#endif
    else if (pin.Type == Relay)
        digitalWrite(pin.Pin, (pin.ConvertValue(pin.value) > 0 ? HIGH : LOW));	
	
    //Надо допиливать
    if(SaveState){
        FSFiles pinstext = FSFiles("/lm"+(String)pin.Pin+".txt");
        pinstext.WriteFile((String)pin.Pin + "=" + pin.Type + "=" + pin.value);
    }
}

bool LightModules::SetLightID(byte id, int brith){
	if(brith < 0) return false;
	LightPin lp = Pins.get(id);
	if(lp.value == brith && brith != 0) return false;
	int prBR = lp.value;
	
    SetLight(lp, brith);	
	
	Pins.set(id, lp);
	for(byte i=0; i < PinStateChanged.size(); i++)
		PinStateChanged.get(i)(id, brith, prBR);
	return true;
}

void LightModules::Reverse(byte id){
	if(Pins.get(id).Type == Relay){
		SetLightID(id, (GetBrightness(id) == 0 ? 1 : 0));  
	}
	else if(Pins.get(id).Type == PWM){
		SetLightID(id, map(GetBrightness(id), MinimumBrightnes, MaximumBrightnes, MaximumBrightnes, MinimumBrightnes));  
	}
}

bool LightModules::SetLightAll(int brith){
	bool rt = false;
    for(int i = 0; i < Pins.size(); i++){
        if(SetLightID(i, brith)) rt = true; 
    }  
	return rt;
}

void LightModules::Strobo(byte pin, int col, int del){   
    col = col > 300 ? 300 : col;
    del = del > 1000 ? 1000 : del;
    bool stops = SaveState;    
    SaveState = false;
    for (int i = 0; i < col; i++) {
       SetLightID(pin, MaximumBrightnes);
       //(*gtw).yieldM();
       delay(del);
       SetLightID(pin, MinimumBrightnes);
       //(*gtw).yieldM();
       delay(del);
    }
    SaveState = stops;
}

void LightModules::StroboPro(byte pin, int col, int del, int pdel){ 
    col = col > 300 ? 300 : col;
    del = del > 1000 ? 1000 : del; 
    bool stops = SaveState;    
    SaveState = false;    
    for (int i = 0; i < col; i++) {
       SetLightID(pin, MaximumBrightnes);
       //(*gtw).yieldM();
       delay(del);
       SetLightID(pin, MinimumBrightnes);
       //(*gtw).yieldM();
       delay(pdel);
    }
    SaveState = stops;    
}

void LightModules::StroboAll(int col, int del){   
    col = col > 300 ? 300 : col;
    del = del > 1000 ? 1000 : del;
    bool stops = SaveState;    
    SaveState = false;
    for (int i = 0; i < col; i++) {
       SetLightAll(MaximumBrightnes);
       //(*gtw).yieldM();
       delay(del);
       SetLightAll(MinimumBrightnes);
       //(*gtw).yieldM();
       delay(del);
    }   
}

void LightModules::StroboAllPro(int col, int del, int pdel){   
    col = col > 300 ? 300 : col;
    del = del > 1000 ? 1000 : del;
    bool stops = SaveState;    
    SaveState = false;
    for (int i = 0; i < col; i++) {
       SetLightAll(MaximumBrightnes);
       //(*gtw).yieldM();
       delay(del);
       SetLightAll(MinimumBrightnes);
       //(*gtw).yieldM();
       delay(pdel);
    } 
    SaveState = stops;    
}

/*String LightModules::JSONParse(String text){
    //Serial.println("jsonp");
    String tmp = "";
      DynamicJsonBuffer jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(text);
      int leg = root["Params"].size();
      int delays = 0;
      tmp += "Name = " + root["name"].as<String>() + "<br />";
      for (int i = 0; i < leg; i++) {
        //tmp += root["Params"][i]["name"].as<String>() + "<br />";
        if (root["Params"][i]["name"].as<String>() == "playmusic") {
          //http://192.168.1.42:8080/jsonrpc?request={%22jsonrpc%22:%222.0%22,%22id%22:%221%22,%22method%22:%22Player.Open%22,%22params%22:{%22item%22:{%22file%22:%22'+file+'%22}}}
          //(*gtw).SendDataGET("/jsonrpc?request={\"jsonrpc\":\"2.0\",\"id\":\"1\",\"method\":\"Player.Open\",\"params\":{\"item\":{\"file\":\"" + root["Params"][i]["file"].as<String>() + "\"}}}", "192.168.1.42", 8080);
          delays = 0;
        }
        else if (root["Params"][i]["name"].as<String>() == "setlight") {
          SetLightID(root["Params"][i]["pin"].as<int>(), root["Params"][i]["brightness"].as<int>());
          delays = 0;
          //logg.Log((String)i + " Setlight");
        }
        else if (root["Params"][i]["name"].as<String>() == "setlightall") {
          SetLightAll(root["Params"][i]["brightness"].as<int>());
          delays = 0;
          //logg.Log((String)i + " Setlight");
        }
        else if (root["Params"][i]["name"].as<String>() == "strobo") {
          int col = root["Params"][i]["col"];
          Strobo(root["Params"][i]["pin"].as<int>(), col, root["Params"][i]["times"].as<int>());
          delays = col * (root["Params"][i]["times"].as<int>() * 2);
          //logg.Log((String)i + " strobo");
        }
        else if (root["Params"][i]["name"].as<String>() == "strobopro") {
          int col = root["Params"][i]["col"];
          StroboPro(root["Params"][i]["pin"].as<int>(), col, root["Params"][i]["times"].as<int>(), root["Params"][i]["nostrob"].as<int>());
          delays = col * ((root["Params"][i]["times"].as<int>() + root["Params"][i]["nostrob"].as<int>()) * 2);
          //logg.Log((String)i + " strobo");
        }
        else if (root["Params"][i]["name"].as<String>() == "stroboall") {
          int col = root["Params"][i]["col"];
          StroboAll(col, root["Params"][i]["times"].as<int>());
          delays = col * (root["Params"][i]["times"].as<int>() * 2);
          //logg.Log((String)i + " stroboall");
        }
        else if (root["Params"][i]["name"].as<String>() == "stroboallpro") {
          int col = root["Params"][i]["col"];
          StroboAllPro(col, root["Params"][i]["times"].as<int>(), root["Params"][i]["nostrob"].as<int>());
          delays = col * ((root["Params"][i]["times"].as<int>() + root["Params"][i]["nostrob"].as<int>()) * 2);
          //logg.Log((String)i + " stroboall");
        }
        else if (root["Params"][i]["name"].as<String>() == "sleep") {
          //logg.Log((String)i + " sleep");
          //delay((root["Params"][i]["times"].as<float>() * 1000) - delays);
        }
      }
      return tmp;
      //logg.Log(tmp);
}*/
int LightModules::StartFade(FadeData data){
    for(int i = 0; i < Fades.size(); i++){
        if(GetFade(i).Pin == data.Pin) StopFade(i);
    }
    countfade++;
    data.IsRun = true;    
    Fades.add(data);
    SetLightID(data.Pin, data.MinV);
}
FadeData LightModules::GetFade(int id){
    return Fades.get(id);
}
void LightModules::StopFade(int id){
    Fades.remove(id);
    countfade--;
}
void LightModules::SetFade(int id, FadeData fd){
    Fades.set(id, fd);
}
void LightModules::StopAllFade(){
    for(int i = 0; i < Fades.size(); i++){
        FadeData fd = GetFade(i);
        fd.IsRun = false;
        SetFade(i, fd);
    }
}

bool LightModules::AddBuferState(byte pin, int brith){
    for(int i = 0; i < Bufers.size(); i++){
        BufferData bf = Bufers.get(i);
        if(bf.Pin == pin){
            Bufers.set(i, {pin, brith});
            return true;
        }
    }
    Bufers.add({pin, brith});
    return false;
}

void LightModules::RunBuffer(){
	#ifdef EffectsON
		if(Bufers.size() > 0)
			StopAllEffect();
	#endif
    for(int i = 0; i < Bufers.size(); i++){
        BufferData bf = Bufers.get(i); 
        SetLightID(bf.Pin, bf.Brig);
    }
	Bufers.clear();
}

int LightModules::GetBrightness(byte pinid){
    return Pins.get(pinid).value;
}

void LightModules::RunScript(byte scriptID){
	if(scriptID < Scripts.size())
		if(Scripts.get(scriptID).Enable)
			TelnetRun(String(Scripts.get(scriptID).Script));
}
#ifdef EffectsON
void LightModules::RunEffect(byte effectID){
	if(effectID < Effects.size()){
		for(int i = 0; i < Effects.size(); i++){
			if(i == effectID) continue;
			Effect ef = Effects.get(i);
			ef.CurState = false;
			Effects.set(i, ef);
		}			
		Effect ef = Effects.get(effectID);
		if(ef.Enable){
			ef.CurState = true;
		}
		Effects.set(effectID, ef);
	}
}

void LightModules::StopAllEffect(){
	for(int i = 0; i < Effects.size(); i++){
		Effect ef = Effects.get(i);
		ef.CurState = false;
		Effects.set(i, ef);
	}			
}
#endif
void LightModules::Load(){
	String rd = fstext.ReadFile();
    int countQ = (*gtw).Split(rd, '|', 0).toInt();
    String data = (*gtw).Split(rd, '|', 1);
    for(int i = 0; i < countQ; i++){
        String str = (*gtw).Split(data, '!', i);
        LightScript qq;
		qq.Enable = (*gtw).Split(str, '_', 0) == "1";
		qq.Name = (*gtw).Split(str, '_', 1);
		qq.Script = (*gtw).Split(str, '_', 2);
		qq.Script.replace("!","");
        Scripts.add(qq);
    }
	
	#ifdef EffectsON
		rd = efdata.ReadFile();
		countQ = (*gtw).Split(rd, '|', 0).toInt();
		data = (*gtw).Split(rd, '|', 1);
		for(int i = 0; i < countQ; i++){
			String str = (*gtw).Split(data, '!', i);
			Effect ef = Effects.get(i);
			//ef.Enable = (*gtw).Split(str, '@', 0) == "1";
			ef.Enable = IsStr(str, "1");
			//String params = (*gtw).Split(str, '@', 1);
			//int paramsCount = (*gtw).Split(params, ':', 0).toInt();
			//String paramsData = (*gtw).Split(params, ':', 1);
			//for(int a = 0; a < paramsCount; a++){
			//	String parName = (*gtw).Split(paramsData, '=', 0);
			//	String parValue = (*gtw).Split(paramsData, '=', 1);
			//	ef = SetEFparam(ef, parName, parValue);
			//}
			Effects.set(i, ef);
		}
	#endif
}

void LightModules::Save(){
	byte countQ = Scripts.size();
    String sb = ((String)countQ) + "|";
    for(byte i = 0; i < countQ; i++){
        LightScript em = Scripts.get(i);
        sb += (String)(em.Enable ? "1" : "0") + "_" + String(em.Name) + "_" + String(em.Script) + "!";
    }                   
    fstext.WriteFile(sb);
	
	#ifdef EffectsON
		countQ = Effects.size();
		sb = ((String)countQ) + "|";
		for(byte i = 0; i < countQ; i++){
			Effect ef = Effects.get(i);
			//sb += (String)(ef.Enable ? "1" : "0") + "@" + GenerateSaveParamsEF(i) + "!";
			//sb += (String)(ef.Enable ? "1" : "0") + "@!";
			sb += (String)(ef.Enable ? "1" : "0") + "!";
		}                   
		efdata.WriteFile(sb);
	#endif
}

void LightModules::init(){
	(*gtw).preInit();
	
    bool stops = SaveState;    
    SaveState = false;
    for(uint8_t i = 0; i < Pins.size(); i++){
        pinMode(Pins.get(i).Pin, OUTPUT); 
        SetLightID(i, 0);
    }

    if(stops){
        for(uint8_t i = 0; i < Pins.size(); i++){
            FSFiles pinstext = FSFiles("/lm"+(String)Pins.get(i).Pin+".txt");
			if(!pinstext.Exist()){
				Serial.println((String)"Pin " + (String)Pins.get(i).Pin + " is none saved value");
				continue;
			}
            String rdd = pinstext.ReadFile();
            int typepin = (*gtw).Split(rdd, '=', 1).toInt();
            int br = (*gtw).Split(rdd, '=', 2).toInt();
            SetLightID(i, br);
        }
    } 
    SaveState = stops;    
    
    ESP8266WebServer& server1 = (*gtw).GetServer();
    
      /*server1.on("/jsonget", [&]() {
         JSONParse(server1.arg(0));
        server1.send(200, "text/html", "OK");
      });*/
	  
      server1.on("/setlight", [&]() {
        ExternalSetLightID(server1.arg(0).toInt(), server1.arg(1).toInt());
        server1.send(200, "text/html", String(server1.arg(0).toInt()) + " as " + String(server1.arg(1).toInt()));
      });

      server1.on("/setlightall", [&]() {
        ExternalSetLightAll(server1.arg(0).toInt());
        server1.send(200, "text/html", "all as " + String(server1.arg(0).toInt()));
      });

      server1.on("/strobo", [&]() {
        server1.send(200, "text/html", String(server1.arg(0).toInt()) + " as " + String(server1.arg(1).toInt()));
        Strobo(server1.arg(0).toInt(), server1.arg(1).toInt(), server1.arg(2).toInt());
		//for(byte i=0; i < ExternalPinStateChanged.size(); i++)
		//	ExternalPinStateChanged.get(i)(id, brith);
      });

      server1.on("/strobopro", [&]() {
        server1.send(200, "text/html", String(server1.arg(0).toInt()) + " as " + String(server1.arg(1).toInt()) + " as " + String(server1.arg(2).toInt()) + "/" + String(server1.arg(3).toInt()));
        StroboPro(server1.arg(0).toInt(), server1.arg(1).toInt(), server1.arg(2).toInt(), server1.arg(3).toInt());
		//for(byte i=0; i < ExternalPinStateChanged.size(); i++)
		//	ExternalPinStateChanged.get(i)(id, brith);
      });

      server1.on("/stroboall", [&]() {
        server1.send(200, "text/html", String("all") + " as " + String(server1.arg(0).toInt()));
        StroboAll(server1.arg(0).toInt(), server1.arg(1).toInt());
		//for(byte i=0; i < ExternalPinStateChanged.size(); i++)
		//	ExternalPinStateChanged.get(i)(id, brith);
      });

      server1.on("/stroboallpro", [&]() {
        server1.send(200, "text/html", String("all") + " as " + String(server1.arg(0).toInt()) + " as " + String(server1.arg(1).toInt()) + "/" + String(server1.arg(2).toInt()));
        StroboAllPro(server1.arg(0).toInt(), server1.arg(1).toInt(), server1.arg(2).toInt());
		//for(byte i=0; i < ExternalPinStateChanged.size(); i++)
		//	ExternalPinStateChanged.get(i)(id, brith);
      });
      
      
      server1.on("/startfadeup", [&]() {
        server1.send(200, "text/html", "FadeUp start");
        StartFade(CreateFadeData(Up, server1.arg(1).toInt(), server1.arg(0).toInt(), server1.arg(2).toInt(), GetBrightness(server1.arg(0).toInt())));
      });     
      
      server1.on("/startfadedown", [&]() {
        server1.send(200, "text/html", "FadeDown start");
        StartFade(CreateFadeData(Down, server1.arg(1).toInt(), server1.arg(0).toInt(), GetBrightness(server1.arg(0).toInt()), server1.arg(2).toInt()));
      });
      
      server1.on("/stopfade", [&]() {
        server1.send(200, "text/html", "All fade stop");
        StopAllFade();
      });
      
      server1.on("/getfades", [&]() {
        server1.send(200, "text/html", String(Fades.size()));
      });
      
      server1.on("/getpins", [&]() {
        server1.send(200, "text/html", String(GetPins()));
      });
	  
	  server1.on("/reverse", [&]() {
		ExternalReverse(server1.arg(0).toInt());
        server1.send(200, "text/html", "Pin " + server1.arg(0) + " reversed");
      });
	  
	  server1.on("/runscript", [&]() {
		server1.send(200, "text/html", "Script " + server1.arg(0) + " running");
		delay(5);
		RunScript(server1.arg(0).toInt());       
      });
	  
	  #ifdef EffectsON
		  server1.on("/starteffect", [&]() {
			server1.send(200, "text/html", "Effect " + server1.arg(0) + " running");
			delay(5);
			RunEffect(server1.arg(0).toInt());       
		  });
		  
		  server1.on("/stopalleffect", [&]() {
			server1.send(200, "text/html", F("All Effects Stop"));
			delay(5);
			StopAllEffect();       
		  });
	  #endif
	  
	  server1.on("/lightscript", [&]() {
		if(server1.arg("type") == "show" || !server1.hasArg("type")){
			if (!server1.chunkedResponseModeStart(200, "text/html")) {
				server1.send(505, F("text/html"), F("HTTP1.1 required"));
				return;
			}
			server1.sendContent(F("<head>"));
			server1.sendContent(F("<meta charset=\"UTF-8\"><title>Настройка скриптов освещения</title>"));
			server1.sendContent(MainJS);
			server1.sendContent(Styles);
			server1.sendContent(F("</head>"));
			server1.sendContent(F("<table>"));			
            for(int i = 0; i < Scripts.size(); i++){
                LightScript tr = Scripts.get(i);
                server1.sendContent((String)"<tr><form action='/lightscript'><input name='type' type='hidden' value='save' /><input name='id' type='hidden' value='"+i+"' /><td>Состояние: <input type='checkbox' " + (tr.Enable == 1 ? "checked": "") +" name='en' /></td><td>Название <input name='Name' value='"+tr.Name+"' required/></td><td>Скрипт <textarea name='Script' maxlength='255' required placeholder='setlight;0;512\nsetlight;1;1023'>"+tr.Script+"</textarea></td><td><input type='submit' value='Сохранить' /></td><td><input type='button' onclick='postAjax(\"/runscript?p="+(String)i+"\", GET, \"\", function(d){})' value='Выполнить' /></td><td><a href='/lightscript?type=remove&id="+i+"'>Удалить</a></td></form></tr>");
            }
            server1.sendContent(F("</table><br /><a href='lightscript?type=add'>Добавить</a><br /><br /><a href='/'>Главная</a>"));
			server1.chunkedResponseFinalize();
		}
		else if(server1.arg("type") == "get"){
            if (!server1.chunkedResponseModeStart(200, "text/html")) {
				server1.send(505, F("text/html"), F("HTTP1.1 required"));
				return;
			}		
            for(int i = 0; i < Scripts.size(); i++){
                LightScript tr = Scripts.get(i);
                server1.sendContent(((String)i+"|"+(tr.Enable == 1 ? "1": "0")+"|"+tr.Name+"|"+tr.Script+"$"));
            }
			server1.chunkedResponseFinalize();
        }
		else if(server1.arg("type") == "save"){
            int ids = server1.arg("id").toInt();
            byte en = server1.arg("en") == "on"; 
            String Name = server1.arg("Name"); 
            String Scrt = server1.arg("Script");
			Scrt.replace("\r", "");
            
			LightScript qq;
			qq.Enable = en;
			qq.Name = Name;
			qq.Script = Scrt;
			Scripts.set(ids, qq);            
            Save();
            server1.send(200, "text/html", (String)"<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/lightscript?type=show' /></head>Скрипт освещения "+ids+" сохранен");
        }
        else if(server1.arg("type") == "add"){  
			LightScript qq;
			qq.Enable = false;
			qq.Script = " ";
			qq.Name = "New script";
            Scripts.add(qq);   
            Save();            
            server1.send(200, "text/html", (String)"<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/lightscript?type=show' /></head>Новый скрипт освещения добавлен");
        }
        else if(server1.arg("type") == "remove"){
            int ids = server1.arg("id").toInt();
            Scripts.remove(ids);
            Save();
            server1.send(200, "text/html", (String)"<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/lightscript?type=show' /></head>Скрипт освещения "+ids+" удален");
        }
		else{
			server1.send(401, "text/html", F("<head><meta charset=\"UTF-8\"></meta></head>Комманда не существует"));
		}
      });
      
      /*server1.on("/calculatetimesum", [&]() {
        server1.send(200, "text/html", String((server1.arg(1).toInt()*server1.arg(0).toInt())/1000) + " seconds");
      });*/
           
      server1.on("/remotepins", [&]() {
		if (!server1.chunkedResponseModeStart(200, "text/html")) {
			server1.send(505, F("text/html"), F("HTTP1.1 required"));
			return;
		}
        server1.sendContent(RussianHead("Управление выводами"));
        server1.sendContent(AJAXJs);
        server1.sendContent(Styles);
		
        for(int i = 0; i < Pins.size(); i++){
            server1.sendContent((String)"<p class=\"mainlinks\">" + i + " (" + String(Pins.get(i).Type == 0 ? "Relay" : "PWM") + ") " + String(Pins.get(i).Type == 0 ? "" : (String)"<input type='number' value=\""+(String)GetBrightness(i)+"\" maxlength='4' min='"+Pins.get(i).MinBrightnes()+"' max='"+Pins.get(i).MaxBrightnes()+"' id='pwm"+(String)(i+1)+(String)"' />") + (Pins.get(i).Type == PWM ? (String)" <a href='#' onclick=\"postAjax(\'setlight?p="+i+"&q="+String(Pins.get(i).Type == 0 ? Pins.get(i).MaxBrightnes() : "'+pwm"+(String)(i+1)+(String)".value+'")+"\', GET, \'\', function(d){}); return false;\">Применить значение</a> " : "") + "<a href='#' onclick=\"postAjax(\'setlight?p="+i+"&q="+Pins.get(i).MaxBrightnes()+"\', GET, \'\', function(d){document.getElementById('pwm"+(String)(i+1)+(String)"').value = "+Pins.get(i).MaxBrightnes()+";}); return false;\">Включить</a> " + "<a href='#' onclick=\"postAjax(\'setlight?p="+i+"&q="+Pins.get(i).MinBrightnes()+"\', GET, \'\', function(d){document.getElementById('pwm"+(String)(i+1)+(String)"').value = "+Pins.get(i).MinBrightnes()+";}); return false;\">Выключить</a> "  + "</p>");
        }
        server1.sendContent(F("<br /><a href='/'>Главная</a>"));
		server1.chunkedResponseFinalize();
      });
	  #ifdef EffectsON
	  server1.on("/effects", [&]() {
		if(server1.arg("type") == "show" || !server1.hasArg("type")){
			if (!server1.chunkedResponseModeStart(200, "text/html")) {
				server1.send(505, F("text/html"), F("HTTP1.1 required"));
				return;
			}
			server1.sendContent(F("<head>"));
			server1.sendContent(F("<meta charset=\"UTF-8\"><title>Управление эффектами</title>"));
			server1.sendContent(MainJS);
			server1.sendContent(AJAXJs);
			server1.sendContent(Styles);
			server1.sendContent(F("</head>"));
			server1.sendContent(F("<table>"));			
			for(int i = 0; i < Effects.size(); i++){
				Effect ef = Effects.get(i);
				server1.sendContent((String)"<tr><td>Включен: <input "+(String)(ef.Enable ? "checked" : "")+" onchange='var st = this.checked ? \"on\" : \"off\"; postAjax(\"/effects?type=setstate&id="+(String)i+"&en=\"+st, GET, \"\", function(d){document.location.reload();})' type='checkbox' /></td><td>"+ef.Name+"</td><td>"+ef.Desc+"</td><td>Активен: <input "+(String)(ef.CurState ? "checked" : "")+" type='checkbox' DISABLED /></td><td><input onclick='postAjax(\"/effects?type=startstop&id="+(String)i+"\", GET, \"\", function(d){document.location.reload();})' type='button' value='"+(String)(ef.CurState ? "Остановить" : "Запустить")+"' /></td>");
				//server1.sendContent(GenerateHTMLParamsEF(i));
				server1.sendContent(F("</tr>"));
			}
			server1.sendContent(F("</table><br /><br /><a href='/'>Главная</a>"));
			server1.chunkedResponseFinalize();
		}
		else if(server1.arg("type") == "get"){
			if (!server1.chunkedResponseModeStart(200, "text/html")) {
				server1.send(505, F("text/html"), F("HTTP1.1 required"));
				return;
			}		
            for(int i = 0; i < Effects.size(); i++){
                Effect ef = Effects.get(i);
                server1.sendContent(((String)i+"|"+(ef.Enable == 1 ? "1": "0")+"|"+ef.Name+"|"+ef.Desc+"|"+(ef.CurState == 1 ? "1": "0")+"$"));
            }
			server1.chunkedResponseFinalize();
		}
		else if(server1.arg("type") == "setstate"){
			int ids = server1.arg("id").toInt();
			bool en = server1.arg("en") == "on"; 
			Effect ef = Effects.get(ids);
			ef.Enable = en;
			Effects.set(ids, ef);            
            Save();
            server1.send(200, "text/html", (String)"<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/effects?type=show' /></head>Эффект "+ids+" сохранен");
		}
		/*else if(server1.arg("type") == "setparam"){
			String name = server1.arg("name");
			String value = server1.arg("value");
			byte ids = server1.arg("id").toInt(); 			
			
			Effect ef = Effects.get(ids);
			ef = SetEFparam(ef, name, value);
			Effects.set(ids, ef);            
            Save();
            server1.send(200, "text/html", (String)"<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/effects?type=show' /></head>Параметр эффекта "+ids+" сохранен");
		}*/
		else if(server1.arg("type") == "startstop"){
			int ids = server1.arg("id").toInt();
			for(int i = 0; i < Effects.size(); i++){
				if(i == ids) continue;
				Effect ef = Effects.get(i);
				ef.CurState = false;
				Effects.set(i, ef);
			}
			Effect ef = Effects.get(ids);
			ef.CurState = !ef.CurState;
			Effects.set(ids, ef);
            server1.send(200, "text/html", (String)"<head><meta charset=\"UTF-8\"><meta http-equiv='refresh' content='1;URL=/effects?type=show' /></head>Эффект "+ids+" изменен");
		}
		else{
			server1.send(401, "text/html", F("<head><meta charset=\"UTF-8\"></meta></head>Комманда не существует"));
		}
      });
	  #endif
	  
	  (*telnLM).on("setlight","Change light from pin {setlight;0;1023 pin;val}");
	  (*telnLM).on("setlightall","{setlightall;1023 val}");
	  (*telnLM).on("strobo","{strobo;2;4;100 pin;col;sleep}");
	  (*telnLM).on("strobopro","{strobopro;10;0;100;50 col;pin;sleep;sleep2}");
	  (*telnLM).on("stroboall","{stroboall;10;100 col;sleep}");
	  (*telnLM).on("stroboallpro","{stroboallpro;10;100;50 col;sleep;sleep2}");
	  (*telnLM).on("fadestart","{fadestart;10;100;50 col;sleep;sleep2}");
	  (*telnLM).on("fadestop","{fadedown;10;100;50 col;sleep;sleep2}");
	  (*telnLM).on("stopallfade","{stopallfade;}");
	  (*telnLM).on("reversepin","{reversepin;0}");
	  #ifdef EffectsON
		  (*telnLM).on("starteffect","{starteffect;0}");
		  (*telnLM).on("stopalleffect","{stopalleffect;}");
	  #endif
	  
	  (*udpLM).on(StudioLight, [&](IPAddress from, String cmd)
	  {
		    String type = (*gtw).Split(cmd, '-', 1);  
			if(type == (*gtw).GetModule(0)){
				TelnetRun((*gtw).Split(cmd, '-', 2));
				(*udpLM).SendMulticast((*udpLM).GetData_OK());
			}
			else if(type == "telnetdata"){
				TelnetRun((*gtw).Split(cmd, '-', 2));
				(*udpLM).SendMulticast((*udpLM).GetData_OK());
			}
	  });
	  
	  (*udpLM).EventStudioLight([&](IPAddress from, String cmd)
	  {
		TelnetRun(cmd);
	  });
	  
	  (*udpLM).EventLightScript([&](IPAddress from, String cmd)
	  {
		RunScript(cmd.toInt());
	  });
	  
	  Load();
	  
	  //if(SaveState)
		//SaveStateUpdate.Start();
}

void LightModules::TelnetRun(String telndq){
	byte countcmd = CountSymbols(telndq, '\n');
	for(byte a = 0; a <= countcmd; a++){
		String telnd = (*gtw).Split(telndq, '\n', a);
		telnd.toLowerCase();
		String type = (*gtw).Split(telnd, ';', 0);
        if (type == "setlight") { //setlight;0;1023 pin;val
          ExternalSetLightID(((*gtw).Split(telnd, ';', 1).toInt()), (*gtw).Split(telnd, ';', 2).toInt());
		  (*telnLM).printSucess("SetLight OK");
        }
        else if (type == "setlightall") { //setlightall;1023 val
          ExternalSetLightAll((*gtw).Split(telnd, ';', 1).toInt());
		  (*telnLM).printSucess("SetLightAll OK");
        }
        else if (type == "strobo") { //strobo;2;4;100 pin;col;sleep
          int col = (*gtw).Split(telnd, ';', 2).toInt();
          Strobo((*gtw).Split(telnd, ';', 1).toInt(), col, (*gtw).Split(telnd, ';', 3).toInt());
		  (*telnLM).printSucess("Strobo OK");
        }
        else if (type == "strobopro") { //strobopro;10;0;100;50 col;pin;sleep;sleep2
          int col = (*gtw).Split(telnd, ';', 1).toInt();
          StroboPro((*gtw).Split(telnd, ';', 1).toInt(), col, (*gtw).Split(telnd, ';', 3).toInt(), (*gtw).Split(telnd, ';', 4).toInt());
		  (*telnLM).printSucess("StroboPro OK");
		}
        else if (type == "stroboall") { //stroboall;10;100 col;sleep
          int col = (*gtw).Split(telnd, ';', 1).toInt();
          StroboAll(col, (*gtw).Split(telnd, ';', 2).toInt());
		  (*telnLM).printSucess("StroboAll OK");
        }
        else if (type == "stroboallpro") { //stroboallpro;10;100;50 col;sleep;sleep2
          int col = (*gtw).Split(telnd, ';', 1).toInt();
          StroboAllPro(col, (*gtw).Split(telnd, ';', 2).toInt(), (*gtw).Split(telnd, ';', 3).toInt());
		  (*telnLM).printSucess("StroboAllPro OK");
        }
        else if (type == "fadestart") { //fadestart;10;100;50 col;sleep;sleep2
          int col = (*gtw).Split(telnd, ';', 1).toInt();
          FadeData l1 = CreateFadeData((FadeType)(*gtw).Split(telnd, ';', 2).toInt(), (*gtw).Split(telnd, ';', 3).toInt(), (*gtw).Split(telnd, ';', 4).toInt(), (*gtw).Split(telnd, ';', 5).toInt(), (*gtw).Split(telnd, ';', 6).toInt());
          StartFade(l1);
		  (*telnLM).printSucess("FadeStart OK");
        }
        else if (type == "fadestop") { //fadedown;10;100;50 col;sleep;sleep2
          int col = (*gtw).Split(telnd, ';', 1).toInt();
          StopFade((*gtw).Split(telnd, ';', 2).toInt());
		  (*telnLM).printSucess("FadeStop OK");
        }
        else if (type == "stopallfade") { //fadestop;
          StopAllFade();
		  (*telnLM).printSucess("StopAllFade OK");
        }
		else if (type == "reversepin") { //reversepin;0
		  int pin = (*gtw).Split(telnd, ';', 1).toInt();
		  ExternalReverse(pin);
		  (*telnLM).printSucess("ReversePin OK");
        }
		#ifdef EffectsON
			else if (type == "starteffect") { //starteffect;0
			  int pin = (*gtw).Split(telnd, ';', 1).toInt();
			  RunEffect(pin);
			  (*telnLM).printSucess("StartEffect OK");
			}
			else if (type == "stopalleffect") { //stopalleffect;
			  StopAllEffect();
			  (*telnLM).printSucess("stopalleffect OK");
			}
		#endif
		else if (type == "printconsole") { //printconsole;aboba
		  String text = (*gtw).Split(telnd, ';', 1);
		  (*gtw).PortPrintln(text);
        }
		else if (type == "sleep") { //sleep;1000
		  int time = (*gtw).Split(telnd, ';', 1).toInt();
		  time = (time > 3000 ? 3000 : time);
		  delay(time);
		  (*gtw).yieldM();
		  (*gtw).PortPrintln("Sleep OK");
        }
	}
}

void LightModules::running(){
    if(TelnetEnable){              
        if((*telnLM).IsDataAvalible()){
            TelnetRun((*telnLM).GetData());
        }
    }
	
	/*if(SaveStateUpdate.IsTick()){
		if(SaveState){
			for(int i = 0; i < Pins.size(); i++){
				FSFiles pinstext = FSFiles("/lm"+(String)Pins.get(i).Pin+".txt");
				pinstext.WriteFile((String)Pins.get(i).Pin + "=" + Pins.get(i).Type + "=" + Pins.get(i).value);
			}
		}
	}*/
    
    for(int i = 0; i < Fades.size(); i++){
        FadeData fd = GetFade(i);
        
        if(fd.IsRun){
            if(millis() - fd.Interval > fd.CurMillis) {
               fd.CurMillis = millis();   // запоминаем текущее время
     
                if(fd.Type == Up)
                    fd.CurV++;
                else if(fd.Type == Down)
                    fd.CurV--;
                else if(fd.Type == Stop)
                    fd.IsRun = false;
        
               bool stops = SaveState;    
               SaveState = false; 
               SetLightID(fd.Pin, fd.CurV);
               SaveState = stops;
			   if(fd.Type == Up){
                    if(fd.CurV == fd.MaxV) fd.IsRun = false;
			   }
               else if(fd.Type == Down){
                    if(fd.CurV == fd.MinV) fd.IsRun = false;  
			   }					
               //delay(1);
               (*gtw).yieldM();
			   SetFade(i, fd);
            }     
        }                
        if(!fd.IsRun) ToDeleteFades.add(i);      
    }
	if(ToDeleteFades.size() > 0){
		for(int i = 0; i < ToDeleteFades.size(); i++){
			StopFade(ToDeleteFades.get(i));
		}	
		ToDeleteFades.clear();
	}
	#ifdef EffectsON
	if(Effects.size() > 0){
		for(int i = 0; i < Effects.size(); i++){
			Effect ef = Effects.get(i);
			if(ef.CurState){
				if(ef.Interval == 0){
					//ef.voids(&Pins, ef.EffectParams);
					ef.voids(&Pins);
				}
				else if(ef.Interval > 0 && !EFTimer.IsRun()){
					EFTimer.ChangeTime(ef.Interval);
					EFTimer.Start();
				}
				else if(ef.Interval > 0 && EFTimer.IsRun() && EFTimer.IsTick()){
					ef.voids(&Pins);
					//ef.voids(&Pins, ef.EffectParams);
				}
			}
		}
	}
	#endif
}