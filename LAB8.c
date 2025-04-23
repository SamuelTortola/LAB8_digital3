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
#include <net/if.h>




/////////////////////////////////////////////////////////////
//DEFINICIONES Y VARIABLES GLOBALES
/////////////////////////////////////////////////////////////
#define MSG_SIZE 60		// Tamaño (máximo) del mensaje.
int modo = 1, modo_voto = 0;  //1 para modo esclavo, 0 para modo maestro

#define IP_SIZE INET_ADDRSTRLEN // Tamaño de la IP (IPv4)

char ip_global[IP_SIZE]; //  Variable global para almacenar la IP local
char broadcast_ip[INET_ADDRSTRLEN]; // Variable global para guardar IP broadcast
char ip_recibida[INET_ADDRSTRLEN];      // La IP que viene del mensaje
char ip_recibida_cliente[INET_ADDRSTRLEN];  // La IP que viene del cliente que envia "VOTE" y "QUIEN ES"

int sockfd, n;  // Descriptores de socket y variable de envio de mensajes

struct sockaddr_in broadcast_addr; // Estructura para la dirección de broadcast
socklen_t broadcast_length = sizeof(struct sockaddr_in); // Tamaño de la estructura de dirección de broadcast

int puerto = 2000; // Puerto por defecto
int numero_ramdon;



/////////////////////////////////////////////////////////////
//FUNCIONES
/////////////////////////////////////////////////////////////
//--Nombres de funciones--//
int number_random(void);
void modo_maestro(int puerto);
void modo_esclavo(int puerto);
void escuchar_mensajes(int sockfd);
void inicializar_broadcast(int puerto);
void obtener_ip_local(void);
void error(const char *msg);
void espera_aleatoria(void);



//--Funciones--//
int number_random(void) {
    srand(time(NULL)); // Inicializar la semilla con el tiempo actual
    int max = 10, min = 1; // Rango de números aleatorios
    int random_number = (rand() % (max - min + 1)) + min;
    return random_number;
}

void error(const char *msg) //Función para manejar errores
{
    perror(msg);
    exit(0);
}

void espera_aleatoria() {

    // Generar número aleatorio entre 100000 (0.1s) y 2000000 (2s)
    int microsegundos = (rand() % (2000000 - 100000 + 1)) + 100000;

    // Esperar ese tiempo
    usleep(microsegundos);
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

        if (strcmp(inet_ntoa(addr.sin_addr), ip_global) == 0) {
            // Si la IP de origen es la misma que la IP local, ignorar el mensaje
        } 
        
        else {
            // Imprimir el mensaje recibido
            // inet_ntoa convierte la dirección IP en una cadena de caracteres
            // ntohs convierte el número de puerto de red a host byte order
            // buffer es el mensaje recibido
            // strcspn elimina el salto de línea del buffer si existe
            // %s es para cadenas de caracteres, %d es para enteros
            
            if (modo_voto == 0){   //Mostrar solo mensajes normales, si no esta en modo normal, y no votación
            printf("Recibido desde %s:%d - Mensaje: %s\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port), buffer);
            }

            //Recibir el mensaje, pero no hacer nada con él, a menos que sea un mensaje de votación o descubrimiento
            strcpy(ip_recibida, inet_ntoa(addr.sin_addr)); // Guardar la IP recibida en la variable global
           //La función strcpy se usa para copiar una cadena de caracteres (en este caso la IP) a otra variable (ip_recibida)
               
                //La función inet_ntoa convierte una dirección IP en formato binario a una cadena de caracteres (en este caso la IP recibida)
                //La función ntohs convierte el número de puerto de red a host byte order (en este caso el puerto recibido)
     
               
        //La función strcspn se usa para encontrar la longitud de la cadena hasta el primer carácter que coincide con el segundo argumento (en este caso "\n")

        
        buffer[strcspn(buffer, "\n")] = 0;  // elimina salto de línea si viene en el buffer

        if (modo_voto == 1){
            char ip[INET_ADDRSTRLEN];
            int numeros;
            
            if (sscanf(buffer, "# %s %d", ip, &numeros) == 2) {
                //La función sscanf se usa para leer datos de una cadena de caracteres (buffer) y almacenarlos en variables
                //El formato "# %s %d" indica que se espera un string (ip) seguido de un entero (numeros)
                printf("IP recibida: %s\n", ip);
                printf("Número recibido: %d\n", numeros);
            
                if (numeros > numero_ramdon) {
                    printf("CONTINUA ESCLAVO, El número recibido es mayor que el número aleatorio: %d\n", numero_ramdon);
                    modo = 1; // Cambiar a modo esclavo
                    modo_voto = 0; // Cambiar a modo de no votación, porque el sistema ya perdió
                    printf("\n");
                    printf("\n");
                    printf("\n");
                } 

                else if (numeros == numero_ramdon) {
                    printf("Continua verificación por IP, El número recibido es igual al número aleatorio: %d\n", numero_ramdon);
                    printf("\n");
                    printf("\n");
                    printf("\n");

                    uint32_t mi_ip = ntohl(inet_addr(ip_global));
                    uint32_t otra_ip = ntohl(inet_addr(ip_recibida));
                      //Aqui se convierte la IP a formato de host (orden de bytes del host) para compararlas
                    //La función inet_addr convierte una cadena de caracteres (en este caso la IP) a una dirección IP en formato binario
                    //La función ntohl convierte el número de puerto de red a host byte order (en este caso el puerto recibido)
                    //host byte order es el orden de bytes que usa la computadora para almacenar los números
                    //network byte order es el orden de bytes que usa la red para transmitir los números

                    //Practicamente una ip como 192.168.1.10  se convierte a un número de 32 bits y si se convierte a entero, se puede comparar

                    if (mi_ip < otra_ip) {
                        printf(" CONTINUA MAESTRO.  IP del sistema (%s) es menor que la IP recibida (%s)\n", ip_global, ip_recibida);
                        printf("\n");
                        printf("\n");
                        printf("\n");
                        modo = 0; // Cambiar a modo maestro
                        printf("\n");

                    } 
                    
                    else if (mi_ip > otra_ip) {
                        printf("CONTINUA ESCLAVO, IP del sistema (%s) es mayor que la IP recibida (%s)\n", ip_global, ip_recibida);
                        modo = 1; // Cambiar a modo esclavo
                        modo_voto = 0; // Cambiar a modo de no votación, porque el sistema ya perdió
                        printf("\n");
                        printf("\n");
                        printf("\n");
                    } 
                    
                    else {
                        printf("Las IPs son iguales\n");
                        printf("\n");
                        printf("\n");
                        printf("\n");
                    }

                } 
                    
                else if (numeros < numero_ramdon) {
                    printf("CONTINUA MAESTRO, El número recibido es menor que el número aleatorio: %d\n", numero_ramdon);
                    modo = 0; // Cambiar a modo maestro
                    printf("\n");
                    printf("\n");
                    printf("\n");
                }
                
            } 
            
            else {

            }


        }
 


        if (modo == 0 && strcmp(buffer, "QUIEN ES") == 0) { //Si esta en modo maestro
            //Esta forma de comparar el buffer que se usa strcmp es para comparar cadenas de caracteres, 
            //en este caso el buffer y la cadena "QUIEN ES", se iguala a a 0 porque si son iguales
            //la función strcmp devuelve 0, si no son iguales devuelve un valor diferente de 0.
            modo_voto = 0; // Cambiar a modo de no votación, porque el sistema quedo como maestro de todos


            
            broadcast_addr.sin_addr.s_addr = inet_addr(ip_recibida_cliente);
            char mensaje[MSG_SIZE];  // Buffer para el mensaje
            snprintf(mensaje, MSG_SIZE, "Alan y Samuel en %s - master\n", ip_global);
            n = sendto(sockfd, mensaje, strlen(mensaje), 0,
                       (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
            if (n < 0)
                error("sendto");
            
        }

         else if (modo == 1 && strcmp(buffer, "QUIEN ES") == 0) {
            modo_voto = 0; // Cambiar a modo de no votación
            // ignorar porque es esclavo


        } 


        else if (strcmp(buffer, "VOTE") == 0) {  // iniciar proceso de votación
            strcpy(ip_recibida_cliente, inet_ntoa(addr.sin_addr)); // Guardar la IP recibida del ciente en la variable global
            printf("\n");
            printf("\n");
            printf("________________________Iniciando proceso de votación______________________________\n");
            modo_voto = 1; // Cambiar a modo de votación
            numero_ramdon = number_random();
            printf("\n");
            printf("Enviando a todos: # %s %d\n", ip_global, numero_ramdon);
            printf("\n");
            printf("\n");
            printf("\n");
            espera_aleatoria(); // Esperar un tiempo aleatorio antes de enviar el mensaje



            broadcast_addr.sin_addr.s_addr = inet_addr(broadcast_ip);

            char mensaje[MSG_SIZE];  // Buffer para el mensaje

            //snprintf sirve para rellenar la cadena de caracteres, en este caso el mensaje

            // esta función se usa así: int snprintf(char *str, size_t size, const char *format, ...);
            //str: el buffer donde se va a escribir (como una variable char mensaje[100];)
            //size: el tamaño máximo que puede escribir (para evitar sobrepasarse)
            //format: como en printf, usás %s, %d, etc.
            //y luego los valores a insertar
           
         
            snprintf(mensaje, MSG_SIZE, "# %s %d\n", ip_global, numero_ramdon);
            
            // Enviar el mensaje
            n = sendto(sockfd, mensaje, strlen(mensaje), 0,
                       (struct sockaddr *)&broadcast_addr, sizeof(broadcast_addr));
            if (n < 0)
                error("sendto");


        }
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
    printf("\n");
    printf("\n");
    printf("\n");
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
    printf("\n");
    printf("\n");
    printf("\n");
    escuchar_mensajes(sockfd);
}

//NOTA: ESTAS  DOS ULTIMAS FUNCIONES SOLO SE ACTIVAN CUANDO SE EJECUTA EL CÓDIGO, NO DURANTE LA EJECUCIÓN. 
//ESTAS FUNCIONES SIRVEN PARA MOSTRAR EN QUE SE INICIA EL PROGRAMA Y EN QUE PUERTO SE INICIA. NO EN QUE  MODO TERMINA

void obtener_ip_local() {
    struct ifaddrs *ifaddr, *ifa;
  

    // Obtener todas las interfaces
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        exit(EXIT_FAILURE);
    }

    // Recorremos todas las interfaces
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
         // Si la dirección es IPv4 (AF_INET), no loopback y tiene broadcast
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_INET && strcmp(ifa->ifa_name, "lo") != 0 && ifa->ifa_flags & IFF_BROADCAST && ifa->ifa_broadaddr != NULL) {

            struct sockaddr_in *addr = (struct sockaddr_in *)ifa->ifa_addr;
            struct sockaddr_in *bcast = (struct sockaddr_in *)ifa->ifa_broadaddr;

            inet_ntop(AF_INET, &addr->sin_addr, ip_global, sizeof(ip_global));
            inet_ntop(AF_INET, &bcast->sin_addr, broadcast_ip, sizeof(broadcast_ip));
            printf("Interfaz %s -> IP: %s, Broadcast: %s\n", ifa->ifa_name, ip_global, broadcast_ip);
        }
    }

    freeifaddrs(ifaddr);
}


void inicializar_broadcast(int puerto) {
    //Función para inicializar la dirección de broadcast
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port = htons(puerto);  
}




/////////////////////////////////////////////////////////////
//PROGRAMA PRINCIPAL
/////////////////////////////////////////////////////////////

// Función principal
int main(int argc, char *argv[]) {

  // printf("Numero aleatorio generado: %d\n", number_random());


    if (argc != 2) {
        fprintf(stderr, "Uso: %s <puerto>\n", argv[0]);
        printf("Colocando puerto por defecto: 2000\n");
        printf("Para cambiar el puerto, ejecuta el programa con el puerto deseado como argumento.\n");
        printf("Ejemplo: %s 3000\n", argv[0]);


    }
    if (argc == 2) {

     puerto = atoi(argv[1]);
    }

    if (puerto <= 1025 || puerto > 65535) {
        // Evitar puertos reservados (0–1023) y dejar margen de seguridad
        fprintf(stderr, "Puerto inválido. Debe ser un número entre 1025 y 65535.\n");
        exit(EXIT_FAILURE);
    }
    printf("\n");
    printf("\n");
    printf("\n");
    printf("\n");
    printf("***************** Información del socket: *********************************\n");
    printf("\n");
    obtener_ip_local();
    inicializar_broadcast(puerto);
    srand(time(NULL)); // Inicializar la semilla para tiempos aleatorios


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