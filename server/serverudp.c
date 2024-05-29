/*

	Lectura remota de una palabra para devolver el numero de vocales usando sockets pertenecientes
	a la familia TCP, en modo conexion.
	Codigo del servidor

	Nombre Archivo: tcpserver.c
	Archivos relacionados: num_vocales.h tcpclient.c 
	Fecha: Febrero 2023

	Compilacion: cc serverudp.c -lnsl -o serverudp
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

struct User {
    char username[32];
    char password[32];
};

struct User users[] = {
    {"user1", "1234"},
    {"user2", "1234"},
    {"user3", "1234"}
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

// Función para verificar si un usuario ya está autenticado
bool is_user_authenticated(const char *username) {
    FILE *file = fopen("usuarios.txt", "r");
    if (file == NULL) {
        return false;
    }
    char line[32];
    while (fgets(line, sizeof(line), file)) {
        // Eliminar el salto de línea al final
        line[strcspn(line, "\n")] = '\0';
        if (strcmp(line, username) == 0) {
            fclose(file);
            return true;
        }
    }
    fclose(file);
    return false;
}

// Función para agregar un usuario autenticado al archivo
void add_authenticated_user(const char *username) {
    FILE *file = fopen("usuarios.txt", "a");
    if (file != NULL) {
        fprintf(file, "%s\n", username);
        fclose(file);
    } else {
        perror("Error al abrir usuarios.txt");
    }
}

int main() {
    char buffer1[BUFFER_SIZE];
    struct sockaddr_in servaddr, cliaddr;
    int sockfd, len, tiempo;
    const char *file_path = "BD.txt";

    // Crear socket UDP
    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == -1) {
        perror("Fallo en la creación del socket");
        exit(EXIT_FAILURE);
    }

    // Configurar dirección del servidor
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(5070);

    // Enlazar direccion del servidor al socket
    if (bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Fallo en el enlace del socket");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Recibir las credenciales del cliente
        len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer1, BUFFER_SIZE, 0,
                        (struct sockaddr *)&cliaddr, (socklen_t *)&len);
        if (n < 0) {
            perror("recvfrom failed");
            exit(EXIT_FAILURE);
        } else {
            buffer1[n] = '\0';
        }
        if (n <= 0) {
            perror("Error al recibir datos");
            continue;
        }
        buffer1[n] = '\0';

        xor_crypt(buffer1, "parangaricutirimicuaro", n);

        printf("Recibi: %s\n", buffer1);

        // Separar el nombre de usuario y la contraseña
        char *servicio = strtok(buffer1, ":");
        if (strcmp(servicio, "Autenticar Usuario") == 0) {
            char *username = strtok(NULL, ":");
            char *password = strtok(NULL, ":");

            if (username == NULL || password == NULL) {
                printf("Formato de credenciales inválido\n");

                char response[32] = {'F','o','r','m','a','t','o',' ','d','e',' ','c','r','e','d','e','n','c','i','a','l','e','s',' ','i','n','v','a','l','i','d','o'};
                char *ptrresponse = &response;
                xor_crypt(ptrresponse, "parangaricutirimicuaro", 32);
                sendto(sockfd, ptrresponse, 32, 0, (struct sockaddr *)&cliaddr, len);
                continue;
            }

            // Verificar si el usuario ya está autenticado
            if (is_user_authenticated(username)) {
                printf("Usuario ya autenticado: %s\n", username);
                // Cambiar el tamaño del array para incluir el carácter nulo
                char response[23] = {'U', 's', 'u', 'a', 'r', 'i', 'o', ' ', 'y', 'a', ' ', 'a', 'u', 't', 'e', 'n', 't', 'i', 'c', 'a', 'd', 'o', '\0'};
                char *ptrresponse = response;
                printf("B\n");
                xor_crypt(ptrresponse, "parangaricutirimicuaro", 22); // Aquí parece correcto, pero podrías querer revisar la implementación de xor_crypt
                sendto(sockfd, ptrresponse, 22, 0, (struct sockaddr *)&cliaddr, len); // Enviar el tamaño correcto, que ahora es 23}
                continue;
            }

            // Autenticar al usuario
            if (authenticate(username, password)) {
                printf("Usuario autenticado: %s\n", username);

                // Agregar a la lista de usuarios autenticados
                add_authenticated_user(username);

                char response[1] = {'1'}; // Indicador de autenticación exitosa

                // Abrir el archivo en modo append (añadir al final)
                // Usa "w" si deseas sobrescribir el contenido existente
                FILE *file = fopen(file_path, "a");
                
                // Verificar si el archivo se abrió correctamente
                if (file == NULL) {
                    perror("Error al abrir el archivo");
                    return 1;
                }
                
                // Escribir en el archivo
                fprintf(file, "Nuevo Usuario:%s\n", username);
                
                // Cerrar el archivo
                fclose(file);
                
                char *ptrresponse = &response;
                printf("antes: %s\n", ptrresponse);
                xor_crypt(ptrresponse, "parangaricutirimicuaro", 1);
                printf("envie: %s\n", ptrresponse);
                sendto(sockfd, ptrresponse, 1, 0, (struct sockaddr *)&cliaddr, len);
            } else {
                printf("Credenciales inválidas para: %s\n", username);
                sendto(sockfd, "0", strlen("0"), 0, (struct sockaddr *)&cliaddr, len);
            }
        }
    }
    close(sockfd);
    return 0;
}