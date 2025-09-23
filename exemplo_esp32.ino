#include <WiFi.h>          // Para gerenciar a conexão Wi-Fi.
#include <PubSubClient.h>  // Para comunicação via MQTT (o protocolo de mensagens).
#include <ESP32Servo.h>    // Para controlar o servo motor.

// --- Configurações da Rede Wi-Fi ---
const char* ssid = "LabSEA-2.4GHz"; // Nome da rede Wi-Fi que o ESP32 vai se conectar.
const char* password = "senha"; // Senha da rede Wi-Fi.

// --- Configurações do Broker MQTT ---
const char* mqtt_server = "10.10.2.211"; // Endereço IP do servidor que gerencia as mensagens.
const int mqtt_port = 30001; // Porta de comunicação com o servidor MQTT.
const char* mqtt_user = "guest"; // Nome de usuário para se conectar ao servidor (se necessário).
const char* mqtt_password = "guest"; // Senha para se conectar ao servidor (se necessário).

// --- Tópicos MQTT (Canais de Comunicação) ---
const char* topic_command = "servo.cmd";       // Canal para RECEBER comandos (ex: "forward", "stop").
const char* topic_status = "servo.status"; // Canal para ENVIAR o status (ex: "obstacle_detected").

// --- Mapeamento dos Pinos Físicos ---
const int PIN_SERVO = 23;      // O pino 23 está conectado ao servo motor.
const int PIN_TRIG = 14;       // O pino 14 (Trigger) envia o pulso do sensor ultrassônico.
const int PIN_ECHO = 12;       // O pino 12 (Echo) recebe o retorno do pulso do sensor.
const int RED_LED = 5;         // O pino 5 está conectado ao LED vermelho.
const int GREEN_LED = 4;       // O pino 4 está conectado ao LED verde.

// --- PARÂMETROS PARA SERVO DE ROTAÇÃO CONTÍNUA ---
// Valores que controlam a velocidade e direção. 90 é o ponto neutro (parado).
// Você pode ajustar os valores de velocidade para mais perto ou mais longe de 90.
const int SERVO_STOP = 95;
const int SERVO_FORWARD_SPEED = 120; // Velocidade para frente (valores > 90)
const int SERVO_BACKWARD_SPEED = 60; // Velocidade para trás (valores < 90)

// --- Objetos e Variáveis de Controle ---
WiFiClient espClient; // Cria um cliente para se conectar à rede.
PubSubClient client(espClient); // Usa o cliente de rede para criar um cliente MQTT.
Servo servo; // Cria um objeto para controlar nosso servo motor.
bool obstacle = false; // Variável para guardar o estado: "true" se há um obstáculo, "false" se não há.


long readDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duration = pulseIn(PIN_ECHO, HIGH, 20000); // Mede a duração do eco em microssegundos.
  long distance = duration * 0.034 / 2;
  return distance;
}

void publishStatus(const char* msg) {
  client.publish(topic_status, msg); // Publica a mensagem no canal de status.
}

// --- LÓGICA DE COMANDOS ATUALIZADA ---
void callback(char* topic, byte* payload, unsigned int length) {
  String cmd;
  // Converte a mensagem recebida para um texto (String)
  for (unsigned int i = 0; i < length; i++) cmd += (char)payload[i];
  Serial.printf("Recebido: %s\n", cmd.c_str()); // Mostra o comando recebido no monitor serial.

  if (cmd == "forward") {
    if (!obstacle) { // E se NÃO houver um obstáculo
      servo.write(SERVO_FORWARD_SPEED); // Gira o servo para frente com velocidade definida.
      Serial.println("Servo: GIRANDO PARA FRENTE");
    } else {
      publishStatus("obstacle_detected"); // Avisa que está bloqueado.
      Serial.println("Bloqueado: obstáculo detectado");
    }
  }
  else if (cmd == "backward") {
    servo.write(SERVO_BACKWARD_SPEED); // Gira o servo para trás com velocidade definida.
    Serial.println("Servo: GIRANDO PARA TRÁS");
  }
  else if (cmd == "stop") {
    servo.write(SERVO_STOP); // Envia explicitamente o comando para PARAR o servo.
    Serial.println("Servo: PARADO");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando se reconectar ao MQTT Broker...");
    // Tenta conectar usando o ID "ESP32Client" e as credenciais definidas
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado!");
      // Se conseguir conectar, se inscreve no tópico de comandos para poder ouvi-los.
      client.subscribe(topic_command);
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
  Serial.begin(115200);
  // Configura os pinos de saída e entrada
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  
  servo.attach(PIN_SERVO); // "Anexa" o objeto servo ao pino físico correto.
  servo.write(SERVO_STOP); // GARANTE que o servo comece PARADO.
  Serial.println("Servo iniciado em estado PARADO.");

  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("WiFi conectado");
  
  // Configura o cliente MQTT com o endereço do servidor e a função de callback
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback); // Diz ao cliente qual função chamar quando uma mensagem chegar.
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop(); // Permite que o cliente MQTT processe mensagens.
  
  long dist = readDistance(); // Lê a distância do sensor
  
  if (dist > 0 && dist < 15 && !obstacle) {
    obstacle = true; // ...agora há um obstáculo.
    servo.write(SERVO_STOP); // Adicionado: Para o servo por segurança.
    publishStatus("obstacle_detected"); // Avisa sobre o obstáculo.
    digitalWrite(RED_LED, HIGH);   // Acende o LED vermelho.
    digitalWrite(GREEN_LED, LOW);  // Apaga o LED verde.
    Serial.println("Obstáculo detectado!");
  } else if ((dist >= 15 || dist == 0) && obstacle) {
    obstacle = false; // ...agora não há mais obstáculo.
    publishStatus("obstacle_cleared"); // Avisa que o caminho está livre.
    digitalWrite(RED_LED, LOW);    // Apaga o LED vermelho.
    digitalWrite(GREEN_LED, HIGH); // Acende o LED verde.
    Serial.println("Obstáculo liberado");
  }

  delay(100); // Espera 100 milissegundos antes de começar o loop novamente.
}
