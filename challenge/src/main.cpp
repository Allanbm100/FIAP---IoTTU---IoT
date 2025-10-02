#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <math.h>

const char* SSID = "Wokwi-GUEST";
const char* SENHA = "";

const char* MQTT_BROKER = "SEU_IP"; 
const int MQTT_PORTA = 1883;
const char* MQTT_TOPICO = "fiap/iot/moto";

const int LED_PIN = 2;
const int BUZZER_PIN = 4;

const double RSSI_A_1_METRO = -45.0;
const double N_PATH_LOSS = 2.5;
const double EARTH_RADIUS_METERS = 6371000.0;
const double LIMITE_MAX = 120.0;
const double LIMITE_MIN = -10.0;

const int NUM_MOTOS = 8; 


struct PontoCartesiano { double x; double y; };
struct PontoGeografico { double lat; double lon; };

const char* statusDescricao[] = {"", "Disponível", "Indisponível", "Em Manutenção"}; 

struct Totem {
  int id_antena;        
  int id_patio;           
  const char* codigo_antena; 
  PontoGeografico posGeo;   
  PontoCartesiano posCart; 
};

struct Moto {
  int id_status;        
  int id_patio;           
  const char* placa_moto;   
  const char* chassi_moto;  
  const char* nr_motor_moto; 
  const char* modelo_moto;  
  
  double rssi[3]; 
};

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

Totem totens[3] = {
  {1, 1, "ANTENA001", {-23.573300, -46.640000}, {0.00, 0.00}}, 
  {2, 1, "ANTENA002", {-23.573300, -46.639500}, {0.00, 0.00}}, 
  {3, 1, "ANTENA003", {-23.572800, -46.640000}, {0.00, 0.00}}
};

Moto motos[NUM_MOTOS] = {
  {1, 1, "ABC1A23", "CHS1234567890ABCD", "MTR123A", "Honda CG 160", {-71.0, -94.0, -96.0}}, 
  {3, 1, "MNO3E45", "CHS6677889900EFGH", "MTR345E", "Kawasaki Ninja 400", {-85.0, -80.0, -90.0}}, 
  {2, 1, "PQR6F78", "CHS1231231231IJKL", "MTR678F", "Triumph Tiger 900", {-95.0, -72.0, -99.0}}, 
  {1, 1, "YZA5I67", "CHS4564564564MNOP", "MTR567I", "KTM 390 Duke", {-92.0, -98.0, -73.0}}, 
  {1, 1, "BCD8J90", "CHS5675675675QRST", "MTR890J", "Royal Enfield Himalayan", {-70.0, -80.0, -90.0}}, 
  {3, 1, "TUV6Q78", "CHS1112223334UVWZ", "MTR678Q", "Dafra Citycom 300i", {-88.0, -78.0, -84.0}}, 

  {1, 1, "TEST1A1", "CHTEST1AAATEST1AA", "MTTEST1", "Mottu Sim1", {-80.0, -85.0, -88.0}}, 
  {3, 1, "TEST2B1", "CHTEST2BBATEST2BB", "MTTEST2", "Mottu Sim2", {-75.0, -82.0, -78.0}}
};

double grausParaRadianos(double graus) { return graus * PI / 180.0; }
double radianosParaGraus(double radianos) { return radianos * 180.0 / PI; }
double rssiParaDistancia(double rssi) { return pow(10.0, (RSSI_A_1_METRO - rssi) / (10.0 * N_PATH_LOSS)); }
double calcularDistanciaHaversine(PontoGeografico p1, PontoGeografico p2) {
    double dLat = grausParaRadianos(p2.lat - p1.lat);
    double dLon = grausParaRadianos(p2.lon - p1.lon);
    double lat1 = grausParaRadianos(p1.lat);
    double lat2 = grausParaRadianos(p2.lat);
    double a = sin(dLat / 2) * sin(dLat / 2) + sin(dLon / 2) * sin(dLon / 2) * cos(lat1) * cos(lat2);
    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return EARTH_RADIUS_METERS * c;
}
PontoGeografico calcularPontoDestino(PontoGeografico partida, double rumoRadianos, double distanciaMetros) {
    double lat1 = grausParaRadianos(partida.lat);
    double lon1 = grausParaRadianos(partida.lon);
    double dR = distanciaMetros / EARTH_RADIUS_METERS;
    double lat2 = asin(sin(lat1) * cos(dR) + cos(lat1) * sin(dR) * cos(rumoRadianos));
    double lon2 = lon1 + atan2(sin(rumoRadianos) * sin(dR) * cos(lat1), cos(dR) - sin(lat1) * sin(lat2));
    return {radianosParaGraus(lat2), radianosParaGraus(lon2)};
}
PontoCartesiano trilaterar(PontoCartesiano p1, double r1, PontoCartesiano p2, double r2, PontoCartesiano p3, double r3) {
    double A = -2 * p1.x + 2 * p2.x;
    double B = -2 * p1.y + 2 * p2.y;
    double C = r1*r1 - r2*r2 - p1.x*p1.x + p2.x*p2.x - p1.y*p1.y + p2.y*p2.y;
    double D = -2 * p2.x + 2 * p3.x;
    double E = -2 * p2.y + 2 * p3.y;
    double F = r2*r2 - r3*r3 - p2.x*p2.x + p3.x*p3.x - p2.y*p2.y + p3.y*p3.y;
    double denominador = (A * E - B * D);
    if (abs(denominador) < 1e-6) {
        return {-9999.0, -9999.0};
    }
    double x = (C * E - F * B) / denominador;
    double y = (A * F - C * D) / denominador;
    return {x, y};
}
void setup_wifi() {
    delay(10); Serial.println(); Serial.print("Conectando a "); Serial.println(SSID);
    WiFi.begin(SSID, SENHA); int tentativas = 0;
    while (WiFi.status() != WL_CONNECTED && tentativas < 20) { delay(500); Serial.print("."); tentativas++; }
    if(WiFi.status() == WL_CONNECTED) { Serial.println("WiFi conectado!"); Serial.print("Endereço IP: "); Serial.println(WiFi.localIP()); } 
    else { Serial.println("Falha ao conectar no WiFi. Reiniciando em 10s..."); delay(10000); ESP.restart(); }
}
void reconnect_mqtt() {
    while (!mqttClient.connected()) {
        Serial.print("Tentando conectar ao MQTT Broker..."); String clientId = "esp32-mottu-client-";
        clientId += String(random(0xffff), HEX);
        if (mqttClient.connect(clientId.c_str())) { Serial.println("conectado!"); } 
        else { Serial.print("falhou, rc="); Serial.print(mqttClient.state()); Serial.println(" tentando novamente em 5 segundos"); delay(5000); }
    }
}


void loop_simulacao_e_publicacao() {
  StaticJsonDocument<4096> doc;
  JsonArray motos_array = doc.createNestedArray("motos");

  for (int i = 0; i < NUM_MOTOS; i++) {
    Moto& moto = motos[i];
    
    double d1 = rssiParaDistancia(moto.rssi[0]);
    double d2 = rssiParaDistancia(moto.rssi[1]);
    double d3 = rssiParaDistancia(moto.rssi[2]);
    PontoCartesiano posMotoCart = trilaterar(totens[0].posCart, d1, totens[1].posCart, d2, totens[2].posCart, d3);
    
    String alerta = "OK";
    bool foraDoPatio = false;

    if (posMotoCart.x < LIMITE_MIN || posMotoCart.x > LIMITE_MAX || 
        posMotoCart.y < LIMITE_MIN || posMotoCart.y > LIMITE_MAX) {
        alerta = "FORA DO PÁTIO (Simulador)"; 
        foraDoPatio = true;
    }
    
    const char* statusAtual = statusDescricao[moto.id_status];

    double distanciaOrigem = sqrt(pow(posMotoCart.x, 2) + pow(posMotoCart.y, 2));
    double rumoOrigem = atan2(posMotoCart.x, posMotoCart.y); 
    PontoGeografico posMotoGeo = calcularPontoDestino(totens[0].posGeo, rumoOrigem, distanciaOrigem);

    JsonObject moto_obj = motos_array.createNestedObject();
    
    moto_obj["status"] = statusAtual;
    moto_obj["alerta"] = "VERIFICAR"; 
    moto_obj["id_status"] = moto.id_status;         
    moto_obj["id_patio"] = moto.id_patio;           
    moto_obj["placa_moto"] = moto.placa_moto;       
    moto_obj["chassi_moto"] = moto.chassi_moto;     
    moto_obj["nr_motor_moto"] = moto.nr_motor_moto;  
    moto_obj["modelo_moto"] = moto.modelo_moto;     
    
    String rfidCode = "RFID_MOTO_" + String(i + 1);
    moto_obj["codigo_rfid_tag"] = rfidCode; 
    moto_obj["ssid_wifi_tag"] = SSID; 

    moto_obj["x"] = posMotoCart.x;
    moto_obj["y"] = posMotoCart.y;
    moto_obj["latitude"] = posMotoGeo.lat;
    moto_obj["longitude"] = posMotoGeo.lon;
    
    if (foraDoPatio) {
        Serial.printf("ALERTA MOTO %d: POSIÇÃO CALCULADA FORA DOS LIMITES. O Java irá validar.\n", i + 1);
    } else {
        Serial.printf("MOTO %d (x,y): (%.2f, %.2f)m | Status: %s\n", i + 1, posMotoCart.x, posMotoCart.y, statusAtual);
    }
  }
  
  char jsonBuffer[4096];
  serializeJson(doc, jsonBuffer);

  Serial.println("--- Publicando Array de Motos (Payload Único) ---");
  if (mqttClient.publish(MQTT_TOPICO, jsonBuffer)) {
    digitalWrite(LED_PIN, HIGH); delay(50); digitalWrite(LED_PIN, LOW);
  } else {
    Serial.println("Falha na publicação MQTT do array.");
  }
}

void setup() {
  Serial.begin(115200); pinMode(LED_PIN, OUTPUT); pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); digitalWrite(BUZZER_PIN, LOW);
  setup_wifi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);
  mqttClient.setBufferSize(4096); 
  reconnect_mqtt();
  
  totens[0].posCart = {0.0, 0.0};
  
  double dist1_2 = calcularDistanciaHaversine(totens[0].posGeo, totens[1].posGeo);
  totens[1].posCart = {dist1_2, 0.0}; 
  
  double dist1_3 = calcularDistanciaHaversine(totens[0].posGeo, totens[2].posGeo);
  totens[2].posCart = {0.0, dist1_3}; 
  
  Serial.printf("POS CARTESIANAS RECALCULADAS:\n");
  Serial.printf("ANTENA 1: (%.2f, %.2f)m\n", totens[0].posCart.x, totens[0].posCart.y);
  Serial.printf("ANTENA 2: (%.2f, %.2f)m (Distancia: %.2f m)\n", totens[1].posCart.x, totens[1].posCart.y, dist1_2);
  Serial.printf("ANTENA 3: (%.2f, %.2f)m (Distancia: %.2f m)\n", totens[2].posCart.x, totens[2].posCart.y, dist1_3);


  Serial.println("--- Publicando Array de Totens (TB_ANTENA) via MQTT ---");
  
  StaticJsonDocument<1024> doc_totem_array; 
  JsonArray antenas_array = doc_totem_array.createNestedArray("antenas");

  for (int i = 0; i < 3; i++) {
    JsonObject totem_obj = antenas_array.createNestedObject();
    
    totem_obj["id_antena"] = totens[i].id_antena;
    totem_obj["id_patio"] = totens[i].id_patio;
    totem_obj["codigo_antena"] = totens[i].codigo_antena;
    totem_obj["latitude_antena"] = totens[i].posGeo.lat;
    totem_obj["longitude_antena"] = totens[i].posGeo.lon;

    totem_obj["x"] = totens[i].posCart.x;
    totem_obj["y"] = totens[i].posCart.y;
  }
  
  char jsonBuffer_antena[1024];
  serializeJson(doc_totem_array, jsonBuffer_antena);

  if (mqttClient.publish(MQTT_TOPICO, jsonBuffer_antena)) {
    Serial.println(">> Array de Antenas (TB_ANTENA) publicado com sucesso.");
  } else {
    Serial.println("Falha na publicação do array de Antenas.");
  }

  Serial.println("------------------------------------");
}

void loop() {
  if (!mqttClient.connected()) { reconnect_mqtt(); } 
  mqttClient.loop(); 

  static unsigned long lastMsg = 0;
  unsigned long now = millis();
  if (now - lastMsg > 3000) { 
    lastMsg = now;
    loop_simulacao_e_publicacao(); 
  }
}