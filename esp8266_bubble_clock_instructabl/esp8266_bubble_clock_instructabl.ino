//OTAでアップロード可能なプログラムサンプル
//https://makers-with-myson.blog.so-net.ne.jp/2017-01-11
//MainOTAに本体を追記する

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#define heartbeatINTERVAL_led 4000 // msec 

void setup() {
  Serial.begin(115200);

  Serial.println("Connecting Wifi...");
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("your_wifi_ssid", "your_wifi_password");
  //  wifiMulti.addAP("ssid_from_AP_2", "your_password_for_AP_2");
  //  wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  while (wifiMulti.run() != WL_CONNECTED) {
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("WiFi not connected!");
      delay(1000);
      //      ESP.restart();
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("   SSID: ");
  Serial.print(WiFi.SSID());
  Serial.print("   IP address: ");
  Serial.println(WiFi.localIP());

  // Hostname defaults to esp8266-[ChipID]
ArduinoOTA.setHostname("buble clock.local");  // <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

  // No authentication by default
  //ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  OTA_setup();//追記
}

void loop() {
  ArduinoOTA.handle();
  OTA_loop();//追記
}
