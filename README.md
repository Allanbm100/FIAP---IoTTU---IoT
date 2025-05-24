# 🛵 Projeto IoTTU - Mapeamento Inteligente do Pátio

## 📋 Descrição do Projeto

Este projeto foi desenvolvido como parte do Challenge 2025, promovido pela Mottu em parceria com a FIAP. A proposta é criar uma solução para mapeamento inteligente de motocicletas nos pátios das filiais da Mottu, permitindo o monitoramento em tempo real da localização e status das motos.

A solução combina tecnologias de IoT (com ESP32), MQTT, visão computacional e um dashboard interativo via Node-RED para visualizar os dados capturados.

---

## 🔧 Tecnologias Utilizadas

- C++ (Arduino/ESP32)
- MQTT (PubSubClient e Mosquitto)
- Node-RED
- Wi-Fi RSSI para estimativa de distância
- Broker MQTT local
- Dashboard web em tempo real

---

## 📡 Arquitetura da Solução

1. **ESP32** simula sensores embarcados nas motos, publicando dados MQTT com ID da moto, status (Disponível, Em manutenção, Indisponível), distância estimada e zona (A, B, C, D).
2. **Broker MQTT** recebe os dados.
3. **Node-RED** consome os dados MQTT, armazena em variáveis de fluxo e exibe num painel interativo.

---

## 🚀 Instalação e Execução

### ESP32 (Firmware Arduino)

1. Abra o `main.cpp` na IDE Arduino.
2. Configure o Wi-Fi (SSID e senha) e o IP do broker MQTT.
3. Carregue o código no ESP32.
4. O dispositivo começará a publicar os dados em loop.

### Node-RED

1. Instale o Node-RED:  
   ```bash
   npm install -g --unsafe-perm node-red
   ```
2. Inicie o Node-RED:
   ```bash
   node-red
   ```
3. Importe o fluxo do arquivo `flows.json` na interface do Node-RED (http://<SEU_IP>:1880).
4. Configure o broker MQTT no Node-RED com o mesmo IP usado no ESP32.
5. Acesse o dashboard: http://<SEU_IP>:1880/ui

---

## 📡 MQTT - Tópicos e Payloads

- **Tópico**: `mottu/motos`
- **Payload exemplo**:
```json
{
  "id": "moto-001",
  "status": "Disponível",
  "zona": "B",
  "distancia": 58.25
}
```

---

## 📍 Funcionalidades

- Identificação única de motos.
- Cálculo de zona baseado em distância (simulação de proximidade).
- Publicação periódica de dados via MQTT.
- Visualização em painel web com botão para limpar os dados.
- Preparado para escalabilidade (até 100+ motos simuladas).

---

## 👥 Equipe

| Nome Completo         | RM       |
| [Allan Brito Moreira] | [558948] |
| [Levi Magni]          | [98276]  |
| [Caio Liang]          | [558868] |
