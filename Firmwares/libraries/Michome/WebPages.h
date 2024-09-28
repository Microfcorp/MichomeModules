#ifndef WebPages_h
#define WebPages_h
	
    #define ColorMan F("<script>function convertColor(color) {if(color.substring(0,1) == '#') {color = color.substring(1);}var rgbColor = {};rgbColor.r = parseInt(color.substring(0,2),16);rgbColor.g = parseInt(color.substring(2,4),16);rgbColor.b = parseInt(color.substring(4),16);return rgbColor;}</script>")
    
	#define AJAXJs F("<script>var GET = 'GET'; var POST = 'POST'; var HEAD = 'HEAD';  function createXMLHttp() {if (typeof XMLHttpRequest != \"undefined\") {return new XMLHttpRequest();} else if (window.ActiveXObject) { var aVersions = [\"MSXML2.XMLHttp.5.0\",\"MSXML2.XMLHttp.4.0\",\"MSXML2.XMLHttp.3.0\",\"MSXML2.XMLHttp\",\"Microsoft.XMLHttp\"];for (var i = 0; i < aVersions.length; i++) {try {var oXmlHttp = new ActiveXObject(aVersions[i]);return oXmlHttp;} catch (oError) {}}throw new Error(\"Невозможно создать объект XMLHttp.\");}};  function postAjax(url, type, data, callback) { var oXmlHttp = createXMLHttp();var sBody = data;oXmlHttp.open(type, url, true);oXmlHttp.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");oXmlHttp.onreadystatechange = function() {if (oXmlHttp.readyState == 4) {callback(oXmlHttp.responseText);}};oXmlHttp.send(sBody);}</script>")
		
	#define MainJS F("<script>function InvertVisible(blockS){blockS.style.display = (blockS.style.display == 'none' ? 'block' : 'none');} function elementDisable(a, b){for(var i=0; i < a.children.length; i++){elementDisable(a.children[i], b);} a.disabled = b;}</script>")
	
	//#define TermometrsJS F("<script>function LoadTermometrInfo(temp, id){postAjax('/gettempinfo?nonehtml=1&id='+id, GET, '', function(d){document.getElementById(temp).title = d.replaceAll(\"<br />\", \"\\n\");});} function LoadTemperature(temp, id){postAjax('/gettemp?id='+id, GET, '', function(d){document.getElementById(temp).innerHTML = d;});} function LoadTermometrs(){postAjax('/counttemp', GET, '', function(d){for(var i = 0; i < parseInt(d); i++) { let p = document.createElement('p'); p.innerHTML = 'Температура на датчике '+i+': <i><span class=\"mainvalue\"  id=\"termsp'+i+'\"></span></i>'; document.getElementById('terms').append(p); LoadTermometrInfo('termsp'+i, i); LoadTemperature('termsp'+i, i); window.setInterval('LoadTemperature(\"termsp'+i+'\", '+i+')',15000);} })}; LoadTermometrs();</script>")
		
	#define MeteoStationJS F("<script>function LoadMeteoInfo(temp, id){postAjax('/getmeteoinfo?nonehtml=1&id='+id, GET, '', function(d){document.getElementById(temp).title = d.replaceAll(\"<br />\", \"\\n\");});} function LoadMeteo(temp, id){postAjax('/getmeteo?id='+id, GET, '', function(d){document.getElementById(temp).innerHTML = d;});} function LoadMeteos(){postAjax('/meteoenable', GET, '', function(d){if(d == '') return; var ids = d.split(','); for(var i = 0; i < ids.length; i++) { let p = document.createElement('p'); p.innerHTML = 'Значение на датчике '+i+': <i><span class=\"mainvalue\"  id=\"termsp'+i+'\"></span></i>'; document.getElementById('meteo').append(p); LoadMeteoInfo('termsp'+i, ids[i]); LoadMeteo('termsp'+i, ids[i]); window.setInterval('LoadMeteo(\"termsp'+i+'\", '+ids[i]+')',15000);} })}; LoadMeteos();</script>")
		
	#define PowerModuleJS F("<script>function LoadPower(pw, id){postAjax('/getpm?id='+id, GET, '', function(d){var dt = d.split(','); document.getElementById(pw).innerHTML = '<br />Напряжение: ' + dt[0] + ' V<br />Сила тока: ' + dt[1] + ' A<br />Мощность: ' + dt[2] + ' W';});} function LoadPoweres(){postAjax('/pmenable', GET, '', function(d){if(d == '') return; var ids = d.split(','); for(var i = 0; i < ids.length; i++) { let p = document.createElement('p'); p.innerHTML = 'Значение на датчике '+i+': <i><span class=\"mainvalue\"  id=\"powersp'+i+'\"></span></i>'; document.getElementById('power').append(p); LoadPower('powersp'+i, ids[i]); window.setInterval('LoadPower(\"powersp'+i+'\", '+ids[i]+')',2000);} })}; LoadPoweres();</script>")
		
	#define ChangeTypeJS F("<script>function changepin(a,b){ if(a.indexOf('script') != -1 || a.indexOf('ef') != -1) { document.getElementById('changer'+b).style.display = 'none';} else {document.getElementById('changer'+b).style.display = 'block'; postAjax('/getpins', GET, '', function(d) { ar = d.split('<br />'); for(var i = 0; i < ar.length; i++) { if(ar[i].trim() != '') { if(ar[i].split('-')[0].split('(')[0] == a) { if(ar[i].split('-')[1].trim() == 'PWM') { document.getElementById('maxpin'+b).max = 1023; } else if(ar[i].split('-')[1].trim() == 'Relay') { document.getElementById('maxpin'+b).max = 1; document.getElementById('maxpin'+b).value = (document.getElementById('maxpin'+b) > 1 ? 1 : document.getElementById('maxpin'+b)); } } }} })} }</script>")
		
    #define AutoChangeTime F("<script>function AutoChangeTime(){postAjax('/timemodule', GET, '', function(d){document.getElementById('timemod').innerHTML = d;}); window.setTimeout('AutoChangeTime()',1000);} AutoChangeTime();</script>")
	
    #define WebMainJS F(" \
	<script> \
	function sendData(){postAjax('/refresh', GET, '', function(d){alert(d);});}\
	function UpdateMain(){\
		postAjax('?params=get', GET, '', function(d){\
			var q = JSON.parse(d);\
			document.title = q['idModule'] + ' - Общая информация';\
			document.getElementById('timemod').innerHTML = q['time'];\
			document.getElementById('timemod').title = (q['isDS32'] == 'true' ? 'Присутствует модуль RTC DS32' : 'Модуль RTC отсутствует');\
			document.getElementById('typeModule').innerHTML = q['typeModule'];\
			document.getElementById('idModule').innerHTML = q['idModule'];\
			document.getElementById('versionModule').innerHTML = q['version'];\
			document.getElementById('uptime').innerHTML = q['uptime'];\
			document.getElementById('ssidV').innerHTML = q['ssidV'];\
			document.getElementById('ssidH').innerHTML = (q['wifiState'] == 'true' ? 'Подключен к сети:' : 'Ошибка подключения к');\
			document.getElementById('ip').innerHTML = (q['wifiState'] == 'true' ? q['ip'] : q['lip']);\
			document.getElementById('rssiV').innerHTML = (q['rssi'] + ' dBm'); \
			window.setTimeout('UpdateMain()',1000); \
		}); \
	} \
	UpdateMain(); \
	</script>")
	
	#define Styles F("<style>.mainbody { font-size: 16px; font-family: serif; } .mainlinks {font-size: 15px; font-family: cursive;} .mainvalue {font-family: monospace; color: red} .line {border-bottom: 1px solid #000;} a, a:visited {color: blue;} .curtimer {background-color: #f7e2e2; border-color: #ffdfdf;} .isdayoff {background-color: #ffed0038;}</style>")	
	
	//File: Favicon.png, Size: 415
	#define Favicon_png_len 415
	const uint8_t Favicon_png[] PROGMEM = {  
	0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 0x00, 0x00, 0x00, 0x0D, 0x49, 0x48, 0x44, 0x52, 
	0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x20, 0x08, 0x06, 0x00, 0x00, 0x00, 0x73, 0x7A, 0x7A, 
	0xF4, 0x00, 0x00, 0x00, 0x01, 0x73, 0x52, 0x47, 0x42, 0x00, 0xAE, 0xCE, 0x1C, 0xE9, 0x00, 0x00, 
	0x00, 0x04, 0x67, 0x41, 0x4D, 0x41, 0x00, 0x00, 0xB1, 0x8F, 0x0B, 0xFC, 0x61, 0x05, 0x00, 0x00, 
	0x00, 0x09, 0x70, 0x48, 0x59, 0x73, 0x00, 0x00, 0x0E, 0xC3, 0x00, 0x00, 0x0E, 0xC3, 0x01, 0xC7, 
	0x6F, 0xA8, 0x64, 0x00, 0x00, 0x01, 0x34, 0x49, 0x44, 0x41, 0x54, 0x58, 0x47, 0xED, 0x96, 0x61, 
	0x8E, 0x84, 0x20, 0x0C, 0x85, 0x51, 0xE3, 0x6F, 0x13, 0xF5, 0x1E, 0x7B, 0x1B, 0x0F, 0xEA, 0x05, 
	0x59, 0x60, 0x0B, 0x5B, 0x99, 0xB6, 0xB6, 0x35, 0xF3, 0x63, 0x92, 0xF9, 0x12, 0xA2, 0x23, 0x7D, 
	0x7D, 0x0F, 0x1C, 0x12, 0xC3, 0x97, 0x4F, 0x27, 0xC2, 0x70, 0x33, 0xC2, 0xD5, 0x4A, 0x6F, 0x1C, 
	0x87, 0x61, 0x70, 0x05, 0xF1, 0x04, 0x68, 0x46, 0xD3, 0x34, 0x85, 0x64, 0x5C, 0xEE, 0x63, 0x2C, 
	0x8F, 0xCD, 0x21, 0xFE, 0xD4, 0x7A, 0xB0, 0x41, 0xAF, 0x95, 0xE6, 0x58, 0x9E, 0x04, 0x90, 0x50, 
	0xF7, 0xF5, 0xFE, 0x07, 0x28, 0x03, 0xEB, 0x62, 0x0A, 0x9A, 0x00, 0x79, 0xD5, 0xD4, 0xCA, 0xB1, 
	0x21, 0x65, 0xCE, 0xE9, 0x2E, 0x48, 0x01, 0xA4, 0x06, 0xD4, 0x73, 0xA9, 0x96, 0x9B, 0x63, 0xB7, 
	0x0D, 0x0B, 0xFA, 0x1A, 0xB6, 0x19, 0x20, 0xD5, 0xBF, 0xF8, 0xDD, 0xBD, 0x02, 0x2A, 0xE0, 0x30, 
	0xCF, 0x33, 0xDC, 0xFE, 0x93, 0x8F, 0x64, 0x82, 0xAC, 0x87, 0xAB, 0x09, 0x71, 0xDB, 0x10, 0x8F, 
	0xEB, 0xBC, 0xA7, 0xA0, 0x92, 0x57, 0xE7, 0x5A, 0x61, 0x85, 0x13, 0xF7, 0x69, 0xBD, 0x26, 0xB7, 
	0x7D, 0xB4, 0x01, 0x2A, 0xDA, 0x20, 0x6A, 0xFD, 0xE5, 0x15, 0xEC, 0xFB, 0x9E, 0x85, 0x4D, 0xBC, 
	0x2C, 0x4B, 0x19, 0x88, 0xB8, 0x6D, 0x1B, 0xD7, 0x3C, 0xAC, 0xEB, 0xFA, 0xA2, 0x4F, 0xCF, 0xE0, 
	0x57, 0x21, 0x42, 0x4D, 0xA3, 0x4F, 0x84, 0x27, 0x2D, 0x73, 0x15, 0xB3, 0x5E, 0x2A, 0x92, 0xD0, 
	0x04, 0x90, 0x68, 0x7A, 0xEE, 0x14, 0x50, 0x06, 0x9C, 0x29, 0x85, 0x5A, 0x2F, 0x1D, 0x43, 0x2C, 
	0xB0, 0x98, 0x57, 0x54, 0x7A, 0x2E, 0x00, 0xB5, 0x95, 0xDA, 0xED, 0xCD, 0xA8, 0xF5, 0x54, 0xB2, 
	0x3B, 0xA3, 0xBB, 0xDD, 0x78, 0xAA, 0x0F, 0xE1, 0x3C, 0xCF, 0x3D, 0x5D, 0x72, 0x23, 0x3C, 0xAC, 
	0x5C, 0xF4, 0xD0, 0xD3, 0xC6, 0x38, 0x8E, 0xFD, 0xC7, 0x26, 0x6E, 0xDA, 0x43, 0xCD, 0x71, 0xB5, 
	0x3E, 0x8E, 0xE3, 0xF8, 0x49, 0x97, 0x66, 0x04, 0xE1, 0x2E, 0x03, 0x6A, 0xDE, 0x0B, 0x65, 0xEC, 
	0xFD, 0x2C, 0x7F, 0x04, 0x0A, 0xF2, 0xE5, 0x53, 0x09, 0xE1, 0x17, 0x71, 0x58, 0x63, 0x78, 0xB7, 
	0x6C, 0x0F, 0x66, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82
	};
	
	static String RussianHead(String title, String Insering = ""){
        return ("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><link rel=\"icon\" type=\"image/x-png\" href=\"favicon.ico\"><title>"+title+"</title><meta charset=\"UTF-8\">"+Insering+"</head>");
    }
	static String MetaRefresh(String point = "/", int time = 1){
        return ("<meta http-equiv='refresh' content='"+String(time)+";URL="+point+"'/>");
    }
	static String GetColorRssi(int rssi){
        if(rssi >= -50) return F("green");
        else if(rssi >= -68) return F("#358735");
        else if(rssi >= -75) return F("yellow");
        else if(rssi >= -85) return F("darkred");
        else if(rssi >= -100) return F("gray");
    }
	static String SVGRSSI(int rssi){
		String tmp = F("<?xml version=\"1.0\" ?> <svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" style=\"width: 32px; height: 19px;\"> <g>");
		if(rssi >= -100) tmp += F("<rect x=\"1.775\" y=\"13.16\" width=\"4.131\" height=\"14.312\"/>"); //первая палочка
		if(rssi >= -85) tmp += F("<rect x=\"8.648\" y=\"9.895\" width=\"4.13\" height=\"17.578\"/>"); //вторая
		if(rssi >= -75) tmp += F("<rect x=\"15.133\" y=\"5.188\" width=\"4.129\" height=\"22.285\"/>"); //третья
		if(rssi >= -65) tmp += F("<rect x=\"21.567\" y=\"0\" width=\"4.13\" height=\"27.473\"/>"); //четвертая
		tmp += F("</g></svg>");
		return tmp;
	}
	static String SVGRSSI(WIFISignal signal){
		if(signal == NoSignal){
			return F("<svg version=\"1.1\" style=\"width: 32px; height: 19px;\"></svg>");
		}
		String tmp = F("<?xml version=\"1.0\" ?> <svg version=\"1.1\" xmlns=\"http://www.w3.org/2000/svg\" style=\"width: 32px; height: 19px;\"> <g>");
		if(signal >= LowSignal) tmp += F("<rect x=\"1.775\" y=\"13.16\" width=\"4.131\" height=\"14.312\"/>"); //первая палочка
		if(signal >= MidSSignal) tmp += F("<rect x=\"8.648\" y=\"9.895\" width=\"4.13\" height=\"17.578\"/>"); //вторая
		if(signal >= MidHSignal) tmp += F("<rect x=\"15.133\" y=\"5.188\" width=\"4.129\" height=\"22.285\"/>"); //третья
		if(signal >= HighSignal) tmp += F("<rect x=\"21.567\" y=\"0\" width=\"4.13\" height=\"27.473\"/>"); //четвертая
		tmp += F("</g></svg>");
		return tmp;
	}
	static String formatFileSize(int size)
    {
        String a[] = { "KB", "MB", "GB", "TB", "PB" };
        int pos = 0;
        while (size >= 1024)
        {
            size /= 1024;
            pos++;
        }
        return (String)round(size) + " " + a[pos];
    }
	static String millisToTime(unsigned long mills){
		unsigned long rawTime = mills/1000;
		unsigned long hours = (rawTime % 86400L) / 3600;
		String hoursStr = hours < 10 ? "0" + String(hours) : String(hours);
		
		unsigned long minutes = (rawTime % 3600) / 60;
		String minuteStr = minutes < 10 ? "0" + String(minutes) : String(minutes);

		unsigned long seconds = rawTime % 60;
		String secondStr = seconds < 10 ? "0" + String(seconds) : String(seconds);

		unsigned int days = (rawTime / 86400L);
		
		return (days > 0 ? String(days) + F(" дней ") : "") + hoursStr + ":" + minuteStr + ":" + secondStr;
	}
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
    static void WebConfigurator(ESP8266WebServer *server, WIFIConfig *configmod){
        if (!(*server).chunkedResponseModeStart(200, "text/html")) {
			(*server).send(505, F("text/html"), F("HTTP1.1 required"));
			return;
		}
		(*server).sendContent(F("<!Doctype html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title>Конфигурация модуля</title><meta charset=\"UTF-8\"><script>function Change(d){if (window.getSelection) window.getSelection().anchorNode.children[0].value = d;}</script></head><body><div>"));
		(*server).sendContent(Styles);
		//(*server).sendContent(RussianHead("Сетевые настройки модуля"));
        #if !defined(NoScanWIFi)
		  (*server).sendContent(F("<h3>Доступные WIFI сети</h3>"));
          int n = WiFi.scanComplete();
          if(n > 0)
          {
            for (int i = 0; i < n; ++i)
            {
              (*server).sendContent(F("<p>"));
              (*server).sendContent("<a href=\"#\" onclick=\"Change('"+String(WiFi.SSID(i))+"'); return false;\">"+String(WiFi.SSID(i))+""+SVGRSSI(WiFi.RSSI(i))+"</a>");
              (*server).sendContent(F("</p>"));              
            }
			WiFi.scanDelete();
			WiFi.scanNetworks(true);
          }
		  else if(n == 0){
			(*server).sendContent(F("<p>Сети WIFI не найдены</p>"));
			WiFi.scanDelete();
			WiFi.scanNetworks(true);
		  }			
		  else if(n == WIFI_SCAN_RUNNING)
			(*server).sendContent(F("<p>Сканирование не завершено, обновите страницу позже</p>")); 
		  else{
			WiFi.scanNetworks(true);
			(*server).sendContent(F("<p>Сканирование не запущено</p>"));
		  }
		#else
		  (*server).sendContent(F("<h4>Автопоиск сетей WIFI отключен в данной версии ПО</h4>"));
        #endif
        (*server).sendContent(F("<div><br /><form action=\"/setconfig\" method=\"get\"><table>"));
		for(uint8_t i = 0, t = 1; i < CountWIFI; i++, t++){
			(*server).sendContent(F("<tr><td><p>SSID "));
			(*server).sendContent((String)t);
			(*server).sendContent(F(" сети: <input max='"));
			(*server).sendContent((String)WL_SSID_MAX_LENGTH);
			(*server).sendContent(F("' type=\"text\" value=\""));
			(*server).sendContent((*configmod).WIFI[i].SSID + (String)"\" id=\"ssid");
			(*server).sendContent((String)i);
			(*server).sendContent(F("\" name=\"ssid"));
			(*server).sendContent((String)i);
			(*server).sendContent(F("\"></p></td><td><p>Пароль: <input max='"));
			(*server).sendContent((String)WL_WPA_KEY_MAX_LENGTH);
			(*server).sendContent(F("' value=\""));
			(*server).sendContent((*configmod).WIFI[i].Password + (String)"\" type=\"password\" name=\"password");
			(*server).sendContent((String)i);
			(*server).sendContent(F("\"></p></td></tr>"));
		}
		
		(*server).sendContent("</table><p>WIFI стандарт: <select name='wifistandart'><option "+(String)((*configmod).WIFIType==1?"selected":"")+" value='1'>b стандарт</option><option "+(String)((*configmod).WIFIType==2?"selected":"")+" value='2'>b/g стандарт</option><option "+(String)((*configmod).WIFIType==3?"selected":"")+" value='3'>b/g/n стандарт</option></select></p>");
		(*server).sendContent((String)"<p>ID модуля: <input max='"+(String)WL_SSID_MAX_LENGTH+"' type=\"text\" value=\""+(*configmod).IDModule+"\" name=\"moduleid\"></p>");	
		(*server).sendContent((String)"<p>Имя сети Michome: <input max='"+(String)WL_SSID_MAX_LENGTH+"' type=\"text\" value=\""+(*configmod).MNetwork+"\" name=\"mnetwork\"></p>");	
		(*server).sendContent((String)"<p>Имя комнаты в сети: <input max='"+(String)WL_SSID_MAX_LENGTH+"' type=\"text\" value=\""+(*configmod).MRoom+"\" name=\"mroom\"></p>");	
		(*server).sendContent((String)"<p>Адрес шлюза: <input max='"+(String)WL_SSID_MAX_LENGTH+"' type=\"text\" value=\""+(*configmod).Geteway+"\" name=\"geteway\"></p>");	
		(*server).sendContent((String)"<p>Использовать шлюз: <input type=\"checkbox\" "+((*configmod).UseGeteway ? "checked" : "")+" name=\"usegetaway\"></p>");	
		(*server).sendContent(F("<p><input type=\"submit\" value=\"Сохранить и перезагрузить\"></p>"));	
		(*server).sendContent(F("</form>"));
		#ifdef FlashWEB
		(*server).sendContent(F("<p><a href='/updatemanager'>Обновление ПО модуля</a></p>"));
		#endif
		(*server).sendContent(F("<p><a href='/'>На главную</a></p></body></html>"));
		(*server).chunkedResponseFinalize();
    }
    static void WebMain(ESP8266WebServer *server, String type, bool IsTimers, bool SendGetaway, bool IsConnectToGateway, LinkedList<michomeError> *mErrors){
        int rssi = WiFi.RSSI();
		//id.replace("|", " ");
        
		if (!(*server).chunkedResponseModeStart(200, "text/html")) {
			(*server).send(505, F("text/html"), F("HTTP1.1 required"));
			return;
		}
		
		//Шапка
		(*server).sendContent(F("<html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"><title></title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">"));
		(*server).sendContent(AJAXJs);
		//(*server).sendContent(AutoChangeTime);
		(*server).sendContent(WebMainJS);
		(*server).sendContent(Styles);
		//if(IsStr(type, Termometr)) (*server).sendContent(TermometrsJS);
		if(IsStr(type, MeteoModule)) (*server).sendContent(MeteoStationJS);
		if(IsStr(type, PowerMods)) (*server).sendContent(PowerModuleJS);
		(*server).sendContent(F("</head><body class='mainbody'>"));
		(*server).sendContent(F("<div style='background: linear-gradient(45deg, #10ebf3, #87c7ff);'>"));
		(*server).sendContent(F("<p>Тип модуля: <span id='typeModule' class='mainvalue'></span></p>"));
		(*server).sendContent(F("<p>ID модуля: <span id='idModule' class='mainvalue'></span></p>"));
		(*server).sendContent(F("<p>Версия ПО модуля: <span id='versionModule' class='mainvalue'></span></p>"));
		(*server).sendContent(F("<div class=\"line\"></div>"));
		(*server).sendContent(F("<p>Платформа модуля: <span class='mainvalue'>")); (*server).sendContent(F(ARDUINO_BOARD)); (*server).sendContent(F("</span></p>"));
		#if defined(FLASHMODE_DOUT)
			(*server).sendContent(F("<p><span class='mainvalue'><i>Используется режим памяти DOUT с меньшим объемом</i></span></p>"));
		#endif
		(*server).sendContent("<p>Дата сборки ПО: <span class='mainvalue'>"+(String)__DATE__+" "+(String)__TIME__+"</span></p>");
		(*server).sendContent(F("<p>Версия ПО системы Michome: <span class='mainvalue'>")); (*server).sendContent(String(FVer)); (*server).sendContent(F("</span></p></div><div>"));
		
		(*server).sendContent(F("<p>Время работы: <span id='uptime' class='mainvalue'></span> <a href='/restart'>Перезагрузить модуль</a></p>"));
		(*server).sendContent(F("<p>Время на модуле: <span title='' id='timemod' class='mainvalue'></span></p></div>"));		
		if(SendGetaway){
			#if defined(CheckConnectGateway)
			(*server).sendContent("<p>Соединение со шлюзом: <span class='mainvalue'>"+(String)(IsConnectToGateway ? "установлено" : "потеряно")+"</span> <a href='#' onClick='sendData();'>Отправить данные</a></p></div>");
			#else
			(*server).sendContent(F("<p>Соединение со шлюзом: <span class='mainvalue'>проверка отключена в данной версии ПО</span></p></div>"));	
			#endif
		}
		(*server).sendContent(F("<div style='background: linear-gradient(45deg, #10ebf3, #87c7ff);'>"));
		(*server).sendContent(F("<p> <span id='ssidH'></span> <span id='ssidV' class='mainvalue'></span></p>"));
		(*server).sendContent("<p>IP адрес модуля: <span id='ip' class='mainvalue'>0.0.0.0</span></p>");
		(*server).sendContent((String)"<p>Уровень сигнала: <span id='rssiV' class='mainvalue' style='color: blue;'>0 dBm</span> "+SVGRSSI(rssi)+"</p>");		
		
		//Начало блоков данных
		//if(IsStr(type, Termometr)) (*server).sendContent(F("<div class=\"line\"></div><div id='terms'></div>"));
		if(IsStr(type, MeteoModule)) (*server).sendContent(F("<div class=\"line\"></div><div id='meteo'></div>"));
		if(IsStr(type, PowerMods)) (*server).sendContent(F("<div class=\"line\"></div><div id='power'></div>"));
		//Конец блоков данных
		
		if((IsStr(type, MeteoModule) || IsStr(type, PowerMods) || IsStr(type, Informetr)) && !SendGetaway) (*server).sendContent(F("<div class=\"line\"></div><p><h4 class='mainvalue'>Для корректной работы данного модуля, неободимо включить \"Испольнозовать шлюз в настройках модуля\"</h4></p>"));
		(*server).sendContent(F("</div>"));
		
		//Ошибки на модуле
		if((*mErrors).size() > 0) (*server).sendContent(F("<div style='background: linear-gradient(45deg, #ffafd2, #ffdada);'><p>На модуле возникли следующие ошибки:</p>"));
		for(uint8_t i = 0; i < (*mErrors).size(); i++){
			michomeError err = (*mErrors).get(i);
			(*server).sendContent((String)"<p><i><span class='mainvalue'>" + err.source + " (" + err.file + ", " + err.moduleUptime + ") - " + err.message + "</span></i></p>");
		}
		if((*mErrors).size() > 0) (*server).sendContent(F("</div>"));
		///Ошибки на модуле
		//Конец Шапки
		
		//Начало кнопок
		(*server).sendContent(F("<div><p class='mainlinks'><a href='/configurator'>Настройки модуля</a></p>"));
		(*server).sendContent(F("<div><p class='mainlinks'><a href='/timemodule?type=setting'>Настройка подсистемы времени</a></p>"));
		(*server).sendContent(F("<div><p class='mainlinks'><a href='/getlogs'>Системные логи</a></p>"));
		#ifdef WriteDataToFile
			if(SendGetaway) (*server).sendContent(F("<div><p class='mainlinks'><a href='/getdatalogs'>Журнал передаваемых на шлюз данных</a></p>"));
		#endif
		#ifdef ADCV
			(*server).sendContent(F("<div><p class='mainlinks'><a href='/getvcc'>Напряжение питания модуля</a></p>"));
		#endif
		if(IsStr(type, StudioLight)) (*server).sendContent(F("<p class='mainlinks'><a href='/getpins'>Просмотреть доступные выводы</a></p><p class='mainlinks'><a href='/remotepins'>Управление выводами</a></p>"));
		if(IsStr(type, StudioLight)) (*server).sendContent(F("<p class='mainlinks'><a href='/lightscript?type=show'>Конфигурация скриптов освещения</a></p>"));
		if(IsStr(type, StudioLight)) (*server).sendContent(F("<p class='mainlinks'><a href='/effects?type=show'>Управление эффектами освещения</a></p>"));
		if(IsTimers) (*server).sendContent(F("<p class='mainlinks'><a href='/qconfig'>Конфигурация таймеров</a></p>"));
		(*server).sendContent(F("<p class='mainlinks'><a href='/udptrigger?type=show'>Конфигурация UDP триггеров</a></p>"));
		if(IsStr(type, WS28Module)) (*server).sendContent(F("<p class='mainlinks'><a href='/remotepins'>Управление адресной лентой</a></p>"));
		if(IsStr(type, Informetr)) (*server).sendContent(F("<p class='mainlinks'><a href='/inform?type=show'>Настройка информетра</a></p>"));
		if(IsStr(type, Informetr)) (*server).sendContent(F("<p class='mainlinks'><a href='/test'>Проверка экрана информетра</a></p>"));
		if(IsStr(type, OLED)) (*server).sendContent(F("<p class='mainlinks'><a href='/oled?type=show'>Настройка OLED модуля</a></p>"));
		if(IsStr(type, "NotaMesh")) (*server).sendContent(F("<p class='mainlinks'><a href='/notamesh'>Управление гирляндой</a></p>"));
		//Конец Страницы
		(*server).sendContent(F("</div></body></html>"));
		(*server).chunkedResponseFinalize();
		return;
    }
#endif // #ifndef WebPages_h