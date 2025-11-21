# arduino-projeto-respiracao-mqtt
Projeto Arduino Uno + ESP8266 + HC-SR04 com comunicação MQTT

#Intellecta

Este trabalho apresenta o desenvolvimento de um sistema de **respiração guiada** baseado em conceitos da **Internet das Coisas (IoT)**, utilizando um kit de hardware composto por uma placa Uno R3 compatível com Arduino, LEDs de diferentes cores, sensores e componentes eletrônicos básicos.  

O sistema tem como objetivo auxiliar os usuários em práticas de **relaxamento e redução do estresse**, em alinhamento com o **Objetivo de Desenvolvimento Sustentável 3 da ONU (Saúde e Bem-estar)**.  

O protótipo utiliza LEDs para representar as fases do ciclo respiratório — **inspirar, segurar e expirar** — oferecendo uma solução acessível e de baixo custo para a promoção da saúde mental, explorando tecnologias embarcadas e recursos da IoT.

---

#Lista de componentes

- 1 × Placa **Arduino Uno R3**  
- 1 × Módulo **ESP8266** (Wi‑Fi)  
- 1 × Sensor ultrassônico **HC‑SR04**  
- 3 × LEDs (verde, vermelho e azul)  
- 3 × Resistores de 150 Ω  
- 1 × Protoboard  
- Jumpers diversos  
- Cabo USB para conexão ao computador  

#Resultados
O sistema detecta presença a menos de 20 cm com o sensor HC‑SR04.
Após a detecção, os LEDs são acionados em sequência para guiar o ciclo respiratório:
Verde (Inspirar) → 5 segundos
Vermelho (Segurar) → 2 segundos
Azul (Expirar) → 4 segundos
Vermelho (Segurar) → 2 segundos
Ao final do ciclo, o Arduino envia ao ESP8266 a mensagem MQTT “RESPIRAÇÃO COMPLETA”.
A mensagem é publicada no broker Mosquitto (test.mosquitto.org:1883), no tópico respiracao/teste.
Clientes externos (ex.: HiveMQ Web Client) conseguem assinar o tópico e visualizar a mensagem publicada.

#Conclusão
O protótipo Intellecta demonstrou a viabilidade da integração entre hardware simples e protocolos de rede para aplicações em Internet das Coisas (IoT).
O sistema cumpriu o requisito de comunicação via MQTT, eliminando a necessidade de ponte pelo notebook e garantindo autonomia na publicação de dados. Além disso, mostrou-se confiável na detecção de presença e na execução do ciclo de LEDs, oferecendo uma solução acessível e de baixo custo para práticas de relaxamento e promoção da saúde mental.
Conclui-se que a proposta pode ser expandida para incluir autenticação segura, brokers privados e integração com plataformas de análise em nuvem, consolidando sua aplicabilidade em cenários reais de monitoramento e bem-estar.

---

## Código principal (Arduino)

```cpp
#include <SoftwareSerial.h>

// Comunicação com ESP8266
SoftwareSerial esp(2, 3); // RX = 2, TX = 3

// Pinos dos LEDs
const int ledVerde = 10;
const int ledVermelho = 11;
const int ledAzul = 12;

// Sensor HC-SR04
const int trigPin = 7;
const int echoPin = 8;

void setup() {
  Serial.begin(9600);
  esp.begin(115200);

  pinMode(ledVerde, OUTPUT);
  pinMode(ledVermelho, OUTPUT);
  pinMode(ledAzul, OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  Serial.println("=== Sistema iniciado ===");

  // Conexão ESP8266
  enviarComando("AT", 2000);
  enviarComando("AT+RST", 2000);
  enviarComando("AT+CWMODE=1", 1000);
  enviarComando("AT+CWJAP=\"POCO X3 PRO\",\"1234568G\"", 8000);
  enviarComando("AT+CIPSTART=\"TCP\",\"test.mosquitto.org\",1883", 5000);

  // Pacote CONNECT MQTT
  enviarComando("AT+CIPSEND=18", 1000);
  esp.print("\x10\x10\x00\x04MQTT\x04\x02\x00<\x00\x03UNO");
}

void loop() {
  long distancia = medirDistancia();

  if (distancia > 0 && distancia < 20) {
    delay(5000);

    acenderLED(ledVerde, 5000, "Verde = Inspirar");
    acenderLED(ledVermelho, 2000, "Vermelho = Segurar");
    acenderLED(ledAzul, 4000, "Azul = Expirar");
    acenderLED(ledVermelho, 2000, "Vermelho = Segurar");

    // Publicar mensagem MQTT
    String msg = "\x30\x28\x00\x11respiracao/testeRESPIRAÇÃO COMPLETA";
    enviarComando("AT+CIPSEND=" + String(msg.length()), 1000);
    esp.print(msg);

    Serial.println(">>> Mensagem MQTT enviada: RESPIRAÇÃO COMPLETA");
  }

  delay(500);
}

// Funções auxiliares
void acenderLED(int pino, int tempo, String nome) {
  digitalWrite(pino, HIGH);
  Serial.println("LED ligado: " + nome);
  delay(tempo);
  digitalWrite(pino, LOW);
  Serial.println("LED desligado: " + nome);
}

long medirDistancia() {
  digitalWrite(trigPin, LOW); delayMicroseconds(5);
  digitalWrite(trigPin, HIGH); delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duracao = pulseIn(echoPin, HIGH, 60000);
  if (duracao == 0) return -1;
  return duracao * 0.034 / 2;
}

void enviarComando(String cmd, int tempo) {
  esp.println(cmd);
  delay(tempo);
  while (esp.available()) {
    Serial.write(esp.read());
  }
}
