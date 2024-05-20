import pygame
import socket
import sys
import tkinter as tk

# Configuración del servidor
SERVER_HOST = "localhost"  # Cambiar a la dirección IP del servidor si es remoto
SERVER_PORT = 5060
BUFFER_SIZE = 1024

# Inicializar Pygame
pygame.init()
pygame.font.init()

# Colores
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
LIGHT_GRAY = (200, 200, 200)
GREEN = (0, 200, 0)

# Configuración de la pantalla de inicio
WIDTH, HEIGHT = 500, 400
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Autenticación de Usuario")

# Fuentes
font = pygame.font.SysFont(None, 30)
small_font = pygame.font.SysFont(None, 20)

def draw_text(text, font, color, surface, x, y):
    textobj = font.render(text, 1, color)
    textrect = textobj.get_rect()
    textrect.topleft = (x, y)
    surface.blit(textobj, textrect)

def chat_window(username, grupos):
    def enviar_mensaje():
        mensaje = entrada_mensaje.get()
        grupo_seleccionado = lista_grupos.get(tk.ACTIVE)
        if grupo_seleccionado in grupos:
            mensaje_formateado = f"{username}:{mensaje}"
            grupos[grupo_seleccionado].append(mensaje_formateado)
            actualizar_mensajes(grupo_seleccionado)
            mensaje_formateado = f"Nuevo Mensaje:{grupo_seleccionado}:{username}:{mensaje}"
            enviar_mensaje_servidor(mensaje_formateado)

    def actualizar_mensajes(grupo):
        lista_mensajes.delete(0, tk.END)
        if grupo in grupos:
            for mensaje in grupos[grupo]:
                lista_mensajes.insert(tk.END, mensaje)

    def crear_grupo():
        nombre_grupo = entrada_grupo.get()
        if nombre_grupo:
            lista_grupos.insert(tk.END, nombre_grupo)
            grupos[nombre_grupo] = []
            
            # Enviar mensaje al servidor indicando que se creó un nuevo grupo
            mensaje_servidor = f"Crear Grupo:{username}:{nombre_grupo}"
            enviar_mensaje_servidor(mensaje_servidor)

    def enviar_mensaje_servidor(mensaje):
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((SERVER_HOST, SERVER_PORT))
                s.sendall(mensaje.encode())
                response = s.recv(1024).decode()
                print("Respuesta del servidor:", response)
        except Exception as e:
            print("Error al conectar al servidor:", e)

    def agregar_usuario_a_grupo():
        usuario = entrada_usuario.get()
        grupo_seleccionado = lista_grupos.get(tk.ACTIVE)
        if grupo_seleccionado in grupos:
            mensaje_formateado = f"Agregar Usuario a Grupo:{grupo_seleccionado}:{username}:{usuario}"
            enviar_mensaje_servidor(mensaje_formateado)

    def actualizar():
        # Solicitar el archivo BD.txt al servidor
        try:
            with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                s.connect((SERVER_HOST, SERVER_PORT))
                s.sendall("Actualizar:".encode())

                # Recibir la longitud del archivo
                length_bytes = s.recv(4)
                if not length_bytes:
                    print("Error al recibir la longitud del archivo")
                    return
                length = int.from_bytes(length_bytes, byteorder='big')

                # Recibir el contenido del archivo
                data = b""
                remaining = length
                while remaining > 0:
                    part = s.recv(min(BUFFER_SIZE, remaining))
                    if not part:
                        print("Error al recibir el contenido del archivo")
                        break
                    data += part
                    remaining -= len(part)

                content = data.decode('utf-8')
                
                for line in content.split('\n'):
                    if line:
                        colon_index = line.find(':')
                        service = line[:colon_index].strip()
                        message = line[colon_index+1:].strip()
                        print(service, "\n")
                        print(message, "\n")

        except Exception as e:
            print("Error al conectar al servidor:", e)

    ventana = tk.Tk()
    ventana.title("Aplicación de Mensajería")
    ventana.geometry("800x600")

    lista_grupos = tk.Listbox(ventana)
    lista_grupos.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

    lista_mensajes = tk.Listbox(ventana)
    lista_mensajes.pack(side=tk.RIGHT, fill=tk.BOTH, expand=True)

    entrada_mensaje = tk.Entry(ventana)
    entrada_mensaje.pack(side=tk.BOTTOM, fill=tk.X)

    boton_enviar = tk.Button(ventana, text="Enviar", command=enviar_mensaje)
    boton_enviar.pack(side=tk.BOTTOM, fill=tk.X)

    entrada_usuario = tk.Entry(ventana)
    entrada_usuario.pack(side=tk.BOTTOM, fill=tk.X)

    boton_agregar_usuario = tk.Button(ventana, text="Agregar Usuario", command=agregar_usuario_a_grupo)
    boton_agregar_usuario.pack(side=tk.BOTTOM, fill=tk.X)


    entrada_grupo = tk.Entry(ventana)
    entrada_grupo.pack(side=tk.BOTTOM, fill=tk.X)

    boton_crear_grupo = tk.Button(ventana, text="Crear Grupo", command=crear_grupo)
    boton_crear_grupo.pack(side=tk.BOTTOM, fill=tk.X)

    boton_actualizar = tk.Button(ventana, text="Actualizar", command=actualizar)
    boton_actualizar.pack(side=tk.BOTTOM, fill=tk.X)

    for grupo in grupos.keys():
        lista_grupos.insert(tk.END, grupo)

    def on_select(event):
        widget = event.widget
        index = widget.curselection()[0]
        grupo_seleccionado = widget.get(index)
        actualizar_mensajes(grupo_seleccionado)

    lista_grupos.bind("<<ListboxSelect>>", on_select)

    ventana.mainloop()

def main():
    username = ""
    password = ""
    input_rect = pygame.Rect(200, 100, 250, 32)
    password_rect = pygame.Rect(200, 160, 250, 32)
    send_button_rect = pygame.Rect(200, 220, 100, 40)
    active_input = False
    active_password = False

    while True:
        screen.fill(WHITE)
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            if event.type == pygame.MOUSEBUTTONDOWN:
                if input_rect.collidepoint(event.pos):
                    active_input = True
                    active_password = False
                elif password_rect.collidepoint(event.pos):
                    active_password = True
                    active_input = False
                elif send_button_rect.collidepoint(event.pos):
                    try:
                        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
                            s.connect((SERVER_HOST, SERVER_PORT))
                            message = f"Autenticar Usuario:{username}:{password}"
                            s.sendall(message.encode())
                            response = s.recv(1024).decode()
                            print("Respuesta del servidor:", response)
                            if response == "1":
                                chat_window(username, {})
                                break
                    except Exception as e:
                        print("Error al conectar al servidor:", e)
                else:
                    active_input = False
                    active_password = False

            if event.type == pygame.KEYDOWN:
                if active_input:
                    if event.key == pygame.K_BACKSPACE:
                        username = username[:-1]
                    else:
                        username += event.unicode
                if active_password:
                    if event.key == pygame.K_BACKSPACE:
                        password = password[:-1]
                    else:
                        password += event.unicode

        draw_text("Usuario:", font, BLACK, screen, 50, 110)
        draw_text("Contraseña:", font, BLACK, screen, 50, 170)
        pygame.draw.rect(screen, LIGHT_GRAY, input_rect)
        pygame.draw.rect(screen, LIGHT_GRAY, password_rect)
        pygame.draw.rect(screen, GREEN, send_button_rect)
        draw_text(username, font, BLACK, screen, input_rect.x + 5, input_rect.y + 5)
        draw_text("*" * len(password), font, BLACK, screen, password_rect.x + 5, password_rect.y + 5)
        draw_text("Enviar", font, WHITE, screen, send_button_rect.x + 20, send_button_rect.y + 10)

        pygame.display.flip()

if __name__ == "__main__":
    main()
