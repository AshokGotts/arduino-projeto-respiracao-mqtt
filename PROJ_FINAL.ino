#include <SoftwareSerial.h>

// ----- Comunicação com ESP8266 -----
SoftwareSerial esp(2, 3); // RX = 2, TX = 3

// ----- Pinos dos LEDs -----
const int ledVerde = 10;
const int ledVermelho = 11;
const int ledAzul = 12;

// ----- Sensor HC-SR04 -----
const int trigPin = 7;
const int echoPin = 8;

void setup() {
  Serial.begin(9600);
  esp.begin(115200); // ESP deve estar configurado para 9600 baud

  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.println("=== Teste Serial iniciado ===");

  // Teste ESP
  enviarComando("AT", 2000);
  Serial.println(">>> Teste ESP enviado: AT");

  // Reset ESP
  enviarComando("AT+RST", 2000);
  Serial.println(">>> ESP reiniciado");

  // Configurar modo estação
  enviarComando("AT+CWMODE=1", 1000);
  Serial.println(">>> ESP em modo estação");

  // Conectar ao Wi-Fi (POCO X3 PRO)
  enviarComando("AT+CWJAP=\"POCO X3 PRO\",\"1234568G\"", 8000);
  Serial.println(">>> Conectando ao Wi-Fi POCO X3 PRO...");

  // Conectar ao broker MQTT na porta 1883
  enviarComando("AT+CIPSTART=\"TCP\",\"test.mosquitto.org\",1883", 5000);
  Serial.println(">>> Conectando ao broker MQTT na porta 1883...");

  // Enviar pacote CONNECT MQTT
  enviarComando("AT+CIPSEND=18", 1000);
  esp.print("\x10\x10\x00\x04MQTT\x04\x02\x00<\x00\x03UNO");
  Serial.println(">>> Pacote CONNECT MQTT enviado");

  Serial.println("=== Sistema pronto. Aguardando presença... ===");
}

void loop() {
  long distancia = medirDistancia();

  if (distancia > 0 && distancia < 20) {
    Serial.print("Presença detectada a ");
    Serial.print(distancia);
    Serial.println(" cm. Aguardando 5 segundos...");
    delay(5000);

    acenderLED(ledVerde, 5000, "Verde = OK");
    acenderLED(ledVermelho, 2000, "Vermelho = OK");
    acenderLED(ledAzul, 4000, "Azul = OK");
    acenderLED(ledVermelho, 2000, "Vermelho = OK");

    // Publicar mensagem MQTT
    String msg = "\x30\x28\x00\x11respiracao/testeRESPIRAÇÃO COMPLETA";
    enviarComando("AT+CIPSEND=" + String(msg.length()), 1000);
    esp.print(msg);

    Serial.println(">>> Mensagem MQTT enviada: RESPIRAÇÃO COMPLETA\n");
  }

  delay(500);
}

// ----- Funções auxiliares -----
void acenderLED(int pino, int tempo, String nome) {
  digitalWrite(pino, HIGH);
  Serial.print("LED ligado: ");
  Serial.println(nome);
  delay(tempo);
  digitalWrite(pino, LOW);
  Serial.print("LED desligado: ");
  Serial.println(nome);
}

long medirDistancia() {
  digitalWrite(trigPin, LOW); delayMicroseconds(5);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracao = pulseIn(echoPin, HIGH, 60000);
  if (duracao == 0) {
    Serial.println(">>> Nenhum objeto detectado");
    return -1;
  }
  long cm = duracao * 0.034 / 2;
  Serial.print(">>> Distância medida: ");
  Serial.print(cm);
  Serial.println(" cm");
  return cm;
}

void enviarComando(String cmd, int tempo) {
  Serial.print(">> Enviando comando: ");
  Serial.println(cmd);
  esp.println(cmd);
  delay(tempo);
  while (esp.available()) {
    char c = esp.read();
    Serial.write(c);
  }
}
