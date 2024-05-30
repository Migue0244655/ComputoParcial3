/*

	Lectura remota de una palabra para devolver el numero de vocales usando sockets pertenecientes
	a la familia TCP, en modo conexion.
	Codigo del servidor

	Nombre Archivo: tcpserver.c
	Archivos relacionados: num_vocales.h tcpclient.c 
	Fecha: Febrero 2023

	Compilacion: cc server2.c -lnsl -o server2
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

// Función para cifrar/descifrar usando XOR
void xor_crypt(char *data, const char *key, int data_len) {
    int key_len = strlen(key);
     // Agregar un carácter nulo al final de data si no está presente
    if (data[data_len] != '\0') {
        data[data_len] = '\0';
        data_len++;
    }
    char *tempdata;
    strcpy(tempdata,data);
    for (char i = 0; i < data_len; i++) {
        char tempval=data[i]^key[i % key_len];
        *(data+i) = tempval;
    }
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
    server_addr.sin_port = htons(5061);

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

            xor_crypt(buffer, "parangaricutirimicuaro", bytes_received);

            printf("Recibi: %s\n", buffer);

            // Separar el nombre de usuario y la contraseña
            char *servicio = strtok(buffer, ":");
            if(strcmp(servicio, "Crear Grupo") == 0){
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

                char response[6] = {'c','r','e','a','d','o'};
                char *ptrresponse = &response;
                xor_crypt(ptrresponse, "parangaricutirimicuaro", 6);
                send(client_fd, ptrresponse, 6, 0);
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

                char response[8] = {'a','g','r','e','g','a','d','o'};
                char *ptrresponse = &response;
                xor_crypt(ptrresponse, "parangaricutirimicuaro", 8);
                send(client_fd, ptrresponse, 8, 0);
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

                char response[8] = {'r','e','c','i','b','i','d','o'};
                char *ptrresponse = &response;
                xor_crypt(ptrresponse, "parangaricutirimicuaro", 8);
                send(client_fd, ptrresponse, 8, 0);
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
                    size_t buffer_len = strlen(buffer);
                    if (send(client_fd, buffer, buffer_len, 0) == -1) {
                        perror("Error al enviar datos");
                        break;
                    }
                }
                fclose(file);
            }
            else if(strcmp(servicio, "Eliminar Grupo") == 0){
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
                fprintf(file, "Eliminar Grupo:%s\n", NombreGrupo);
                
                // Cerrar el archivo
                fclose(file);

                char response[9] = {'e','l','i','m','i','n', 'a', 'd', 'o'};
                char *ptrresponse = &response;
                xor_crypt(ptrresponse, "parangaricutirimicuaro", 9);
                send(client_fd, ptrresponse, 9, 0);
            }
            else if(strcmp(servicio, "Comprobar") == 0){
                printf("entre comprobado\n");
                char response[1] = {'9'};
                char *ptrresponse = &response;
                xor_crypt(ptrresponse, "parangaricutirimicuaro", 1);
                send(client_fd, ptrresponse, 1, 0);
                printf("envie 9\n");
            }
            else{
                char response[11] = {'n','o',' ','s','e','r','v','i','c','i','o'};
                char *ptrresponse = &response;
                xor_crypt(ptrresponse, "parangaricutirimicuaro", 11);
                send(client_fd, ptrresponse, 11, 0);
            }

            close(client_fd);
            break;
        }
    }

    close(server_fd);
    return 0;
}