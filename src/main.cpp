#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <LittleFS.h>

#include "blink.h"
#include "PubSubClient.h"

const char* hostName = "myplug";
const char* ssid = "CDS";
const char* password = "Chan888999";

const char* ID_MQTT = "d6d8fcd160ce48a3b38ff76e7e2df726";
const char* topic = "myplug003";
const char* mqtt_server = "bemfa.com";
const int mqtt_server_port = 9501;

unsigned long lastMqttReconnectTime = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

AsyncWebServer server(80);

void connectWifi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
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
		blink_off();
	} else if (msg == "on") {
		blink_on();
	} else {
		msg.replace("on#", "");
    blink_on();
	}
}

void reconnect() {
  if (lastMqttReconnectTime > millis() - 1000 * 60 * 5) {
    return;
  }
  lastMqttReconnectTime = millis();
  int i = 5;
	while (!mqttClient.connected() && i--) {
		Serial.print("Attempting MQTT connection...");
		// Attempt to connect
		if (mqttClient.connect(ID_MQTT)) {
			Serial.println("connected");
			Serial.print("subscribe:");
			Serial.println(topic);
			//订阅主题，如果需要订阅多个主题，可发送多条订阅指令client.subscribe(topic2);client.subscribe(topic3);
			mqttClient.subscribe(topic);
		} else {
			Serial.print("failed, rc=");
			Serial.print(mqttClient.state());
			Serial.println(" try again in 5 seconds");
			// Wait 1 seconds before retrying
			delay(1000);
		}
	}
}

void setServer() {
  if (MDNS.begin(hostName)) { // Start mDNS with name esp8266
		Serial.println("mDNS started");
		Serial.printf("http://%s.local\n", hostName);
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

  server.begin();
  Serial.println("Web server started");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("\nSerial star Running");
  blink_on();
  connectWifi();
  setServer();

  mqttClient.setServer(mqtt_server, mqtt_server_port); // 设置mqtt服务器
  mqttClient.setCallback(callback); // mqtt消息处理

  blink_ok();
}

void loop() {
  MDNS.update();
	if (mqttClient.connected()) {
		mqttClient.loop();
	} else {
		reconnect();
	}
}