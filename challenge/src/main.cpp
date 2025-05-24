#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Wokwi-GUEST";
const char* password = "";

const char* mqtt_server = "SEU_IP";
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

String calcularZona(float distancia) {
  if (distancia < 30.0) return "A";
  else if (distancia < 60.0) return "B";
  else if (distancia < 120.0) return "C";
  else return "D";
}

const char* statusOpcoes[] = {"Disponível", "Em manutenção", "Indisponível"};

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  conectarWiFi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) conectarMQTT();
  client.loop();

  for (int i = 1; i <= 10; i++) {
    int rssi = WiFi.RSSI();
    float distancia = calcularDistancia(rssi);
    String zona = calcularZona(distancia);

    char idMoto[10];
    snprintf(idMoto, sizeof(idMoto), "moto-%03d", i);

    int idxStatus = random(0, 3);
    const char* status = statusOpcoes[idxStatus];

    String payload = "{";
    payload += "\"id\":\"" + String(idMoto) + "\",";
    payload += "\"status\":\"" + String(status) + "\",";
    payload += "\"zona\":\"" + zona + "\",";
    payload += "\"distancia\":" + String(distancia, 2);
    payload += "}";

    Serial.println("Enviando: " + payload);
    client.publish(mqtt_topic, payload.c_str());

    delay(2000);
  }

  delay(50000);
}
