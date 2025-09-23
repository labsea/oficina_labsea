from pynput import keyboard                    # Para "ouvir" e reagir às teclas pressionadas no teclado.
from is_wire.core import Message, Subscription # Ferramentas para criar e receber mensagens no formato correto.
from streamChannel import StreamChannel        # Módulo para gerenciar a conexão com o servidor de mensagens.

# --- Configurações ---
# Endereço do nosso "servidor de correio" (Broker). Note que a porta pode ser diferente da usada pelo ESP32.
BROKER_URI = "amqp://guest:guest@10.10.2.211:30000"

# Nomes dos "canais" de comunicação, definidos como constantes para fácil manutenção.
topic1 = "servo.cmd"       # Canal para ENVIAR comandos para o ESP32.
topic2 = "servo.status"    # Canal para RECEBER status do ESP32.

# Cria o canal de comunicação com o Broker, usando o endereço que definimos acima.
channel = StreamChannel(BROKER_URI)

# --- Função para Enviar um Comando ---
def enviar_comando(cmd):
    """Cria uma mensagem, codifica o comando para o formato de rede (bytes) e a publica no tópico de comando."""
    msg = Message()
    msg.body = cmd.encode("utf-8")           # Converte o texto (ex: "forward") para bytes.
    channel.publish(msg, topic=topic1)
    print(f"[PC] Enviado comando: {cmd}")

# --- Preparação para Receber Status ---
# Cria a "inscrição" para poder "ouvir" um tópico específico.
subscription = Subscription(channel=channel)
# Inscreve-se no canal de status. A partir daqui, o programa está pronto para receber mensagens enviadas pelo ESP32.
subscription.subscribe(topic=topic2)

def on_press(key):
    """Esta função é chamada automaticamente toda vez que uma tecla é PRESSIONADA."""
    try:
        if key == keyboard.Key.up:       # Se a tecla pressionada for a seta para cima...
            enviar_comando("forward")
        elif key == keyboard.Key.down:   # Se for a seta para baixo...
            enviar_comando("backward")
    except Exception as e:
        print("Erro:", e)

def on_release(key):
    """Esta função é chamada automaticamente toda vez que uma tecla é SOLTA."""
    if key in [keyboard.Key.up, keyboard.Key.down]:        # Se a tecla solta for uma das setas...
        enviar_comando("stop")                             # ...envia o comando para parar o servo.
    if key == keyboard.Key.esc:                            # Se a tecla solta for 'ESC'...
        return False                                       # ...sinaliza para o programa encerrar.


print("Use ↑ e ↓ para controlar o servo. ESC para sair.")

# Cria o "ouvinte" (listener) que vai monitorar o teclado.
# Ele conecta as ações do teclado (pressionar/soltar) com as nossas funções (on_press/on_release).
listener = keyboard.Listener(on_press=on_press, on_release=on_release)
# Inicia o monitoramento. A partir deste ponto, o programa está reagindo ao teclado.
listener.start()

# Este loop roda continuamente para verificar se alguma mensagem chegou do ESP32.
while True:
    # `channel.consume()` aguarda até que uma mensagem chegue no tópico em que nos inscrevemos (topic2).
    msg = channel.consume()

    # Se uma mensagem de fato chegou...
    if msg:
        status = msg.body.decode()                # Decodifica a mensagem de bytes de volta para texto.

        # Exibe uma mensagem amigável baseada no status recebido.
        if status == "obstacle_detected":
            print("⚠️ Obstáculo detectado! Movimento para frente bloqueado.")
        elif status == "obstacle_cleared":
            print("✅ Obstáculo removido, movimento liberado.")
