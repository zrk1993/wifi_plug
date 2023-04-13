#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h> 
#include <AsyncElegantOTA.h>
#include <PubSubClient.h>
#include <LittleFS.h>
#include <OneButton.h>

#include "blink.h"
#include "config.h"

#define SW_PIN 4
#define BUTTON_PIN 12

bool sw_state = false;
bool enable_mqtt = false;

unsigned long lastMqttReconnectTime = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

AsyncWebServer server(80);
DNSServer dns;

OneButton button(BUTTON_PIN, true);

void changeSw(bool state) {
  sw_state = state;
  digitalWrite(SW_PIN, state);
}

void connectWifi() {
  AsyncWiFiManager wifiManager(&server, &dns);
  wifiManager.autoConnect("ESPConfigAP");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

String chipInfo() {
  String str = "";
  str += "&ChipID=";
  str += ESP.getChipId();
  str += "&FlashSize=";
  str += ESP.getFlashChipSize();
  str += "&AvailableHeap=";
  str += ESP.getFreeHeap();
  str += "&StationSSID=" + WiFi.SSID();
  str += "&StationIP=" + WiFi.localIP().toString();
  str += "&StationMAC=" + WiFi.macAddress();
  return str;
}

void callback(char* topic, byte* payload, size_t length) {
	Serial.print("Topic:");
	Serial.println(topic);
	String msg = "";
	for (size_t i = 0; i < length; i++) {
		msg += (char)payload[i];
	}
	Serial.print("Msg:");
	Serial.println(msg);
	if (msg == "off") {
    changeSw(false);
    blink(2, 150);
	} else {
    changeSw(true);
    blink(2, 150);
	}
}

bool checkMqttEnable () {
  enable_mqtt = *config.mqtt_host != '\0' && *config.mqtt_port != '\0' && *config.mqtt_key != '\0' && *config.mqtt_topic != '\0';
  return enable_mqtt;
}

void reconnect() {
  if (lastMqttReconnectTime != 0 && millis() > lastMqttReconnectTime && millis() < lastMqttReconnectTime + 1000 * 60 * 5) {
    return;
  }
  lastMqttReconnectTime = millis();
  int i = 5;
	while (!mqttClient.connected() && i--) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (mqttClient.connect(config.mqtt_key)) {
			Serial.println("connected");
			Serial.print("subscribe:");
			Serial.println(config.mqtt_topic);
			//订阅主题，如果需要订阅多个主题，可发送多条订阅指令client.subscribe(topic2);client.subscribe(topic3);
			mqttClient.subscribe(config.mqtt_topic);
		} else {
			Serial.print("failed, rc=");
			Serial.print(mqttClient.state());
			Serial.println(" try again in 1 seconds");
			// Wait 1 seconds before retrying
			delay(1000);
		}
	}
}

void setServer() {
  if (MDNS.begin(config.host_name)) { // Start mDNS with name esp8266
		Serial.println("mDNS started");
		Serial.printf("http://%s.local\n", config.host_name);
	}

  AsyncElegantOTA.begin(&server); // Start ElegantOTA

  // star littlefs
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  Serial.println("Littlefs is success open");
  AsyncStaticWebHandler* handler = &server.serveStatic("/", LittleFS, "/");
  handler->setDefaultFile("index.html");

  server.on("/state", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", sw_state ? "on" : "off");
	});
  
	server.on("/sw", HTTP_GET, [](AsyncWebServerRequest *request) {
		if(request->hasArg("state")) {
			String v = request->arg("state");
      changeSw(v.equals("on"));
		}
    request->send(200, "text/plain", "ok");
	});

  server.on("/info", HTTP_GET, [](AsyncWebServerRequest *request) {
    String info = chipInfo();
		request->send(200, "text/plain", info);
	});
  server.on("/loadConfig", HTTP_GET, [](AsyncWebServerRequest *request) {
		String config_txt = getConfigTxt();
		request->send(200, "text/plain", config_txt);
	});
	server.on("/saveConfig", HTTP_GET, [](AsyncWebServerRequest *request) {
		strcpy(config.host_name, request->arg("host_name").c_str());
    strcpy(config.mqtt_host, request->arg("mqtt_host").c_str());
    strcpy(config.mqtt_port, request->arg("mqtt_port").c_str());
		strcpy(config.mqtt_key, request->arg("mqtt_key").c_str());
		strcpy(config.mqtt_topic, request->arg("mqtt_topic").c_str());
		saveConfig();
		request->send(200, "text/plain", "save ok, delay restart ...");
	});

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request) {
		request->send(200, "text/plain", "restart");
		Serial.println("restart 。。。 ");
		ESP.restart();
	});

  server.begin();
  Serial.println("Web server started");
}

void buttonClick() {
  changeSw(!sw_state);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("\nSerial star Running");
  pinMode(BLINK_PIN, OUTPUT);
  pinMode(SW_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  blink_on();
	loadConfig();
	Serial.println("config_txt: " + getConfigTxt());
  connectWifi();
  setServer();

  if (checkMqttEnable()) {
    Serial.println("MQTT Start ");
    mqttClient.setServer(config.mqtt_host, atoi(config.mqtt_port)); // 设置mqtt服务器
    mqttClient.setCallback(callback); // mqtt消息处理
    reconnect();
  }

  blink_ok();
  changeSw(false);
  button.attachClick(buttonClick);

  Serial.println("start ok.");
}

void loop() {
  delay(100);
  button.tick();
  MDNS.update();
  if (enable_mqtt) {
    if (mqttClient.connected()) {
      mqttClient.loop();
    } else {
      reconnect();
    }
  }
}