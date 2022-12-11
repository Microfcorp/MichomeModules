#ifndef WebPages_h
#define WebPages_h
    //static const String XNR = " function createXMLHttp() {if (typeof XMLHttpRequest != \"undefined\") {return new XMLHttpRequest();} else if (window.ActiveXObject) { var aVersions = [\"MSXML2.XMLHttp.5.0\",\"MSXML2.XMLHttp.4.0\",\"MSXML2.XMLHttp.3.0\",\"MSXML2.XMLHttp\",\"Microsoft.XMLHttp\"];for (var i = 0; i < aVersions.length; i++) {try {var oXmlHttp = new ActiveXObject(aVersions[i]);return oXmlHttp;} catch (oError) {}}throw new Error(\"Невозможно создать объект XMLHttp.\");}}; ";
	
    #define ColorMan F("<script>function convertColor(color) {if(color.substring(0,1) == '#') {color = color.substring(1);}var rgbColor = {};rgbColor.r = parseInt(color.substring(0,2),16);rgbColor.g = parseInt(color.substring(2,4),16);rgbColor.b = parseInt(color.substring(4),16);return rgbColor;}</script>")
    
	#define AJAXJs F("<script>var GET = 'GET'; var POST = 'POST'; var HEAD = 'HEAD';  function createXMLHttp() {if (typeof XMLHttpRequest != \"undefined\") {return new XMLHttpRequest();} else if (window.ActiveXObject) { var aVersions = [\"MSXML2.XMLHttp.5.0\",\"MSXML2.XMLHttp.4.0\",\"MSXML2.XMLHttp.3.0\",\"MSXML2.XMLHttp\",\"Microsoft.XMLHttp\"];for (var i = 0; i < aVersions.length; i++) {try {var oXmlHttp = new ActiveXObject(aVersions[i]);return oXmlHttp;} catch (oError) {}}throw new Error(\"Невозможно создать объект XMLHttp.\");}};  function postAjax(url, type, data, callback) { var oXmlHttp = createXMLHttp();var sBody = data;oXmlHttp.open(type, url, true);oXmlHttp.setRequestHeader(\"Content-Type\", \"application/x-www-form-urlencoded\");oXmlHttp.onreadystatechange = function() {if (oXmlHttp.readyState == 4) {callback(oXmlHttp.responseText);}};oXmlHttp.send(sBody);}</script>")
		
	#define MainJS F("<script>function InvertVisible(blockS){blockS.style.display = (blockS.style.display == 'none' ? 'block' : 'none');}</script>")
    
	//#define ChangeTypeJS F("<script>var isrel = false; function changepin(id, iq){postAjax('/getpins', GET, '', function(d){var lines = d.split('<br />'); for(var i = 0; i < lines.length; i++){var ids = parseInt(lines[i][0]); if(ids == id){if(lines[i].split('-')[1].substring(1).trim() == 'Relay'){document.getElementById('valuepin'+iq).innerHTML = 'Значение'+(isrel ? '(Релейный)' : '(PWM)')+':';}}}})}</script>")
		
	#define ChangeTypeJS F("<script>function changepin(a,b){ if(a.indexOf('script') != -1 || a.indexOf('ef') != -1) { document.getElementById('changer'+b).style.display = 'none';} else {document.getElementById('changer'+b).style.display = 'block'; postAjax('/getpins', GET, '', function(d) { ar = d.split('<br />'); for(var i = 0; i < ar.length; i++) { if(ar[i].trim() != '') { if(ar[i].split('-')[0].split('(')[0] == a) { if(ar[i].split('-')[1].trim() == 'PWM') { document.getElementById('maxpin'+b).max = 1023; } else if(ar[i].split('-')[1].trim() == 'Relay') { document.getElementById('maxpin'+b).max = 1; document.getElementById('maxpin'+b).value = (document.getElementById('maxpin'+b) > 1 ? 1 : document.getElementById('maxpin'+b)); } } }} })} }</script>")
		
	//static const String ChangeTypeJS = isrelayJS + "<script>function changepin(id, i){isrelay(id); document.getElementById('valuepin'+i).innerHTML = 'Значение'+(isrel ? '(Релейный)' : '(PWM)')+':';}</script>";
    #define AutoChangeTime F("<script>function AutoChangeTime(){postAjax('/timemodule', GET, '', function(d){timemod.innerHTML = d;}); window.setTimeout('AutoChangeTime()',1000);} AutoChangeTime();</script>")
	#define Styles F("<style>.mainbody { font-size: 16px; font-family: serif; } .mainlinks {font-size: 15px; font-family: cursive;} .mainvalue {font-family: monospace; color: red} .line {border-bottom: 1px solid #000;} a, a:visited {color: blue;} .curtimer {background-color: #f7e2e2; border-color: #ffdfdf;}</style>")
	
	static String RussianHead(String title, String Insering = ""){
        return ("<head><title>"+title+"</title><meta charset=\"UTF-8\">"+Insering+"</head>");
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
	static String millisToTime(long mills){
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
    static void WebConfigurator(ESP8266WebServer *server, String SSID0, String Password0, String SSID1, String Password1, String SSID2, String Password2, String Gateway, bool UseGetaway, byte WIFIS, String IDM){
        if (!(*server).chunkedResponseModeStart(200, "text/html")) {
			(*server).send(505, F("text/html"), F("HTTP1.1 required"));
			return;
		}
		(*server).sendContent(F("<!Doctype html><html><head><title>Конфигурация модуля</title><meta charset=\"UTF-8\"><script>function Change(d){if (window.getSelection) window.getSelection().anchorNode.children[0].value = d;}</script></head><body><div>"));
		(*server).sendContent(Styles);
		//(*server).sendContent(RussianHead("Сетевые настройки модуля"));
        #ifndef NoScanWIFi
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
			(*server).sendContent(F("<p>Обновите страницу еще раз</p>"));
		  }
        #endif
        (*server).sendContent(F("<div><br /><form action=\"/setconfig\" method=\"get\"><table>"));
		(*server).sendContent("<tr><td><p>SSID 1 сети: <input max='"+(String)WL_SSID_MAX_LENGTH+"' type=\"text\" value=\""+SSID0+"\" id=\"ssid0\" name=\"ssid0\"></p></td><td><p>Пароль 1 сети: <input max='"+(String)WL_WPA_KEY_MAX_LENGTH+"' value=\""+Password0+"\" type=\"password\" name=\"password0\"></p></td></tr>");
		(*server).sendContent("<tr><td><p>SSID 2 сети: <input max='"+(String)WL_SSID_MAX_LENGTH+"' type=\"text\" value=\""+SSID1+"\" id=\"ssid1\" name=\"ssid1\"></p></td><td><p>Пароль 2 сети: <input max='"+(String)WL_WPA_KEY_MAX_LENGTH+"' value=\""+Password1+"\" type=\"password\" name=\"password1\"></p></td></tr>");
		(*server).sendContent("<tr><td><p>SSID 3 сети: <input max='"+(String)WL_SSID_MAX_LENGTH+"' type=\"text\" value=\""+SSID2+"\" id=\"ssid2\" name=\"ssid2\"></p></td><td><p>Пароль 3 сети: <input max='"+(String)WL_WPA_KEY_MAX_LENGTH+"' value=\""+Password2+"\" type=\"password\" name=\"password2\"></p></td></tr>");
		(*server).sendContent("</table><p>WIFI стандарт: <select name='wifistandart'><option "+(String)(WIFIS==1?"selected":"")+" value='1'>b стандарт</option><option "+(String)(WIFIS==2?"selected":"")+" value='2'>b/g стандарт</option><option "+(String)(WIFIS==3?"selected":"")+" value='3'>b/g/n стандарт</option></select></p>");
		(*server).sendContent("<p>ID модуля: <input max='"+(String)WL_SSID_MAX_LENGTH+"' type=\"text\" value=\""+IDM+"\" name=\"moduleid\"></p>");	
		(*server).sendContent("<p>Адрес шлюза: <input max='"+(String)WL_SSID_MAX_LENGTH+"' type=\"text\" value=\""+Gateway+"\" name=\"geteway\"></p><p>Использовать шлюз: <input type=\"checkbox\" "+(UseGetaway ? "checked" : "")+" name=\"usegetaway\"></p><p><input type=\"submit\" value=\"Сохранить и перезагрузить\"></p></form>");	
		(*server).sendContent(F("<p><a href='/updatemanager'>Обновление ПО модуля</a></p></body></html>"));
		(*server).chunkedResponseFinalize();
    }
    static void WebMain(ESP8266WebServer *server, String type, String id, String Times, bool IsTimers, bool IsUDP, bool SendGetaway, double FIRModuleVersion, bool IsConnectToGateway){
        int rssi = WiFi.RSSI();
		//id.replace("|", " ");
        
		if (!(*server).chunkedResponseModeStart(200, "text/html")) {
			(*server).send(505, F("text/html"), F("HTTP1.1 required"));
			return;
		}
		
		//Шапка
		(*server).sendContent("<html><head><meta http-equiv='refresh' content='60;URL=/'/><title>"+id+" - Общая информация</title><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">");
		(*server).sendContent(AJAXJs);
		(*server).sendContent(AutoChangeTime);
		(*server).sendContent(Styles);
		(*server).sendContent(F("</head><body class='mainbody'>"));
		(*server).sendContent(F("<div style='background: linear-gradient(45deg, #10ebf3, #87c7ff);'>"));
		(*server).sendContent("<p>Тип модуля: <span class='mainvalue'>"+type+"</span></p>");
		(*server).sendContent("<p>ID модуля: <span class='mainvalue'>"+id+"</span></p>");
		(*server).sendContent("<p>Версия ПО модуля: <span class='mainvalue'>"+(String)FIRModuleVersion+"</span></p>");
		(*server).sendContent(F("<div class=\"line\"></div>"));
		(*server).sendContent("<p>Платформа модуля: <span class='mainvalue'>"+(String)ARDUINO_BOARD+"</span></p>");
		(*server).sendContent("<p>Дата сборки ПО: <span class='mainvalue'>"+(String)__DATE__+" "+(String)__TIME__+"</span></p>");
		(*server).sendContent("<p>Версия ПО системы Michome: <span class='mainvalue'>"+(String)FVer+"</span></p></div><div>");
		(*server).sendContent("<p>Время работы: <span class='mainvalue'>"+millisToTime(millis())+"</span></p>");
		(*server).sendContent("<p>Время на модуле: <span id='timemod' class='mainvalue'>"+Times+"</span></p></div>");
		if(SendGetaway)
			(*server).sendContent("<p>Соединение со шлюзом: <span id='timemod' class='mainvalue'>"+(String)(IsConnectToGateway ? "установлено" : "потеряно")+"</span></p></div>");
		(*server).sendContent("<div style='background: linear-gradient(45deg, #10ebf3, #87c7ff);'>");
		(*server).sendContent((String)"<p>"+(String)(WiFi.status() != WL_CONNECTED ? "Ошибка подключения к" : "Подключен к сети:")+(String)" <span class='mainvalue'>"+WiFi.SSID()+(String)"</span></p>");
		(*server).sendContent("<p>IP адрес модуля: <span class='mainvalue'>"+(String)(WiFi.status() != WL_CONNECTED ? "192.168.4.1" : WiFi.localIP().toString())+"</span></p>");
		(*server).sendContent((String)"<p>Уровень сигнала: <span class='mainvalue' style='color: blue"+/*GetColorRssi(rssi)+*/(";'>")+String(rssi)+" dBm</span> "+SVGRSSI(rssi)+"</p>");
		(*server).sendContent("</div>");
		//Конец Шапки
		//Начало кнопок
		(*server).sendContent(F("<div><p class='mainlinks'><a href='/configurator'>Настройки модуля</a></p>"));
		(*server).sendContent(F("<div><p class='mainlinks'><a href='/qsettings'>Настройка подсистемы времени</a></p>"));
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
		if(IsUDP) (*server).sendContent(F("<p class='mainlinks'><a href='/udptrigger?type=show'>Конфигурация UDP триггеров</a></p>"));
		if(IsStr(type, Termometr)) (*server).sendContent(F("<div><p class='mainlinks'><a href='/gettemp'>Посмотреть температуру</a></p> <p class='mainlinks'><a href='/gettempinfo'>Посмотреть информацию о термодатчике</a></p></div>"));
		if(IsStr(type, Msinfoo)) (*server).sendContent(F("<div><p class='mainlinks'><a href='/meteo'>Посмотреть погодные данные</a></p> <p class='mainlinks'><a href='/getmsinfoinfo'>Посмотреть информацию о датчиках</a></p></div>"));
		if(IsStr(type, WS28Module)) (*server).sendContent(F("<p class='mainlinks'><a href='/remotepins'>Управление адресной лентой</a></p>"));
		if(IsStr(type, HDC1080md)) (*server).sendContent(F("<p class='mainlinks'><a href='/meteo'>Посмотреть температуру и влажность</a></p>"));
		if(IsStr(type, Informetr)) (*server).sendContent(F("<p class='mainlinks'><a href='/inform?type=show'>Настройка информетра</a></p>"));
		if(IsStr(type, Informetr)) (*server).sendContent(F("<p class='mainlinks'><a href='/test'>Проверка экрана информетра</a></p>"));
		if(IsStr(type, "NotaMesh")) (*server).sendContent(F("<p class='mainlinks'><a href='/notamesh'>Управление гирляндой</a></p>"));
		//Конец Страницы
		(*server).sendContent(F("</div></body></html>"));
		(*server).chunkedResponseFinalize();
		return;
    }
#endif // #ifndef WebPages_h