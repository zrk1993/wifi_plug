#ifndef CONFIG_h
#define CONFIG_h

#include <Arduino.h>
#include <EEPROM.h>
#include <string.h>

/**
* 结构体
*/
struct config_type {
    char host_name[32];
    char mqtt_host[32];
    char mqtt_port[8];
    char mqtt_key[32];
    char mqtt_topic[16];
};

config_type config = { "myplug", "bemfa.com", "9501", "", "myplug003" };

String getConfigTxt() {
    String str = "";
    str += "host_name=";
    str += config.host_name;
    str += "&";
    str += "mqtt_host=";
    str += config.mqtt_host;
    str += "&";
    str += "mqtt_port=";
    str += config.mqtt_port;
    str += "&";
    str += "mqtt_key=";
    str += config.mqtt_key;
    str += "&";
    str += "mqtt_topic=";
    str += config.mqtt_topic;
    str += "&";
    return str;
}

/**
* 存储信息
*/
void saveConfig() {
    EEPROM.begin(128);
    uint8_t *p = (uint8_t*)(&config);
    for (unsigned int i = 0; i < sizeof(config); i++) {
        EEPROM.write(i, *(p + i));
    }
    EEPROM.commit();
}

/**
* 加载存储的信息
*/
uint8_t *p = (uint8_t*)(&config);
void loadConfig() {
    EEPROM.begin(512);
    for (unsigned int i = 0; i < sizeof(config); i++) {
        *(p + i) = EEPROM.read(i);
    }
    EEPROM.commit();
}

#endif
