#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "192.168.0.169";
const int mqtt_port = 1883;
const char* mqtt_topic = "mottu/motos";

WiFiClient espClient;
PubSubClient client(espClient);

float calcularDistancia(int rssi, int txPower = -50, float fatorPerda = 2.0) {
  return pow(10.0, ((float)(txPower - rssi)) / (10.0 * fatorPerda));
}

void conectarWiFi() {
  Serial.print("Conectando ao Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi conectado. IP: " + WiFi.localIP().toString());
}

void conectarMQTT() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("Conectado ao broker MQTT!");
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(". Tentando novamente em 5s...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  conectarWiFi();
  client.setServer(mqtt_server, mqtt_port);
}

struct Moto {
  String id;
  String status;
  String zona;
};

Moto motos[] = {
  {"moto-001", "Manutenção", "A1"},
  {"moto-002", "", "B2"},
  {"moto-003", "ligada", "C1"},
};

void loop() {
  if (!client.connected()) conectarMQTT();
  client.loop();

  for (int i = 0; i < 3; i++) {
    int rssi = WiFi.RSSI();
    float distancia = calcularDistancia(rssi);

    String payload = "{";
    payload += "\"id\":\"" + motos[i].id + "\",";
    payload += "\"status\":\"" + motos[i].status + "\",";
    payload += "\"zona\":\"" + motos[i].zona + "\",";
    payload += "\"rssi\":" + String(rssi) + ",";
    payload += "\"distancia\":" + String(distancia, 2);
    payload += "}";

    Serial.println("Enviando: " + payload);
    client.publish(mqtt_topic, payload.c_str());

    delay(2000);
  }

  delay(25000);
}

