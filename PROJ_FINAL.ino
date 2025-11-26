#include <WiFi.h>
#include <PubSubClient.h>

// ====== CONFIG Wi-Fi ======
const char* ssid     = "Wokwi-GUEST";
const char* password = "";

// ====== CONFIG MQTT ======
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_client_id = "esp32-respiracao-001";
const char* mqtt_topic = "respiracao/fase";  // <<< TOPICO QUE O CELULAR VAI ASSINAR

WiFiClient espClient;
PubSubClient client(espClient);

// ====== LEDs ======
#define LED_VERDE    27
#define LED_VERMELHO 14
#define LED_AZUL     12

// ====== Ultrassônico ======
#define TRIG 5
#define ECHO 18
const int DISTANCIA_MIN = 0;
const int DISTANCIA_MAX = 20;

// ====== Função: medir distância ======
long medirDistancia() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);

  long duracao = pulseIn(ECHO, HIGH, 30000);
  if (duracao == 0) return -1;

  return duracao * 0.034 / 2;
}

// ====== PUBLICAR NO MQTT ======
void publicaFase(String fase) {
  client.publish(mqtt_topic, fase.c_str());
  Serial.print("📤 Publicado no MQTT: ");
  Serial.println(fase);
}

// ====== CONECTAR AO MQTT ======
void reconnect() {
  while (!client.connected()) {
    Serial.print("Conectando ao MQTT... ");
    if (client.connect(mqtt_client_id)) {
      Serial.println("Conectado!");
    } else {
      Serial.print("Falhou, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 3s...");
      delay(3000);
    }
  }
}

// ====== SETUP ======
void setup() {
  Serial.begin(115200);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  // WiFi
  Serial.print("Conectando ao WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println(" Conectado!");

  client.setServer(mqtt_server, mqtt_port);

  Serial.println("===== SISTEMA DE RESPIRAÇÃO COM MQTT =====");
}

// ====== LOOP ======
void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  long distancia = medirDistancia();
  Serial.print("Distância: ");
  Serial.println(distancia);

  if (distancia > DISTANCIA_MIN && distancia < DISTANCIA_MAX) {

    digitalWrite(LED_VERDE, HIGH); delay(5000);
    digitalWrite(LED_VERDE, LOW);
    publicaFase("INSPIRAR");

    digitalWrite(LED_VERMELHO, HIGH); delay(2000);
    digitalWrite(LED_VERMELHO, LOW);
    publicaFase("SEGURAR");

    digitalWrite(LED_AZUL, HIGH); delay(4000);
    digitalWrite(LED_AZUL, LOW);
    publicaFase("EXPIRAR");

    digitalWrite(LED_VERMELHO, HIGH); delay(2000);
    digitalWrite(LED_VERMELHO, LOW);
    publicaFase("SEGURAR");
  }

  delay(200);
}

Coloca a atualização no Git do código por favor
