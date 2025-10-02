# 🛵 Projeto IoTTU - Mapeamento Inteligente do Pátio (Challenge FIAP/Mottu)

## 📋 Descrição do Projeto

Este projeto demonstra uma solução robusta de monitoramento de ativos (**motocicletas**) em um pátio, desenvolvida como parte do **Challenge 2025 da Mottu em parceria com a FIAP**.  
A solução permite o rastreamento em tempo real da localização, calculada via **trilateração**, e o status de cada moto.

A arquitetura combina um **Simulador IoT (ESP32 em C++)** para calcular a localização e publicar os dados, e um **Backend Java Spring Boot** que atua como consumidor das mensagens **MQTT**, processa os dados e persiste em um banco de dados **PostgreSQL (via Docker)**.

---

## 🔧 Tecnologias Utilizadas

| Componente      | Tecnologia              | Função                                                                 |
|-----------------|-------------------------|------------------------------------------------------------------------|
| Simulador IoT   | C++ (Arduino/ESP32)     | Calcula a posição das motos via Trilateração (RSSI) e publica via MQTT |
| Backend         | Java (Spring Boot)      | Recebe mensagens MQTT, processa e armazena os dados                    |
| Mensageria      | MQTT (PubSubClient)     | Protocolo leve para comunicação em tempo real                          |
| Persistência    | PostgreSQL + Docker     | Banco de dados relacional para rastreamento                            |

---

## 🚀 Instalação e Execução

### 1. Pré-requisitos

Certifique-se de ter instalado:

- Java Development Kit (JDK 17+)
- Maven
- Docker e Docker Compose
- Ambiente Arduino/Wokwi ou PlatformIO (para o simulador C++)

---

### 2. ⚠️ Configuração Crucial: Endereço IP do Broker MQTT

A comunicação entre o simulador IoT e o backend Java exige que o **endereço IP** da máquina que executa o broker seja configurado em **dois arquivos**.  
Substitua `SEU_IP` pelo IP real da sua máquina.

| Arquivo                  | Variável       | Onde trocar                              |
|---------------------------|----------------|------------------------------------------|
| `main.cpp` (C++)          | `MQTT_BROKER`  | Pelo endereço IP do seu computador       |
| `application.properties`  | `spring.mqtt.host` | Pelo mesmo endereço usado no `main.cpp` |

---

### 3. Backend Java e Banco de Dados

No repositório **[CHALLENGE-Java-IoTTU](https://github.com/Allanbm100/CHALLENGE-Java-IoTTU)**:

#### B. Rodar a Aplicação Spring Boot

Compile e execute a aplicação (que atuará como consumidor MQTT e integrará com o banco):

Ao acessar no localhost:8080, crie uma conta, acesse-a e crie um pátio.

---

### 4. Simulador IoT (ESP32/C++)

1. Edite o `main.cpp` e configure:
   - IP do `MQTT_BROKER`
   - Credenciais de rede Wi-Fi
2. Compile e carregue no ESP32 (físico ou simulado/Wokwi).
3. O dispositivo publicará um **array com os dados das motos** no tópico `fiap/iot/moto` a cada 3 segundos.

---

## 📡 Tópico e Payload MQTT

### Tópico:

```
fiap/iot/moto
```

### Estrutura do Payload (Array de Motos):

```json
{
  "motos": [
    {
      "status": "Disponível",
      "alerta": "VERIFICAR",
      "id_status": 1,
      "placa_moto": "ABC1A23",
      "modelo_moto": "Honda CG 160",
      "x": 1.15,
      "y": -11.66,
      "latitude": -23.573200,
      "longitude": -46.640100
    }
    // ... Outras motos
  ]
}
```

Além disso, o simulador publica também um **array de antenas (TB_ANTENA)** no mesmo tópico, no formato:

```json
{
  "antenas": [
    {
      "id_antena": 1,
      "codigo_antena": "ANTENA001",
      "latitude_antena": -23.573300,
      "longitude_antena": -46.640000,
      "x": 0.0,
      "y": 0.0
    }
    // ... Outras antenas
  ]
}
```

---

## 👥 Equipe

| Nome Completo       | RM     |
|---------------------|--------|
| Allan Brito Moreira | 558948 |
| Levi Magni          | 98276  |
| Caio Liang          | 558868 |

---

## 📌 Observações Finais

- O `SEU_IP` **deve ser idêntico** no `main.cpp` e no `application.properties`  
- O **Docker precisa estar rodando** para subir o PostgreSQL antes da API  
- Ferramentas como [MQTT Explorer](https://mqtt-explorer.com/) podem ser usadas para inspecionar as mensagens em tempo real  
