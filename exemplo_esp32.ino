#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ===== CONFIG WIFI + MQTT =====
const char* ssid = "LabSEA-2.4GHz";
const char* password = "senha";
const char* mqtt_server = "10.10.2.211"; // broker
const int mqtt_port = 30001;
const char* mqtt_user = "guest";
const char* mqtt_password = "guest";

const int RED_LED = 5;
const int GREEN_LED = 4;
const char* topic1 = "servo.cmd";

WiFiClient espClient;
PubSubClient client(espClient);

// ===== PINOS =====
const int PIN_SERVO = 23;
const int PIN_TRIG = 14;
const int PIN_ECHO = 12;

Servo servo;
bool obstacle = false;
int servoPos = 90;  // posição neutra (0–180)

long readDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duration = pulseIn(PIN_ECHO, HIGH, 20000);
  long distance = duration * 0.034 / 2; // cm
  return distance;
}

void publishStatus(const char* msg) {
  client.publish("servo.status", msg);
}

void callback(char* topic, byte* payload, unsigned int length) {
  String cmd;
  for (unsigned int i = 0; i < length; i++) cmd += (char)payload[i];
  Serial.printf("Recebido: %s\n", cmd.c_str());

  if (cmd == "forward") {
    if (!obstacle) {
      servoPos = min(servoPos + 10, 180); // incrementa posição
      servo.write(servoPos);
      Serial.println("Servo: FORWARD");
    } else {
      publishStatus("obstacle_detected");
      Serial.println("Bloqueado: obstáculo detectado");
    }
  } 
  else if (cmd == "backward") {
    servoPos = max(servoPos - 10, 0); // decrementa posição
    servo.write(servoPos);
    Serial.println("Servo: BACKWARD");
  } 
  else if (cmd == "stop") {
    // opcional: manter posição
    Serial.println("Servo: STOP");
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando se reconectar ao MQTT Broker...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Conectado!");
      // Inscreve-se nos dois tópicos
      client.subscribe(topic1);
      Serial.println("Inscrito nos tópicos:");
      Serial.println(topic1);
    } else {
      Serial.print("Falha, rc=");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  servo.attach(PIN_SERVO);
  servo.write(servoPos);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("WiFi conectado");

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  long dist = readDistance();
  if (dist > 0 && dist < 15 && !obstacle) {
    obstacle = true;
    publishStatus("obstacle_detected");
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    Serial.println("Obstáculo detectado!");
  } else if ((dist >= 15 || dist == 0) && obstacle) {
    obstacle = false;
    publishStatus("obstacle_cleared");
    digitalWrite(RED_LED, LOW);
    digitalWrite(GREEN_LED, HIGH);
    Serial.println("Obstáculo liberado");
  }

  delay(100);
}
