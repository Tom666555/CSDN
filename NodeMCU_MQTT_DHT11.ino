#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"

#define DHTPIN D4     //DHT11与NodeMUC的连接

// 连接WIFI
#define wifi_ssid "HUAWEI"
#define wifi_password "876543210"


#define mqtt_server "42.192.88.238"  // MQTT云地址

//topic
#define topic "AAA"

#define DHTTYPE DHT11   // DHT 11

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {
    delay(10);
    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
Serial.println("WiFi 连接成功!");
}

void reconnect() {
    // 循环,直到我们重新连接
    while (!client.connected()) {
        Serial.print("尝试MQTT连接…");
        if (client.connect("nodeMcuDHT11")) {
            Serial.println("连接成功");
        } else {
            Serial.print("失败,rc =");
            Serial.print(client.state());
            Serial.println(" 在5秒后再试一次");
            delay(5000);
        }
    }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
    return newValue < prevValue - maxDiff || newValue > prevValue + maxDiff;
}

long lastMsg = 0;
int i = 1;
float temp = 0.0;
float hum = 0.0;
//float diff = 1.0;

void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);
    dht.begin();
}

void loop() {
    if (!client.connected()) {
        reconnect();
    }
    client.loop();

    long now = millis();
    if (now - lastMsg > 30000) {
        // 等待几秒钟之间测量
        lastMsg = now;

        float newTemp = dht.readTemperature();
        float newHum = dht.readHumidity();
        temp = newTemp;
        hum = newHum;
        
        // json serialize（json序列化）
        DynamicJsonDocument data(256);
        data["xh"] = i++;
        data["temp"] = temp;
        data["hum"] = hum;
        
        // 发布温度和湿度
        char json_string[256];
        serializeJson(data, json_string);
        // {"xh":1,"temp":23.5,"hum":55}
        Serial.println(json_string);
        client.publish(topic, json_string, false);
        delay(2000);
    }
}
