# 🛵 Projeto IoTTU - Mapeamento Inteligente do Pátio (Challenge FIAP/Mottu)

## Link Do vídeo
https://youtu.be/w2GqpwASFHs

## 📋 Descrição do Projeto

Este projeto demonstra uma solução robusta de monitoramento de ativos (**motocicletas**) em um pátio, desenvolvida como parte do **Challenge 2025 da Mottu em parceria com a FIAP**.  
A solução permite o rastreamento em tempo real da localização, calculada via **trilateração**, e o status de cada moto.

A arquitetura combina um **Simulador IoT (ESP32 em C++)** para calcular a localização e publicar os dados, e um **Backend Java Spring Boot** que atua como consumidor das mensagens **MQTT**, processa os dados e persiste em um banco de dados **PostgreSQL (via Docker)**.

---

## 🔧 Tecnologias Utilizadas

### Hardware/Firmware
- **ESP32** - Microcontrolador
- **C++/Arduino** - Linguagem de programação
- **PlatformIO** - Build system
- **Wokwi** - Simulador online

### Comunicação
- **MQTT** - Protocolo de mensageria
- **Mosquitto** - Broker MQTT
- **PubSubClient** - Biblioteca MQTT para ESP32

### Processamento
- **Node-RED** - Flow-based programming
- **ArduinoJson** - Serialização JSON

### Algoritmos
- **Trilateração** - Cálculo de posição 2D
- **Conversão RSSI → Distância** - Path loss model
- **Haversine** - Conversão coordenadas cartesianas → GPS

## 📡 Arquitetura

```
ESP32 (Wokwi) → MQTT Broker → Node-RED → PostgreSQL
                  ↓
            (Topic: fiap/iot/moto)
```

### Fluxo de Dados

1. **ESP32** lê RSSI das 3 antenas
2. Calcula posição por **trilateração**
3. Converte coordenadas cartesianas para **lat/lon**
4. Publica JSON via **MQTT** (QoS 1)
5. **Node-RED** consome mensagens
6. Executa `INSERT ... ON CONFLICT` no **PostgreSQL**

## 📁 Estrutura do Projeto

```
FIAP---IoTTU---IoT/
│
├── challenge/
│   ├── diagram.json              # Circuito Wokwi
│   ├── wokwi.toml                # Configuração Wokwi
│   └── src/
│       └── main.cpp              # Firmware ESP32
│
├── node-red/
│   ├── flow-iottu.json           # Flow Node-RED
│   └── mosquitto/                # Configs MQTT
│
├── platformio.ini                # Config PlatformIO
├── LICENSE
└── README.md
```

## 🔧 Como Rodar o Projeto

### Pré-requisitos

- **PlatformIO** (extensão VSCode)
- **Node-RED** (`npm install -g --unsafe-perm node-red`)
- **Mosquitto MQTT Broker**
- **PostgreSQL** (ver repositório Java para setup)
- **Node.js** (para executar scripts de integração com Azure PostgreSQL)
- **Módulo pg** (cliente PostgreSQL para Node.js) - instalado via `npm install pg` na pasta `node-red/`
- **(Opcional)** Conta Wokwi para simulação online

### ⚙️ Configuração Importante - Caminho do Script PostgreSQL

**ATENÇÃO**: Antes de importar o flow no Node-RED, você precisa configurar o caminho do script `pg-executor.bat`:

1. No arquivo `node-red/flow-iottu.json`, localize o nó `postgres_exec`
2. Encontre a linha com `"command":`
3. **Substitua** o valor **[CAMINHO COMPLETO DO PG-EXECUTOR.BAT]** pelo que é pedido
4. Use o caminho completo do arquivo no seu sistema

**Exemplo:**
- **Windows**: `C:\\Users\\SeuNome\\Desktop\\Fiap\\IoT\\FIAP---IoTTU---IoT\\node-red\\pg-executor.bat`
- **Mac/Linux**: `/home/seunome/Desktop/Fiap/IoT/FIAP---IoTTU---IoT/node-red/pg-executor.bat`


### Passo a Passo

#### 1. Clone o Repositório
```bash
git clone https://github.com/Allanbm100/FIAP---IoTTU---IoT.git
cd FIAP---IoTTU---IoT
```

#### 2. Configure o MQTT Broker

**Opção A - Mosquitto Local (Windows):**
```bash
# Download: https://mosquitto.org/download/
# Ou via Chocolatey:
choco install mosquitto

# Iniciar serviço
net start mosquitto
```

**Opção B - Docker:**
```bash
docker run -d -p 1883:1883 --name mosquitto eclipse-mosquitto
```

#### 3. Configure o ESP32

Edite `challenge/src/main.cpp`:
```cpp
const char* MQTT_BROKER = "192.168.0.151"; // Altere para seu IP
const int MQTT_PORTA = 1883;
```

#### 4. Compile e Execute

**Opção A - PlatformIO (VSCode):**
```bash
pio run
# ou use o botão "Build" na barra inferior
```

**Opção B - Wokwi Simulator:**
1. Acesse https://wokwi.com/vscode
2. Abra `challenge/diagram.json`
3. Clique em "Start Simulation"

#### 5. Configure Node-RED

```bash
# 1. Instale as dependências do PostgreSQL
cd node-red
npm install pg

# 2. Inicie o Node-RED
node-red

# 3. Acesse http://localhost:1880
```

**4. Configure o caminho do script:**
- Abra o arquivo `node-red/flow-iottu.json` em um editor de texto
- Procure por `[COLOQUE O CAMINHO COMPLETO DO PG-EXECUTOR.BAT]`
- Substitua pelo caminho completo do arquivo `pg-executor.bat` no seu sistema
- Exemplo: `C:\\Users\\Allan\\Desktop\\Fiap\\IoT\\FIAP---IoTTU---IoT\\node-red\\pg-executor.bat`

**5. Importe o flow:**
- No Node-RED: Menu → Import → Selecione `node-red/flow-iottu.json`
- Clique em **Deploy**

**6. Verifique a conexão:**
- Você deve ver mensagens "✅ Salvo" no debug quando o ESP32 publicar dados

#### 6. Verifique os Dados

```bash
# Conecte ao PostgreSQL
docker exec -it postgres_iottu psql -U iottu -d iottu

# Consultas
SELECT COUNT(*) FROM tb_antena WHERE id_antena >= 10000;
SELECT COUNT(*) FROM tb_moto WHERE chassi_moto LIKE 'CHS%';
SELECT * FROM tb_tag WHERE codigo_rfid_tag LIKE 'RFID_MOTO%';
```

## 📡 Protocolo MQTT

### Configuração
- **Broker**: `tcp://192.168.0.151:1883` (altere conforme seu IP)
- **Tópico**: `fiap/iot/moto`
- **QoS**: 1 (At least once delivery)
- **Client ID**: `esp32-mottu-client-XXXX`

### Mensagens Publicadas

#### Setup - Antenas (executado 1x no início)
```json
{
  "antenas": [
    {
      "id_antena": 10001,
      "codigo_antena": "ANTENA001",
      "id_patio": 1,
      "latitude_antena": -23.573300,
      "longitude_antena": -46.640000,
      "x": 0.0,
      "y": 0.0
    }
  ]
}
```

#### Loop - Motocicletas (a cada 3 segundos)
```json
{
  "motos": [
    {
      "status": "Disponível",
      "alerta": "VERIFICAR",
      "id_status": 1,
      "id_patio": 1,
      "placa_moto": "ABC1A23",
      "chassi_moto": "CHS1234567890ABCD",
      "nr_motor_moto": "MTR123A",
      "modelo_moto": "Honda CG 160",
      "codigo_rfid_tag": "RFID_MOTO_1",
      "ssid_wifi_tag": "Wokwi-GUEST",
      "x": 25.43,
      "y": 18.76,
      "latitude": -23.573425,
      "longitude": -46.640123
    }
  ]
}
```

## 🧮 Algoritmos Implementados

### 1. RSSI para Distância
```cpp
double rssiParaDistancia(double rssi) {
    return pow(10.0, (RSSI_A_1_METRO - rssi) / (10.0 * N_PATH_LOSS));
}
```
- **RSSI_A_1_METRO**: -45 dBm (calibração)
- **N_PATH_LOSS**: 2.5 (ambiente indoor)

### 2. Trilateração 2D
```cpp
PontoCartesiano trilaterar(
    PontoCartesiano p1, double r1,
    PontoCartesiano p2, double r2,
    PontoCartesiano p3, double r3
)
```
Resolve o sistema de equações para encontrar (x, y) da moto.

### 3. Haversine (Cartesiano → GPS)
```cpp
PontoGeografico calcularPontoDestino(
    PontoGeografico partida,
    double rumoRadianos,
    double distanciaMetros
)
```
Converte coordenadas locais (metros) em latitude/longitude.

## 🔗 Integração com Backend

Este projeto IoT se integra com:

📦 **Backend Java**: [FIAP--IoTTU--Java-ChallengeProject](https://github.com/Allanbm100/FIAP--IoTTU--Java-ChallengeProject)

**Endpoints disponíveis:**
- `GET /api/v1/motorcycles` - Listar motos
- `GET /api/v1/tags` - Posições em tempo real
- `GET /api/v1/antennas` - Antenas do pátio
- `GET /api/v1/yards/{id}/map` - Mapa do pátio

## 🧪 Testando o Sistema

### 1. Verificar MQTT
```bash
# Instale mosquitto_sub
mosquitto_sub -h localhost -t "fiap/iot/moto" -v
```

### 2. Consultar Banco
```sql
-- Últimas posições recebidas
SELECT 
  m.placa_moto,
  t.codigo_rfid_tag,
  t.latitude_tag,
  t.longitude_tag,
  t.data_hora_tag
FROM tb_moto m
JOIN tb_tag t ON t.id_tag = (
  SELECT tm.id_tag FROM tb_moto_tag tm WHERE tm.id_moto = m.id_moto LIMIT 1
)
WHERE m.chassi_moto LIKE 'CHS%'
ORDER BY t.data_hora_tag DESC;
```

### 3. Logs Node-RED
Verifique a aba **Debug** no Node-RED (`http://localhost:1880`) para ver mensagens processadas.