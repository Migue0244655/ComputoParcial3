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

// Definición de la estructura del nodo
typedef struct Grupo {
    char *cadena;
    struct Grupo *siguiente;
} Grupo;

// Función para agregar un string al final de la lista
void agregarAlFinal(Grupo **inicio, const char *cadena) {
    Grupo *nuevoGrupo = (Grupo *)malloc(sizeof(Grupo));
    if (nuevoGrupo == NULL) {
        fprintf(stderr, "Error: no se pudo asignar memoria.\n");
        exit(1);
    }
    nuevoGrupo->cadena = strdup(cadena);
    nuevoGrupo->siguiente = NULL;

    if (*inicio == NULL) {
        *inicio = nuevoGrupo;
    } else {
        Grupo *actual = *inicio;
        while (actual->siguiente != NULL) {
            actual = actual->siguiente;
        }
        actual->siguiente = nuevoGrupo;
    }
}

// Función para eliminar un grupo en una posición dada
void eliminarGrupo(Grupo **inicio, int posicion) {
    if (*inicio == NULL) {
        fprintf(stderr, "La lista está vacía.\n");
        return;
    }

    if (posicion == 0) {
        Grupo *temp = *inicio;
        *inicio = (*inicio)->siguiente;
        free(temp->cadena);
        free(temp);
        return;
    }

    Grupo *anterior = *inicio;
    Grupo *actual = (*inicio)->siguiente;
    int indice = 1;
    while (actual != NULL && indice < posicion) {
        anterior = actual;
        actual = actual->siguiente;
        indice++;
    }

    if (actual == NULL) {
        fprintf(stderr, "Posición no válida.\n");
        return;
    }

    anterior->siguiente = actual->siguiente;
    free(actual->cadena);
    free(actual);
}

// Función para imprimir la lista
void imprimirLista(Grupo *inicio) {
    Grupo *actual = inicio;
    while (actual != NULL) {
        printf("%s ", actual->cadena);
        actual = actual->siguiente;
    }
    printf("\n");
}

// Función para liberar la memoria de la lista
void liberarLista(Grupo *inicio) {
    Grupo *actual = inicio;
    while (actual != NULL) {
        Grupo *temp = actual;
        actual = actual->siguiente;
        free(temp->cadena);
        free(temp);
    }
}

void modificarElemento(Grupo *inicio, int n, const char *nuevoValor) {
    Grupo *actual = inicio;
    int indice = 0;

    while (actual != NULL && indice < n) {
        actual = actual->siguiente;
        indice++;
    }

    if (actual != NULL) {
        free(actual->cadena); // Liberar la cadena actual
        actual->cadena = strdup(nuevoValor); // Asignar un nuevo valor al elemento número n
    } else {
        printf("No se puede modificar el elemento número %d. La lista es demasiado corta.\n", n);
    }
}

char *concatenarConStringDeLista(Grupo *inicio, int n, const char *otroString) {
    Grupo *actual = inicio;
    int indice = 0;

    // Buscar el elemento en la posición n de la lista
    while (actual != NULL && indice < n) {
        actual = actual->siguiente;
        indice++;
    }

    // Verificar si se encontró el elemento
    if (actual == NULL) {
        fprintf(stderr, "Error: el elemento en la posición %d no existe.\n", n);
        exit(1);
    }

    // Calcular el tamaño total necesario para el resultado
    size_t tamanoResultado = strlen(actual->cadena) + strlen(otroString) + 2; // +2 para la coma y el terminador nulo

    // Asignar memoria para el resultado
    char *resultado = malloc(tamanoResultado);
    if (resultado == NULL) {
        fprintf(stderr, "Error: no se pudo asignar memoria para el resultado.\n");
        exit(1);
    }

    // Concatenar el string del elemento de la lista con otroString, separados por una coma
    sprintf(resultado, "%s,%s", actual->cadena, otroString);

    return resultado;
}



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
    Grupo *Grupos = NULL;
    Grupo *Coordinadores = NULL;
    Grupo *Participantes = NULL;
    Grupo *Mensajes = NULL;

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
                agregarAlFinal(&Mensajes, "");
                imprimirLista(Mensajes);
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
             agregarAlFinal(&Grupos, NombreGrupo);
             imprimirLista(Grupos);
             agregarAlFinal(&Coordinadores, username);
             imprimirLista(Coordinadores);
             agregarAlFinal(&Participantes, username);
             imprimirLista(Participantes);
        }
        else if(strcmp(servicio, "Agregar Usuario a Grupo") == 0){
            char *NombreGrupo = strtok(NULL, ":");
            char *Username = strtok(NULL, ":");
            char *Usuario = strtok(NULL, ":");
            Grupo *actual = Grupos;
            int n = 0;
            while (actual != NULL) {
                if(strcmp(actual->cadena, NombreGrupo) == 0){
                    break;
                }
                n = n+1;
                actual = actual->siguiente;
            }
            modificarElemento(Participantes, n, concatenarConStringDeLista(Participantes, n, Usuario));
            imprimirLista(Participantes);
        }
        else if(strcmp(servicio, "Nuevo Mensaje") == 0){
            char *Grupo = strtok(NULL, ":");
            char *Username = strtok(NULL, ":");
            char *Mensaje = strtok(NULL, ":");
            char *Notificacion;
            Notificacion = malloc(strlen(Grupo) + strlen(Username) + strlen(Mensaje) + 3); // 3 es para los dos puntos y el terminador nulo
            sprintf(Notificacion, "%s:%s:%s", Grupo, Username, Mensaje);
            //HAcer que se modifiquen todos los usuarios del grupo al que se envio
            //modificarElemento(Mensajes, n, Notificacion);
        }

        close(client_fd);
    }

    close(server_fd);
    return 0;
}
