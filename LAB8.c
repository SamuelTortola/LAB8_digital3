/* 	Nombre     : 	client_tcp.c
	Autor      : 	Luis A. Rivera, basado en el tutorial de
					http://www.linuxhowtos.org/C_C++/socket.htm
	Descripción: 	Cliente Simple (TCP)
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MSG_SIZE 40		// Tamaño (máximo) del mensaje. Puede ser mayor a 40.

// Función para desplegar mensajes de error
void error(const char *msg)
{
	perror(msg);
    exit(0);
}

// Al llamar al programa desde la terminal, se debe indicar la dirección del
// servidos y el puerto a usar:
// 		./client_tcp dir_servidor num_puerto
// El número de puerto debe ser el mismo que el usado en el servidor
int main(int argc, char *argv[])
{
    int sockfd, portno, n;	// fd del socket, para el número de puerto
    struct sockaddr_in serv_addr;	// estructuras para la información del
    struct hostent *server;			// servidor
    char buffer[MSG_SIZE];

    if(argc != 3)	// se requieren dos argumentos al llamar al programa
    {
		fprintf(stderr,"ERROR, Uso correcto: %s IP_servidor num_puerto\n", argv[0]);
		exit(0);
    }

    portno = atoi(argv[2]);		// el puerto fue el segundo parámetro al llamar el programa.
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Crea el socket. "Connection based"
    if(sockfd < 0)
        error("ERROR al crear el socket");

	// convierte la dirección del servidor (ej. 192.168.1.101) al formato requerido
    server = gethostbyname(argv[1]);
    if(server == NULL) {
		fprintf(stderr,"ERROR, no hay tal servidor\n");
		exit(0);
    }

    // Llena los campos de la estructura serv_addr
	memset((char *)&serv_addr, 0, sizeof(serv_addr));	// "Limpiar" la estructura (0 todos los bytes)
    serv_addr.sin_family = AF_INET;		// Constante que indica "Internet domain"

    // Copia info de la estructura server a la estructura serv_addr.sin_addr.s_addr.
	memcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);

    serv_addr.sin_port = htons(portno);		// Convierte el número de puerto al formato requerido

    // Para establecer una conexión con el servidor (que debe estar aceptando conexiones)
	// No se necesita llamar a bind(), porque connect hace el binding
	// automáticamente.
    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR al conectar");

	printf("Ingrese mensaje a enviar: ");
    memset(buffer, 0, MSG_SIZE);		// "limpia" el buffer
	fgets(buffer, MSG_SIZE-1, stdin);	// MSG_SIZE-1 porque se agrega el terminador de cadenas
										// se pudo usar gets()

    // Enviar el mensaje
    n = write(sockfd, buffer, strlen(buffer)); // No es necesario usar sendto(), porque
				// la conexión ya se estableción, y el fd ya tiene la información 
				// necesaria para la comunicación entre el servidor y el cliente.
    if(n < 0)
         error("ERROR al escribir al socket");

	memset(buffer, 0, MSG_SIZE);	// "limpia" el buffer
	
	// Leer el mensaje de vuelta que el servidor enviará
    n = read(sockfd, buffer, MSG_SIZE-1);	// No es necesario usar recvfrom()
    if(n < 0)
         error("ERROR al leer del socket");

    printf("%s\n", buffer);	// Imprime el mensaje del servidor
    close(sockfd);			// cierra el socket
	
    return 0;
}

