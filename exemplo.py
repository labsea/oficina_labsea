from pynput import keyboard
from is_wire.core import Message, Subscription
from streamChannel import StreamChannel

broker_uri = "amqp://guest:guest@10.10.2.211:30000"
channel = StreamChannel(broker_uri)

# Envia comando para ESP32
def enviar_comando(cmd):
    msg = Message()
    msg.body = cmd.encode("utf-8")
    channel.publish(msg, topic="servo.cmd")
    print(f"[PC] Enviado comando: {cmd}")

# Recebe status do ESP32 (obstáculo detectado/liberado)
subscription = Subscription(channel=channel)
subscription.subscribe(topic="servo.status")

def on_press(key):
    try:
        if key == keyboard.Key.up:
            enviar_comando("forward")
        elif key == keyboard.Key.down:
            enviar_comando("backward")
    except Exception as e:
        print("Erro:", e)


def on_release(key):
    if key in [keyboard.Key.up, keyboard.Key.down]:
        enviar_comando("stop")
    if key == keyboard.Key.esc:
        return False  # encerra programa

print("Use ↑ e ↓ para controlar o servo. ESC para sair.")

listener = keyboard.Listener(on_press=on_press, on_release=on_release)
listener.start()

# Loop para receber mensagens do ESP32
while True:
    msg = channel.consume()
    if msg:
        status = msg.body.decode()
        if status == "obstacle_detected":
            print("⚠️ Obstáculo detectado! Movimento para frente bloqueado.")
        elif status == "obstacle_cleared":
            print("✅ Obstáculo removido, movimento liberado.")
