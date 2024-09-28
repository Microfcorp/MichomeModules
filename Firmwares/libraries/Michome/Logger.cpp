#include "Logger.h"

//
// конструктор - вызывается всегда при создании экземпляра класса Logger
//
Logger::Logger(String gateway, const char* host)
{
        Serial.begin(115200);
		Gateway = gateway;
		Host = host;
}

//
//
void Logger::Log(String text)
{
		  // Use WiFiClient class to create TCP connections
		  WiFiClient client;
		  const int httpPort = 80;
		  if (!client.connect(Host, httpPort)) {
			Serial.println("connection failed");
			return;
		  }
		  
		  String dataaaa = parsejsonlogger("Log", text);

		  String lengt = (String)dataaaa.length(); 
		  
		  // This will send the request to the server
		  client.print(String("POST ") + "http://" + Gateway + " HTTP/1.1\r\n" +
					   "Host: " + "192.168.1.42" + "\r\n" + 
					   "Content-Length: " + lengt + "\r\n" +
					   "Content-Type: application/x-www-form-urlencoded \r\n" +
					   "Connection: close\r\n\r\n" +
					   "6=" + dataaaa);
		  unsigned long timeout = millis();
		  while (client.available() == 0) {
			if (millis() - timeout > 5000) {
			  Serial.println(">>> Client Timeout !");
			  client.stop();
			  return;
			}
		  }		 
}

String Logger::parsejsonlogger(String type, String data){
  String temp = "";
  temp += "{";
  temp += "\"ip\":\"" + WiFi.localIP().toString() + "\",";
  temp += "\"rssi\":\"" + String(WiFi.RSSI()) + "\",";
  temp += "\"type\":";
  temp += "\"" + type + "\",";   
  temp += "\"data\":{";
  temp += "\"log\": \"" + String(data) + "\"} } \r\n";
  temp += "     ";
  return temp; 
}