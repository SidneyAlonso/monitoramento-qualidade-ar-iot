/*
 * ============================================================
 * Sistema de Monitoramento da Qualidade do Ar em Pontos de Ônibus
 * Universidade Presbiteriana Mackenzie — Faculdade de Computação e Informática
 *
 * Autores: Sidney Quiota Alonso, André Luis de Oliveira, Leandro Carlos Fernandes
 * Ano: 2026
 *
 * Hardware:
 *   - ESP32 DevKit C V4
 *   - Sensor MQ-135 (simulado por potenciômetro no GPIO34)
 *   - Módulo Relé (GPIO26) — representa o ventilador DC
 *   - LED vermelho (GPIO27) — indicador visual do acionamento
 *
 * Protocolo: MQTT via Wi-Fi
 * Broker: broker.hivemq.com (público, porta 1883)
 * Rede Wi-Fi: Wokwi-GUEST (simulação no Wokwi)
 *
 * Tópicos MQTT:
 *   qualidade_ar/ponto_onibus/leitura  → valor ADC do sensor (0–4095)
 *   qualidade_ar/ponto_onibus/status   → "NORMAL", "MODERADO" ou "CRITICO"
 *   qualidade_ar/ponto_onibus/atuador  → "LIGADO" ou "DESLIGADO"
 *
 * Limiares de classificação:
 *   < 2000          → NORMAL   (LED apagado, relé desligado)
 *   >= 2000 < 3000  → MODERADO (LED apagado, relé desligado)
 *   >= 3000         → CRITICO  (LED aceso, relé acionado)
 *
 * Simulação: https://wokwi.com/projects/464845297652829185
 * Repositório: https://github.com/SidneyAlonso/monitoramento-qualidade-ar-iot
 * ============================================================
 */

#include <WiFi.h>
#include <PubSubClient.h>

// ─── Pinos ────────────────────────────────────────────────────────────────────
#define PINO_POT  34   // Potenciômetro (representa sensor MQ-135) — entrada analógica
#define PINO_RELE 26   // Módulo relé — saída digital
#define PINO_LED  27   // LED vermelho indicador — saída digital

// ─── Configurações Wi-Fi ──────────────────────────────────────────────────────
const char* ssid     = "Wokwi-GUEST";
const char* password = "";

// ─── Configurações MQTT ───────────────────────────────────────────────────────
const char* mqtt_server   = "broker.hivemq.com";
const int   mqtt_port     = 1883;
const char* mqtt_clientId = "ESP32_Ponto_Onibus_Sidney";

// ─── Tópicos MQTT ─────────────────────────────────────────────────────────────
const char* topicoLeitura = "qualidade_ar/ponto_onibus/leitura";
const char* topicoStatus  = "qualidade_ar/ponto_onibus/status";
const char* topicoAtuador = "qualidade_ar/ponto_onibus/atuador";

// ─── Limiares de classificação (valores ADC 0–4095) ───────────────────────────
const int limiteModerado = 2000;
const int limiteCritico  = 3000;

// ─── Intervalo de publicação ──────────────────────────────────────────────────
const unsigned long INTERVALO = 3000;  // 3 segundos

// ─── Objetos globais ──────────────────────────────────────────────────────────
WiFiClient   espClient;
PubSubClient client(espClient);

// ─── Funções auxiliares ───────────────────────────────────────────────────────

/**
 * Conecta à rede Wi-Fi e aguarda a conexão ser estabelecida.
 */
void conectarWiFi() {
  Serial.print("[WiFi] Conectando à rede: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("[WiFi] Conectado! IP: ");
  Serial.println(WiFi.localIP());
}

/**
 * Conecta ao broker MQTT. Tenta reconectar em caso de falha.
 */
void conectarMQTT() {
  while (!client.connected()) {
    Serial.print("[MQTT] Conectando ao broker ");
    Serial.print(mqtt_server);
    Serial.print("...");

    if (client.connect(mqtt_clientId)) {
      Serial.println(" conectado!");
    } else {
      Serial.print(" falhou (rc=");
      Serial.print(client.state());
      Serial.println("). Tentando em 3s...");
      delay(3000);
    }
  }
}

/**
 * Publica uma mensagem em um tópico MQTT e exibe no monitor serial.
 */
void publicarMQTT(const char* topico, String mensagem) {
  client.publish(topico, mensagem.c_str());

  Serial.print("[MQTT] Tópico: ");
  Serial.print(topico);
  Serial.print(" -> ");
  Serial.println(mensagem);
}

// ─── Setup ────────────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);

  // Configuração dos pinos de saída
  pinMode(PINO_RELE, OUTPUT);
  pinMode(PINO_LED,  OUTPUT);

  // Estado inicial: relé desligado, LED apagado
  digitalWrite(PINO_RELE, HIGH);
  digitalWrite(PINO_LED,  LOW);

  Serial.println();
  Serial.println("=== Sistema de Monitoramento da Qualidade do Ar ===");
  Serial.println("=== Mackenzie IoT — ESP32 + MQ-135 + MQTT         ===");

  // Conexões
  conectarWiFi();

  client.setServer(mqtt_server, mqtt_port);
  conectarMQTT();

  Serial.println("[Setup] Sistema iniciado. Monitorando...");
}

// ─── Loop principal ───────────────────────────────────────────────────────────

void loop() {
  // Mantém conexão MQTT ativa
  if (!client.connected()) {
    Serial.println("[MQTT] Conexão perdida. Reconectando...");
    conectarMQTT();
  }
  client.loop();

  // Leitura do sensor (potenciômetro simulando MQ-135)
  int leitura = analogRead(PINO_POT);

  // Classificação da qualidade do ar e controle do atuador
  String status;
  String atuador;

  if (leitura >= limiteCritico) {
    status  = "CRITICO";
    atuador = "LIGADO";
    digitalWrite(PINO_LED,  HIGH);  // Acende LED
    digitalWrite(PINO_RELE, LOW);   // Aciona relé (lógica invertida)

  } else if (leitura >= limiteModerado) {
    status  = "MODERADO";
    atuador = "DESLIGADO";
    digitalWrite(PINO_LED,  LOW);   // Apaga LED
    digitalWrite(PINO_RELE, HIGH);  // Desliga relé

  } else {
    status  = "NORMAL";
    atuador = "DESLIGADO";
    digitalWrite(PINO_LED,  LOW);   // Apaga LED
    digitalWrite(PINO_RELE, HIGH);  // Desliga relé
  }

  // Publicação dos dados nos tópicos MQTT
  Serial.println("--------------------------------");
  publicarMQTT(topicoLeitura, String(leitura));
  publicarMQTT(topicoStatus,  status);
  publicarMQTT(topicoAtuador, atuador);
  Serial.println("--------------------------------");

  // Aguarda o próximo ciclo
  delay(INTERVALO);
}
