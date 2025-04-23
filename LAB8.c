 /////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
//Universidad del valle de Guatemala
//Alan Gomez - 22115
//Samuel Tortola - 22094
//Curso Electrónica Digital 3 - IE3059
//Laboratorio 8 
//Hardware: Raspberry Pi 5 8GB RAM, Raspberry Pi 3B 8GB RAM, Computadoras con sistema OS linux
//Archivo: LAB_8.c
//FECHA 22/04/25
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////
//LIBRERIAS
/////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>



/////////////////////////////////////////////////////////////
//DEFINICIONES Y VARIABLES GLOBALES
/////////////////////////////////////////////////////////////
#define MSG_SIZE 60		// Tamaño (máximo) del mensaje.
int modo = 1;  //1 para modo esclavo, 0 para modo maestro



/////////////////////////////////////////////////////////////
//FUNCIONES
/////////////////////////////////////////////////////////////
//--Nombres de funciones--//
int number_random(void);
void modo_maestro(int puerto);
void modo_esclavo(int puerto);
void escuchar_mensajes(int sockfd);


//--Funciones--//
int number_random(void) {
    srand(time(NULL)); // Inicializar la semilla con el tiempo actual
    int max = 10, min = 1; // Rango de números aleatorios
    int random_number = (rand() % (max - min + 1)) + min;
    return random_number;
}


// Función general de escucha (usada tanto por maestro como esclavo)
void escuchar_mensajes(int sockfd) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    char buffer[MSG_SIZE];

    while (1) {
        memset(buffer, 0, MSG_SIZE);
        int n = recvfrom(sockfd, buffer, MSG_SIZE, 0, (struct sockaddr *)&addr, &addrlen);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        printf("Recibido desde %s:%d - Mensaje: %s\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), buffer);
        
        if (modo == 0 && strcmp(buffer, "QUIEN ES") == 0) {
            // responder al cliente
        } else if (modo == 1 && strcmp(buffer, "QUIEN ES") == 0) {
            // ignorar
        } else if (strcmp(buffer, "VOTE") == 0) {
            // iniciar proceso de votación
        }
    }
}

// Función para modo esclavo
void modo_esclavo(int puerto) {
    int sockfd;
    struct sockaddr_in server;
    int boolval = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) perror("Error abriendo socket");

    server.sin_family = AF_INET;
    server.sin_port = htons(puerto);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
        perror("Error haciendo bind");

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
        perror("Error configurando broadcast");

    printf("Modo ESCLAVO activo en puerto %d\n", puerto);
    escuchar_mensajes(sockfd);
}

// Función para modo maestro
void modo_maestro(int puerto) {
    int sockfd;
    struct sockaddr_in server;
    int boolval = 1;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) perror("Error abriendo socket");

    server.sin_family = AF_INET;
    server.sin_port = htons(puerto);
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
        perror("Error haciendo bind");

    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &boolval, sizeof(boolval)) < 0)
        perror("Error configurando broadcast");

    printf("Modo MAESTRO activo en puerto %d\n", puerto);
    escuchar_mensajes(sockfd);
}


void obtener_ip_local() {
    struct ifaddrs *ifaddr, *ifa;
    char ip[INET_ADDRSTRLEN];

    // Obtener todas las interfaces
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Recorremos todas las interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        // Si la dirección es IPv4 (AF_INET) y no es loopback
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, "lo") != 0) {
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            printf("Interfaz %s -> IP: %s\n", ifa->ifa_name, ip);
        }
    }

    freeifaddrs(ifaddr);
}


/////////////////////////////////////////////////////////////
//PROGRAMA PRINCIPAL
/////////////////////////////////////////////////////////////

// Función principal
int main(int argc, char *argv[]) {

  // printf("Numero aleatorio generado: %d\n", number_random());


    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int puerto = atoi(argv[1]);

    if (puerto <= 1025 || puerto > 65535) {
        // Evitar puertos reservados (0–1023) y dejar margen de seguridad
        fprintf(stderr, "Puerto inválido. Debe ser un número entre 1 y 65535.\n");
        exit(EXIT_FAILURE);
    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");
    printf("***************** Información del socket: *********************************\n");
    printf("\n");
    obtener_ip_local();

    while (1){
        if (modo == 0){
            modo_maestro(puerto);
        }
        else if (modo == 1){
            modo_esclavo(puerto);
        }
        else{
            fprintf(stderr, "Modo inválido\n");
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}