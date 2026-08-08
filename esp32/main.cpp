#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"
#include "secrets.h"

// ---------------- WiFi ----------------
const char* ssid = SECRET_SSID;
const char* password = SECRET_PASS;

// ---------------- MQTT ----------------
const char* mqtt_server = SECRET_MQTT_SERVER;
const int mqtt_port = 1883;

// ---------------- DHT ----------------
#define DHTTYPE DHT11
#define TEMP_LIMIT 30.0

#define SENSOR_PIN_1 32
#define SENSOR_PIN_2 33
#define SENSOR_PIN_3 14

DHT dht1(SENSOR_PIN_1, DHTTYPE);
DHT dht2(SENSOR_PIN_2, DHTTYPE);
DHT dht3(SENSOR_PIN_3, DHTTYPE);

// ---------------- Output Pins ----------------
const int LIGHT_PINS[10] = {2, 4, 5, 13, 15, 16, 17, 18, 19, 21};
const int FAN_PINS[5] = {22, 23, 25, 26, 27};

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastSensorRead = 0;

// ---------------- MQTT Callback ----------------
void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }
  
    message.trim();
  
    Serial.printf("[MQTT] %s -> %s\n", topic, message.c_str());
    String topic_str(topic);
    int pin_state = (message == "ON") ? HIGH : LOW;
  
    if (topic_str.startsWith("court/")) {
        int court_index = topic_str.substring(6).toInt();
        if (court_index >= 1 && court_index <= 10) {
            digitalWrite(LIGHT_PINS[court_index - 1], pin_state);
            Serial.printf("Court %d -> %s\n",court_index,message.c_str());
        }
    }
}
//---------------- WiFi ----------------
void connectWiFi() {

    if (WiFi.status() == WL_CONNECTED)
        return;
    Serial.print("Connecting WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("WiFi Connected : ");
    Serial.println(WiFi.localIP());
}
// ---------------- MQTT ----------------
void reconnect() {

    if (client.connected())
        return;
    Serial.print("Connecting MQTT...");
    if (client.connect("ESP32_Silent_Worker")) {
        Serial.println("Connected");
        client.subscribe("court/+/control/light");
    } else {
        Serial.print("Failed rc=");
        Serial.println(client.state());
    }
}
void manageFans() {
    if (millis() - lastSensorRead < 2000)
        return;
        
    lastSensorRead = millis();
    float t1 = dht1.readTemperature();
    float t2 = dht2.readTemperature();
    float t3 = dht3.readTemperature();
    if (isnan(t1)) t1 = 0;
    if (isnan(t2)) t2 = 0;
    if (isnan(t3)) t3 = 0;
    Serial.printf("[Sensor] T1: %.1f | T2: %.1f | T3: %.1f\n",t1, t2, t3);
    digitalWrite(FAN_PINS[0], (t1 >= TEMP_LIMIT) ? HIGH : LOW);
    digitalWrite(FAN_PINS[1], (t1 >= TEMP_LIMIT) ? HIGH : LOW);
    digitalWrite(FAN_PINS[2], (t2 >= TEMP_LIMIT) ? HIGH : LOW);
    digitalWrite(FAN_PINS[3], (t3 >= TEMP_LIMIT) ? HIGH : LOW);
    digitalWrite(FAN_PINS[4], (t3 >= TEMP_LIMIT) ? HIGH : LOW);
}
void setup() {
    Serial.begin(115200);
    dht1.begin();
    dht2.begin();
    dht3.begin();

    // ---------------- Light ----------------
    for (int i = 0; i < 10; i++) {
        pinMode(LIGHT_PINS[i], OUTPUT);
        digitalWrite(LIGHT_PINS[i], LOW);
    }
    
    // ---------------- Fan ----------------
    for (int i = 0; i < 5; i++) {
        pinMode(FAN_PINS[i], OUTPUT);
        digitalWrite(FAN_PINS[i], LOW);
    }

    connectWiFi();
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        connectWiFi();
    }
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    manageFans();
}
