#include <WiFi.h>
#include <WebServer.h>
#include "time.h"

#define BLINK_GPIO 2
// SSID & Password
const char *ssid = "ykp74_home";
const char *password = "00153pyk";

const char *ntpServer = "kr.pool.ntp.org";

//3200초는 1시간, 우리나라는 GMT+9 이므로 3600초x9 = 32400 해 줌
const long  gmtOffset_sec = 32400;
const int   daylightOffset_sec = 0;  //섬머타임 적용 시 3600 (시간을 1시간 빠르게 함) 우리나라는 시행 안하므로 0

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void setup() {
  // No need to initialize the RGB LED
  pinMode(BLINK_GPIO, OUTPUT);

  Serial.begin(115200);
	
  
  Serial.println("ESP32 Simple web Start");
	Serial.println(ssid);

  	//WiFi 접속
	WiFi.begin(ssid, password);

	//접속 완료 하면
	while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
	}
    
	Serial.print("Wifi IP: ");
	Serial.println(WiFi.localIP());  //Show ESP32 IP on serial

  //init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

	//InitWebServer(); 
    
	Serial.println("HTTP server started");
	delay(100); 
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(BLINK_GPIO, HIGH);   // Turn the RGB LED white
  printLocalTime();
  delay(1000);
  digitalWrite(BLINK_GPIO, LOW);    // Turn the RGB LED off
  printLocalTime();
  delay(1000);
}