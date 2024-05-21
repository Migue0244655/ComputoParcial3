/*

	Lectura remota de una palabra para devolver el numero de vocales usando sockets pertenecientes
	a la familia TCP, en modo conexion.
	Codigo del servidor

	Nombre Archivo: tcpserver.c
	Archivos relacionados: num_vocales.h tcpclient.c 
	Fecha: Febrero 2023

	Compilacion: cc server.c -lnsl -o server
	Ejecución: ./tcpserver
*/	

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdbool.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

struct User {
    char username[32];
    char password[32];
};

struct User users[] = {
    {"user1", "password1"},
    {"user2", "password2"},
    {"user3", "password3"}
};

int authenticate(char *username, char *password) {
    int numUsers = sizeof(users) / sizeof(struct User);
    for (int i = 0; i < numUsers; i++) {
        if (strcmp(username, users[i].username) == 0 && strcmp(password, users[i].password) == 0) {
            return 1;
        }
    }
    return 0;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    char buffer[BUFFER_SIZE];
    const char *file_path = "BD.txt";

    // Crear el socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la direccion del servidor
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(5060);

    // Vincular el socket al puerto
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error al vincular el socket");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("Error al escuchar");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Servidor iniciado. Esperando conexiones...\n");

    while (1) {
        // Aceptar una conexión entrante
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (client_fd < 0) {
            perror("Error al aceptar la conexión");
            continue;
        }
        printf("Nueva conexión aceptada desde %s:%d\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pid_t child_pid = fork();

        if (child_pid != 0) {
            //Es el padre
        } 
        else{
            // Recibir las credenciales del cliente
            ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_received <= 0) {
                perror("Error al recibir datos");
                close(client_fd);
                continue;
            }
            buffer[bytes_received] = '\0';
            printf("Recibi: %s\n", buffer);

            // Separar el nombre de usuario y la contraseña
            char *servicio = strtok(buffer, ":");
            if(strcmp(servicio, "Autenticar Usuario") == 0){
                char *username = strtok(NULL, ":");
                char *password = strtok(NULL, ":");


                if (username == NULL || password == NULL) {
                    printf("Formato de credenciales inválido\n");
                    const char *response = "Formato de credenciales inválido";
                    send(client_fd, response, strlen(response), 0);
                    close(client_fd);
                    continue;
                }

                // Autenticar al usuario
                if (authenticate(username, password)) {
                    printf("Usuario autenticado: %s\n", username);
                    const char *response = "1"; // Indicador de autenticación exitosa
                    send(client_fd, response, strlen(response), 0);
                } else {
                    printf("Credenciales inválidas para: %s\n", username);
                    const char *response = "0"; // Indicador de autenticación fallida
                    send(client_fd, response, strlen(response), 0);
                }
            }
            else if(strcmp(servicio, "Crear Grupo") == 0){
                char *username = strtok(NULL, ":");
                char *NombreGrupo = strtok(NULL, ":");
                
                // Abrir el archivo en modo append (añadir al final)
                // Usa "w" si deseas sobrescribir el contenido existente
                FILE *file = fopen(file_path, "a");
                
                // Verificar si el archivo se abrió correctamente
                if (file == NULL) {
                    perror("Error al abrir el archivo");
                    return 1;
                }
                
                // Escribir en el archivo
                fprintf(file, "Nuevo Grupo:%s:%s\n", NombreGrupo, username);
                
                // Cerrar el archivo
                fclose(file);
                
                send(client_fd, "creado", strlen("creado"), 0);
            }
            else if(strcmp(servicio, "Agregar Usuario a Grupo") == 0){
                char *NombreGrupo = strtok(NULL, ":");
                char *Username = strtok(NULL, ":");
                char *Usuario = strtok(NULL, ":");

                // Abrir el archivo en modo append (añadir al final)
                // Usa "w" si deseas sobrescribir el contenido existente
                FILE *file = fopen(file_path, "a");
                
                // Verificar si el archivo se abrió correctamente
                if (file == NULL) {
                    perror("Error al abrir el archivo");
                    return 1;
                }
                
                // Escribir en el archivo
                fprintf(file, "Agregar Usuario a Grupo:%s:%s:%s\n", NombreGrupo, Username, Usuario);
                
                // Cerrar el archivo
                fclose(file);

                send(client_fd, "agregado", strlen("agregado"), 0);
            }
            else if(strcmp(servicio, "Nuevo Mensaje") == 0){
                char *Grupo = strtok(NULL, ":");
                char *Username = strtok(NULL, ":");
                char *Mensaje = strtok(NULL, ":");

                // Abrir el archivo en modo append (añadir al final)
                // Usa "w" si deseas sobrescribir el contenido existente
                FILE *file = fopen(file_path, "a");
                
                // Verificar si el archivo se abrió correctamente
                if (file == NULL) {
                    perror("Error al abrir el archivo");
                    return 1;
                }
                
                // Escribir en el archivo
                fprintf(file, "Nuevo Mensaje:%s:%s:%s\n", Grupo, Username, Mensaje);
                
                // Cerrar el archivo
                fclose(file);

                send(client_fd, "recibido", strlen("recibido"), 0);
            }
            else if(strcmp(servicio, "Actualizar") == 0){
                FILE *file = fopen(file_path, "r");
                if (file == NULL) {
                    perror("Error al abrir el archivo");
                    close(client_fd);
                    exit(EXIT_FAILURE);
                }

                // Obtener la longitud del archivo
                fseek(file, 0, SEEK_END);
                long file_length = ftell(file);
                fseek(file, 0, SEEK_SET);

                // Enviar la longitud del archivo al cliente
                uint32_t length_net = htonl(file_length);
                if (send(client_fd, &length_net, sizeof(length_net), 0) == -1) {
                    perror("Error al enviar la longitud del archivo");
                    fclose(file);
                    close(client_fd);
                    exit(EXIT_FAILURE);
                }

                // Leer el archivo y enviarlo al cliente
                char buffer[BUFFER_SIZE];
                while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
                    if (send(client_fd, buffer, strlen(buffer), 0) == -1) {
                        perror("Error al enviar datos");
                        break;
                    }
                }
                fclose(file);
            }

            close(client_fd);
            break;
        }
    }

    close(server_fd);
    return 0;
}