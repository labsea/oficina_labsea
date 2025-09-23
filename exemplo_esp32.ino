#include <WiFi.h>          // Para gerenciar a conexão Wi-Fi.
#include <PubSubClient.h>  // Para comunicação via MQTT (o protocolo de mensagens).
#include <ESP32Servo.h>    // Para controlar o servo motor.

// --- Configurações da Rede Wi-Fi ---
const char* ssid = "LabSEA-2.4GHz";      // Nome da rede Wi-Fi que o ESP32 vai se conectar.
const char* password = "senha";          // Senha da rede Wi-Fi.

// --- Configurações do Broker MQTT ---
const char* mqtt_server = "10.10.2.211"; // Endereço IP do servidor que gerencia as mensagens.
const int mqtt_port = 30001;             // Porta de comunicação com o servidor MQTT.
const char* mqtt_user = "guest";         // Nome de usuário para se conectar ao servidor (se necessário).
const char* mqtt_password = "guest";     // Senha para se conectar ao servidor (se necessário).

// --- Tópicos MQTT (Canais de Comunicação) ---
// Declarar os tópicos aqui facilita a manutenção do código.
const char* topic1 = "servo.cmd";       // Canal para RECEBER comandos (ex: "forward", "stop").
const char* topic2 = "servo.status";    // Canal para ENVIAR o status (ex: "obstacle_detected").

// --- Objetos de Rede ---
WiFiClient espClient;             // Cria um cliente para se conectar à rede.
PubSubClient client(espClient);   // Usa o cliente de rede para criar um cliente MQTT.

// --- Mapeamento dos Pinos Físicos ---
const int PIN_SERVO = 23;      // O pino 23 está conectado ao servo motor.
const int PIN_TRIG = 14;       // O pino 14 (Trigger) envia o pulso do sensor ultrassônico.
const int PIN_ECHO = 12;       // O pino 12 (Echo) recebe o retorno do pulso do sensor.
const int RED_LED = 5;         // O pino 5 está conectado ao LED vermelho.
const int GREEN_LED = 4;       // O pino 4 está conectado ao LED verde.

// --- Variáveis de Controle ---
Servo servo;                   // Cria um objeto para controlar nosso servo motor.
bool obstacle = false;         // Variável para guardar o estado: "true" se há um obstáculo, "false" se não há.
int servoPos = 90;             // Guarda a posição atual do servo. Começa em 90 graus (neutro).

long readDistance() {
  // Envia um pulso sonoro curto
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  // Mede o tempo que o som demorou para voltar
  long duration = pulseIn(PIN_ECHO, HIGH, 20000); // Mede a duração do eco em microssegundos.

  // Converte o tempo em distância (cm) usando a fórmula
  long distance = duration * 0.034 / 2;
  return distance;
}

void publishStatus(const char* msg) {
  // Publica a mensagem no canal de status que definimos no início.
  client.publish(topic2, msg);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String cmd;
  // Converte a mensagem recebida (que chega como uma sequência de bytes) para um texto (String)
  for (unsigned int i = 0; i < length; i++) cmd += (char)payload[i];

  Serial.printf("Recebido: %s\n", cmd.c_str()); // Mostra o comando recebido no monitor serial.

  // Verifica qual comando foi recebido e age de acordo
  if (cmd == "forward") { // Se o comando for "forward".
    if (!obstacle) { // E se NÃO houver um obstáculo
      servoPos = min(servoPos + 10, 180); // Aumenta a posição do servo, com limite em 180 graus.
      servo.write(servoPos);
      Serial.println("Servo: FORWARD");
    } else { // Se houver um obstáculo
      publishStatus("obstacle_detected"); // Avisa que está bloqueado.
      Serial.println("Bloqueado: obstáculo detectado");
    }
  }
  else if (cmd == "backward") { // Se o comando for "backward"
    servoPos = max(servoPos - 10, 0); // Diminui a posição do servo, com limite em 0 graus.
    servo.write(servoPos);
    Serial.println("Servo: BACKWARD");
  }
  else if (cmd == "stop") { // Se o comando for "stop"
    Serial.println("Servo: STOP");
  }
}

void reconnect() {
  while (!client.connected()) { // Enquanto não estiver conectado...
    Serial.print("Tentando se reconectar ao MQTT Broker...");
    // Tenta conectar usando o ID "ESP32Client" e as credenciais definidas
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado!");
      // Se conseguir conectar, se inscreve no tópico de comandos para poder ouvi-los.
      client.subscribe(topic1);
      Serial.println("Inscrito no tópico de comando.");
    } else {
      Serial.print("Falha, rc="); // Se falhar, mostra o código do erro.
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000); // Espera 5 segundos antes de tentar de novo.
    }
  }
}

void setup() {
  // Inicia a comunicação serial para vermos as mensagens no computador
  Serial.begin(115200);

  // Configura os pinos: quais são de saída (para enviar sinal) e quais são de entrada (para receber sinal)
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  // Garante que os LEDs comecem apagados
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  // "Anexa" o objeto servo ao pino físico correto e o move para a posição inicial
  servo.attach(PIN_SERVO);
  servo.write(servoPos);

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("WiFi conectado");

  // Configura o cliente MQTT com o endereço do servidor e a função de callback
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Diz ao cliente qual função chamar quando uma mensagem chegar.
}

void loop() {
  // 1. Garante que a conexão MQTT está ativa
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Permite que o cliente MQTT processe mensagens de entrada/saída.

  // 2. Lê a distância do sensor
  long dist = readDistance();

  // 3. Lógica de detecção de obstáculo
  if (dist > 0 && dist < 15 && !obstacle) { // Se a distância for menor que 15cm e antes não havia obstáculo...
    obstacle = true; // ...agora há um obstáculo.
    publishStatus("obstacle_detected"); // Avisa a todos sobre o obstáculo.
    digitalWrite(RED_LED, HIGH);   // Acende o LED vermelho.
    digitalWrite(GREEN_LED, LOW);  // Apaga o LED verde.
    Serial.println("Obstáculo detectado!");
  } else if ((dist >= 15 || dist == 0) && obstacle) { // Se a distância for maior que 15cm e antes havia um obstáculo...
    obstacle = false; // ...agora não há mais obstáculo.
    publishStatus("obstacle_cleared"); [cite_start]// Avisa que o caminho está livre. [cite: 23]
    digitalWrite(RED_LED, LOW);    // Apaga o LED vermelho.
    digitalWrite(GREEN_LED, HIGH); // Acende o LED verde.
    Serial.println("Obstáculo liberado");
  }

  // 4. Pequena pausa para não sobrecarregar o processador
  delay(100); // Espera 100 milissegundos antes de começar o loop novamente.
}
