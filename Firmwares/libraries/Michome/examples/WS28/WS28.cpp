#include "WS28.h"

//
// конструктор - вызывается всегда при создании экземпляра класса WS28
//
WS28::WS28(Michome *m, uint16_t LedCount, uint8_t Pin)
{
    gtw = m;
	(*gtw).SetOptionFirmware(WST28, true);
	telnLM = &(*gtw).GetTelnet();
	WS28::LedCount = LedCount;
	WS28::Pin = Pin;
	//strip = {LedCount, Pin};
}


String WS28::GetPins(){
    String tmp = "";
    /*for(int i = 0; i < Pins.size(); i++){
        tmp += (String)i + "(" + Pins.get(i).Pin + ") - " + String(Pins.get(i).Type == 0 ? "Relay" : "PWM")  + "<br />";
    }*/
    return tmp;
}

/*void WS28::RunData(LightData LD){
    
}*/

void WS28::SetLight(uint16_t ledid, RgbColor color){
    strip.SetPixelColor(ledid, color);
    strip.Show();
    
    //
    /*if(SaveState){
        FSFiles pinstext = FSFiles("/ws"+(String)ledid+".txt");
        pinstext.WriteFile((String)ledid + "=" + brith);
    }*/
}

void WS28::SetLightAll(RgbColor color){    
    for(int i = 0; i < LedCount; i++){
        strip.SetPixelColor(i, color); 
    }
	strip.Show();	
}

void WS28::Strobo(uint16_t ledid, RgbColor color, int col, int del){   
    col = col > 300 ? 300 : col;
    del = del > 1000 ? 1000 : del;
    bool stops = SaveState;    
    SaveState = false;
    for (int i = 0; i < col; i++) {
       SetLight(ledid, color);
       //(*gtw).yieldM();
       delay(del);
       SetLight(ledid, color);
       //(*gtw).yieldM();
       delay(del);
    }
    SaveState = stops;
}

void WS28::StroboPro(uint16_t ledid, RgbColor color, int col, int del, int pdel){ 
    col = col > 300 ? 300 : col;
    del = del > 1000 ? 1000 : del; 
    pdel = pdel > 1000 ? 1000 : pdel; 
    bool stops = SaveState;    
    SaveState = false;    
    for (int i = 0; i < col; i++) {
       SetLight(ledid, color);
       //(*gtw).yieldM();
       delay(del);
       SetLight(ledid, color);
       //(*gtw).yieldM();
       delay(del);
    }
    SaveState = stops;    
}

void WS28::StroboAll(RgbColor color, int col, int del){   
    col = col > 300 ? 300 : col;
    del = del > 1000 ? 1000 : del;
    bool stops = SaveState;    
    SaveState = false;
    for (int i = 0; i < col; i++) {
       SetLightAll(color);
       //(*gtw).yieldM();
       delay(del);
       SetLightAll(color);
       //(*gtw).yieldM();
       delay(del);
    }   
}

void WS28::StroboAllPro(RgbColor color, int col, int del, int pdel){   
    col = col > 300 ? 300 : col;
    del = del > 1000 ? 1000 : del;
    bool stops = SaveState;    
    SaveState = false;
    for (int i = 0; i < col; i++) {
       SetLightAll(color);
       //(*gtw).yieldM();
       delay(del);
       SetLightAll(color);
       //(*gtw).yieldM();
       delay(pdel);
    } 
    SaveState = stops;    
}

/*String WS28::JSONParse(String text){
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
int WS28::StartFade(FadeData data){
    for(int i = 0; i < Fades.size(); i++){
        if(GetFade(i).Pin == data.Pin) StopFade(i);
    }
    countfade++;
    data.IsRun = true;    
    Fades.add(data);
    //SetLight(data.Pin, data.MinV);
}
FadeData WS28::GetFade(int id){
    return Fades.get(id);
}
void WS28::StopFade(int id){
    Fades.remove(id);
    countfade--;
}
void WS28::SetFade(int id, FadeData fd){
    Fades.set(id, fd);
}
void WS28::StopAllFade(){
    for(int i = 0; i < Fades.size(); i++){
        FadeData fd = GetFade(i);
        fd.IsRun = false;
        SetFade(i, fd);
    }
}

bool WS28::AddBuferState(uint16_t ledid, RgbColor color){
    for(int i = 0; i < Bufers.size(); i++){
        BufferData bf = Bufers.get(i);
        if(bf.Pin == ledid){
            Bufers.set(i, {ledid, color});
            return true;
        }
    }
    Bufers.add({ledid, color});
    return false;
}

void WS28::RunBuffer(){
    for(int i = 0; i < Bufers.size(); i++){
        BufferData bf = Bufers.get(i); 
		strip.SetPixelColor(bf.Pin, bf.value);
    }
	strip.Show();
	Bufers.clear();
}

/*int WS28::GetBrightness(byte pinid){
    return Pins.get(pinid).value;
}*/

void WS28::init(){
	Serial.flush();
	strip.Begin();
    strip.Show();
	
	/*RgbColor red(128, 0, 0);
	HslColor hslRed(red);
	
	for(int i = 0; i < 50; i++){
		strip.SetPixelColor(i, hslRed);
		strip.Show();
		delay(300);
    }*/
	
    /*bool stops = SaveState;    
    SaveState = false;

    if(stops){
        for(int i = 0; i < Pins.size(); i++){
            FSFiles pinstext = FSFiles("/lm"+(String)Pins.get(i).Pin+".txt");
            String rdd = pinstext.ReadFile();
            int typepin = (*gtw).Split(rdd, '=', 1).toInt();
            int br = (*gtw).Split(rdd, '=', 2).toInt();
            SetLightID(i, br);
        }
    } 
    SaveState = stops; */   
    
    ESP8266WebServer& server1 = (*gtw).GetServer();
    
      /*server1.on("/jsonget", [&]() {
         JSONParse(server1.arg(0));
        server1.send(200, "text/html", "OK");
      });*/

	  /*server1.on("/clearlight", [&]() {
        for(int i = 0; i < Pins.size(); i++){
            SPIFFS.remove("/lm"+(String)Pins.get(i).Pin+".txt");
        }
        server1.send(200, "text/html", "OK");
      });*/
	  
      server1.on("/setlight", [&]() {
        SetLight(server1.arg(0).toInt(), RgbColor(server1.arg("r").toInt(), server1.arg("g").toInt(), server1.arg("b").toInt()));
        server1.send(200, "text/html", String(server1.arg(0).toInt()) + " as ");
      });
	  
	  server1.on("/setlightdiap", [&]() {
		  RgbColor cl = RgbColor(server1.arg("r").toInt(), server1.arg("g").toInt(), server1.arg("b").toInt());
		  for(int i = server1.arg("s").toInt(); i < server1.arg("po").toInt(); i++){
			strip.SetPixelColor(i, cl);
		  }
		  strip.Show();		  
        server1.send(200, "text/html", String(server1.arg(0).toInt()) + " as ");
      });

      server1.on("/setlightall", [&]() {
        SetLightAll(RgbColor(server1.arg("r").toInt(), server1.arg("g").toInt(), server1.arg("b").toInt()));
        server1.send(200, "text/html", "all as ");
      });

      server1.on("/strobo", [&]() {
        server1.send(200, "text/html", String(server1.arg(0).toInt()) + " as " + String(server1.arg(1).toInt()));
        Strobo(server1.arg(0).toInt(), RgbColor(server1.arg("r").toInt(), server1.arg("g").toInt(), server1.arg("b").toInt()), server1.arg(1).toInt(), server1.arg(2).toInt());
      });

      server1.on("/strobopro", [&]() {
        server1.send(200, "text/html", String(server1.arg(0).toInt()) + " as " + String(server1.arg(1).toInt()) + " as " + String(server1.arg(2).toInt()) + "/" + String(server1.arg(3).toInt()));
        StroboPro(server1.arg(0).toInt(), RgbColor(server1.arg("r").toInt(), server1.arg("g").toInt(), server1.arg("b").toInt()), server1.arg(1).toInt(), server1.arg(2).toInt(), server1.arg(3).toInt());
      });

      server1.on("/stroboall", [&]() {
        server1.send(200, "text/html", String("all") + " as " + String(server1.arg(0).toInt()));
        StroboAll(RgbColor(server1.arg("r").toInt(), server1.arg("g").toInt(), server1.arg("b").toInt()), server1.arg(0).toInt(), server1.arg(1).toInt());
      });

      server1.on("/stroboallpro", [&]() {
        server1.send(200, "text/html", String("all") + " as " + String(server1.arg(0).toInt()) + " as " + String(server1.arg(1).toInt()) + "/" + String(server1.arg(2).toInt()));
        StroboAllPro(RgbColor(server1.arg("r").toInt(), server1.arg("g").toInt(), server1.arg("b").toInt()), server1.arg(0).toInt(), server1.arg(1).toInt(), server1.arg(2).toInt());
      });
      
      
      server1.on("/startfadeup", [&]() {
        server1.send(200, "text/html", "FadeUp start");
        StartFade(CreateFadeData(Up, server1.arg(1).toInt(), server1.arg(0).toInt(), server1.arg(2).toInt(), 0));
      });     
      
      server1.on("/startfadedown", [&]() {
        server1.send(200, "text/html", "FadeDown start");
        StartFade(CreateFadeData(Down, server1.arg(1).toInt(), server1.arg(0).toInt(), server1.arg(2).toInt(), 0));
      });
      
      server1.on("/stopfade", [&]() {
        server1.send(200, "text/html", "Sumeer stop");
        StopAllFade();
      });
      
      server1.on("/getfades", [&]() {
        server1.send(200, "text/html", String(Fades.size()));
      });     
      
      /*server1.on("/calculatetimesum", [&]() {
        server1.send(200, "text/html", String((server1.arg(1).toInt()*server1.arg(0).toInt())/1000) + " seconds");
      });*/
           
      server1.on("/remotepins", [&]() {
        String tmp = RussianHead("Управление WS281x");
		tmp += "<p>Выбор светодиодов";
		tmp += "<br /> С: <input type='range' min='0' id='ins' max='"+String(LedCount)+"'> По: <input type='range' min='0' id='outs' max='"+String(LedCount)+"'>";
		tmp += "<br /> Цвет <input type='color' id='bg' value='#ff0000'>";
		tmp += "<br /> <a href='#' onclick=\"postAjax('/setlightall?r=0&g=0&b=0', POST, '', function(d){postAjax('/setlightdiap?s='+ins.value+'&po='+outs.value+'&r='+convertColor(bg.value).r+'&g='+convertColor(bg.value).g+'&b='+convertColor(bg.value).b, POST, '', function(d){});});\">Применить</a>";
		tmp += "</p>";
        server1.send(200, "text/html", AJAXJs + ColorMan + tmp + (String)"<br /><a href='/'>Главная</a>");
      });
}

void WS28::TelnetRun(String telnd){
    String type = (*gtw).Split(telnd, ';', 0);
	type.toLowerCase();
        if (type == "setlight") { //setlight;0;1023 pin;val
          SetLight(((*gtw).Split(telnd, ';', 1).toInt()), RgbColor((*gtw).Split(telnd, ';', 2).toInt(), (*gtw).Split(telnd, ';', 3).toInt(), (*gtw).Split(telnd, ';', 4).toInt()));
		  (*telnLM).printSucess("SetLight OK");
        }
        else if (type == "setlightall") { //setlightall;1023 val
          SetLightAll(RgbColor((*gtw).Split(telnd, ';', 1).toInt(), (*gtw).Split(telnd, ';', 2).toInt(), (*gtw).Split(telnd, ';', 3).toInt()));
		  (*telnLM).printSucess("SetLightAll OK");
        }
        else if (type == "strobo") { //strobo;2;4;100 pin;col;sleep
          int col = (*gtw).Split(telnd, ';', 2).toInt();
          //Strobo(Pins.get((*gtw).Split(telnd, ';', 1).toInt()).Pin, col, (*gtw).Split(telnd, ';', 3).toInt());
		  (*telnLM).printSucess("Strobo OK");
        }
        else if (type == "strobopro") { //strobopro;10;0;100;50 col;pin;sleep;sleep2
          int col = (*gtw).Split(telnd, ';', 1).toInt();
          //StroboPro(Pins.get((*gtw).Split(telnd, ';', 1).toInt()).Pin, col, (*gtw).Split(telnd, ';', 3).toInt(), (*gtw).Split(telnd, ';', 4).toInt());
		  (*telnLM).printSucess("StroboPro OK");
		}
        else if (type == "stroboall") { //stroboall;10;100 col;sleep
          int col = (*gtw).Split(telnd, ';', 1).toInt();
          //StroboAll(col, (*gtw).Split(telnd, ';', 2).toInt());
		  (*telnLM).printSucess("StroboAll OK");
        }
        else if (type == "stroboallpro") { //stroboallpro;10;100;50 col;sleep;sleep2
          int col = (*gtw).Split(telnd, ';', 1).toInt();
          //StroboAllPro(col, (*gtw).Split(telnd, ';', 2).toInt(), (*gtw).Split(telnd, ';', 3).toInt());
		  (*telnLM).printSucess("StroboAllPro OK");
        }
        else if (type == "fadestart") { //fadestart;10;100;50 col;sleep;sleep2
          int col = (*gtw).Split(telnd, ';', 1).toInt();
          //FadeData l1 = CreateFadeData((FadeType)(*gtw).Split(telnd, ';', 2).toInt(), (*gtw).Split(telnd, ';', 3).toInt(), (*gtw).Split(telnd, ';', 4).toInt(), (*gtw).Split(telnd, ';', 5).toInt(), (*gtw).Split(telnd, ';', 6).toInt());
          //StartFade(l1);
		  (*telnLM).printSucess("FadeStart OK");
        }
        else if (type == "fadestop") { //fadedown;10;100;50 col;sleep;sleep2
          //int col = (*gtw).Split(telnd, ';', 1).toInt();
          //StopFade((*gtw).Split(telnd, ';', 2).toInt());
		  (*telnLM).printSucess("FadeStop OK");
        }
        else if (type == "stopallfade") { //fadestop;
          //StopAllFade();
		  (*telnLM).printSucess("StopAllFade OK");
        }
		else if (type == "reversepin") { //OnlyRelay //reversepin;0
		  //int pin = (*gtw).Split(telnd, ';', 1).toInt();
          //if(Pins.get(pin).Type == Relay){
			//SetLightID(pin, !digitalRead(Pins.get(pin).Pin));  
		  //}
		  (*telnLM).printSucess("ReversePin OK");
        }
}

void WS28::running(){
    if(TelnetEnable){              
        if((*telnLM).IsDataAvalible()){
            TelnetRun((*telnLM).GetData());
        }
    }
    
    /*for(int i = 0; i < countfade; i++){
        FadeData fd = GetFade(i);
        
        if(fd.IsRun){
            if(millis() - fd.Interval > fd.CurMillis) {
               fd.CurMillis = millis();   // запоминаем текущее время
     
                if(fd.Type == Up)
                    fd.CurV++;
                else if(fd.Type == Down)
                    fd.CurV--;
                else if(fd.Type == Stop)
                    StopFade(i);
        
               bool stops = SaveState;    
               SaveState = false; 
               SetLightID(fd.Pin, fd.CurV);
               SaveState = stops;
               if(fd.CurV == fd.MaxV) fd.IsRun = false;              
               //delay(1);
               (*gtw).yieldM();
            }     
        }
        SetFade(i, fd);        
        if(!fd.IsRun) StopFade(i);
        if(i >= Fades.size()) break;       
    }*/
}