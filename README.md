# [🤖] Oficina de Integração em Robótica no LabSEA

*_"Exemplo prático de comunicação entre Python (no espaço inteligente LabSEA) e ESP32 via MQTT, para a Oficina de Integração em Robótica."_*

*_Autor: Equipe LabSEA_* [![GitHub](https://img.shields.io/badge/GitHub-%2312100E.svg?style=flat&logo=github&logoColor=white)](https://github.com/labsea)

---

## 📌 Visão Geral
Este projeto demonstra, de forma prática, o funcionamento da comunicação **Pub/Sub via MQTT** entre um código **Python rodando no espaço inteligente do LabSEA** e uma **placa ESP32**.  

Ele foi desenvolvido para ser usado na **Oficina de Integração em Robótica (LabSEA)**, mostrando como comandos simples podem ser enviados pelo Wi-fi para a ESP32 e como esta pode responder com informações de status (ex.: detecção de obstáculos).


O fluxo de dados envolve:
- 📥 **Entrada**: Pressionamento das teclas de direção no computador (↑ e ↓).  
- ⚙️ **Processamento**: Tradução do comando em mensagens MQTT e envio via broker RabbitMQ.  
- 📤 **Saída**:  
  - Comandos de movimento publicados no tópico `servo.cmd`.  
  - Status da ESP32 publicado no tópico `servo.status` (ex.: obstáculo detectado/liberado).  

---

## ⚙️ Arquitetura e Conceitos Aplicados

- **Linguagem**: `Python 3.9` e `Arduino (C++ para ESP32)`
- **Frameworks Principais**: `pynput`, `is-wire`
- **Mensageria**: `RabbitMQ (via MQTT, protocolo pub/sub)`
- **Processamento/Lógica**: Controle de servo motor + detecção de obstáculos (ESP32)
- **Infraestrutura de Execução**: Localmente no **PC do espaço inteligente LabSEA** + ESP32 conectada no Wi-fi

---

## 📡 Tópicos de Comunicação

**Subscrições (Entradas)**  
- `servo.cmd`: Comandos de movimento enviados do PC → ESP32 (`forward`, `backward`, `stop`).

**Publicações (Saídas)**  
- `servo.status`: Status da ESP32 → PC (`obstacle_detected`, `obstacle_cleared`).

---

## ✅ Requisitos e Dependências
### Python
Instale as dependências:
```bash
pip install pynput is-wire
```

### ESP32
Instale as dependências e configure:
- Biblioteca PubSubClient
- Biblioteca WiFi.h (já inclusa na ESP32)
- Biblioteca ESP32Servo.h
- Credenciais da rede Wi-Fi do LabSEA
- Endereço do broker MQTT (RabbitMQ)

---

## 📂 Estrutura de diretórios esperada

```
projeto_labsea_mqtt/
├── exemplo.py            # Código Python principal (envia e recebe mensagens MQTT)
├── streamChannel.py      # Classe auxiliar para consumir mensagens MQTT
└── exemplo_esp32.ino     # Código para ESP32 (recebe comandos e envia status)
```

---

## 🛠️ Execução Local

1. Clone o repositório:
```bash
git clone [URL_DO_REPOSITÓRIO]
cd projeto_labsea_mqtt
```

2. Instale as dependências:
```bash
pip install pynput is-wire
```

3. Configure a ESP32 com o código exemplo_esp32.ino, ajustando Wi-Fi e broker.

4. Execute a aplicação:
```bash
python3 exemplo.py
```

5. Use as teclas ↑ e ↓ para mover o servo.
- Mensagens do status da ESP32 aparecerão no terminal.

---

## ❗ Solução de Problemas Comuns

| Sintoma | Causa Provável / Solução |
|--------|----------------------------|
| PC não conecta ao broker | Verifique se o RabbitMQ está ativo e acessível no LabSEA. |
| ESP32 não publica mensagens | Confira se as credenciais do Wi-Fi e o endereço do broker estão corretos no código exemplo_esp32.ino. |
| Comandos não chegam na ESP32 | Certifique-se de que os tópicos usados (servo.cmd, servo.status) estão corretos e que a ESP32 está inscrita. |

---

## 📬 Contato

Para dúvidas, sugestões ou contribuições, entre em contato com a equipe do **LabSEA**.  
[![Email](https://img.shields.io/badge/Email-%234285F4.svg?style=flat&logo=gmail&logoColor=white)](mailto:labsea.gua@ifes.edu.br)<br>
[![Instagram](https://img.shields.io/badge/Instagram-%23E4405F.svg?style=flat&logo=instagram&logoColor=white)](https://www.instagram.com/labsea.gua/)<br>
[![YouTube](https://img.shields.io/badge/YouTube-%23FF0000.svg?style=flat&logo=youtube&logoColor=white)](https://www.youtube.com/channel/UCpiTMhUtKi3W7QnoOSL9UsQ)<br>

---
 
