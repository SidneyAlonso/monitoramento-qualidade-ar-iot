# Sistema de Monitoramento da Qualidade do Ar em Pontos de Ônibus

**Universidade Presbiteriana Mackenzie — Faculdade de Computação e Informática**  
Autores: Sidney Quiota Alonso, André Luis de Oliveira, Leandro Carlos Fernandes  
Ano: 2026

---

## Descrição

Protótipo de monitoramento da qualidade do ar em pontos de ônibus utilizando Internet das Coisas (IoT). O sistema lê os dados do sensor MQ-135, classifica a qualidade do ar em três níveis (NORMAL, MODERADO ou CRÍTICO) e publica as informações via protocolo MQTT. Quando a condição CRÍTICA é detectada, o sistema aciona automaticamente um módulo relé representando um ventilador DC.

O projeto está relacionado ao **ODS 11 — Cidades e Comunidades Sustentáveis**.

---

## Hardware utilizado

| Componente | Função | Pino ESP32 |
|---|---|---|
| ESP32 DevKit C V4 | Processamento e comunicação | — |
| Sensor MQ-135 (potenciômetro na simulação) | Leitura da qualidade do ar | GPIO34 |
| Módulo Relé | Acionamento do ventilador DC | GPIO26 |
| LED vermelho | Indicador visual do acionamento | GPIO27 |
| Resistor 220Ω | Proteção do LED | — |

---

## Software e protocolo

- **Linguagem:** C++ (Arduino IDE)
- **Bibliotecas:** WiFi.h, PubSubClient
- **Protocolo:** MQTT
- **Broker:** broker.hivemq.com (público, porta 1883)
- **Client ID:** ESP32_Ponto_Onibus_Sidney

### Tópicos MQTT

| Tópico | Descrição | Valores |
|---|---|---|
| `qualidade_ar/ponto_onibus/leitura` | Valor ADC do sensor | 0 a 4095 |
| `qualidade_ar/ponto_onibus/status` | Classificação do ar | NORMAL, MODERADO, CRITICO |
| `qualidade_ar/ponto_onibus/atuador` | Estado do ventilador | LIGADO, DESLIGADO |

### Limiares de classificação

| Faixa ADC | Status | Atuador |
|---|---|---|
| < 2000 | NORMAL | DESLIGADO |
| 2000 – 2999 | MODERADO | DESLIGADO |
| ≥ 3000 | CRITICO | LIGADO |

---

## Como reproduzir

### Simulação no Wokwi

1. Acesse a simulação: [wokwi.com/projects/464845297652829185](https://wokwi.com/projects/464845297652829185)
2. Clique em ▶ Play
3. Gire o potenciômetro para simular variações na qualidade do ar
4. Observe o monitor serial com as publicações MQTT

### Hardware físico

1. Monte o circuito conforme a tabela de pinos acima
2. Instale a biblioteca **PubSubClient** na Arduino IDE
3. Abra o arquivo `esp32_mqtt_mq135_final.ino`
4. Substitua o SSID e senha da rede Wi-Fi (linhas `ssid` e `password`)
5. Faça o upload para a ESP32
6. Monitore os tópicos MQTT no broker.hivemq.com

---

## Estrutura do repositório

```
monitoramento-qualidade-ar-iot/
│
├── esp32_mqtt_mq135_final.ino   # Firmware completo da ESP32
└── README.md                    # Documentação do projeto
```

---

## Comunicação MQTT

O sistema utiliza o protocolo MQTT (OASIS MQTT Version 3.1.1) no modelo publicação/assinatura. A ESP32 conecta-se ao broker público HiveMQ via Wi-Fi e publica os dados a cada **3 segundos** nos três tópicos definidos.

Para visualizar as mensagens em tempo real, acesse o [HiveMQ Websocket Client](https://www.hivemq.com/demos/websocket-client/) e assine o tópico `qualidade_ar/ponto_onibus/#`.

---

## Referências

- ESPRESSIF SYSTEMS. ESP32-DevKitC User Guide. Disponível em: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/user_guide.html
- OASIS. MQTT Version 3.1.1. Disponível em: https://www.oasis-open.org/standard/mqttv3-1-1/
- WINSEN. MQ-135 Gas Sensor Manual. Disponível em: https://www.winsen-sensor.com/d/files/PDF/Semiconductor%20Gas%20Sensor/MQ135%20%28Ver1.4%29%20-%20Manual.pdf
- KNOLLEARY. PubSubClient. Disponível em: https://pubsubclient.knolleary.net/
- HIVEMQ. Public MQTT Broker. Disponível em: https://www.hivemq.com/public-mqtt-broker/
- WOKWI. Online Arduino and ESP32 Simulator. Disponível em: https://wokwi.com/
