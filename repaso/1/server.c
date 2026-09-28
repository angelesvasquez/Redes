// Servidor TCP
// Crea un socket TCP, espera conexiones en el puerto 1100,
// recibe un mensaje del cliente, lo imprime y responde luego cierra la conexion
// y vuelve a esperar otro cliente.


#include <sys/types.h>
// para trabajar con sockets
// socket(), connect(), shutdown()
#include <sys/socket.h> 
#include <netinet/in.h> // Estructura sockaddr_in
#include <arpa/inet.h> // conversion de IPs, inet_pton()
#include <stdio.h> // input output como perror()
#include <stdlib.h> // exit()
#include <string.h> // memset()
#include <unistd.h> // write(), close()

int main(void){
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n; // nro de bytes leidos (read()) o enviados
    char buffer[256]; // almacena el mensaje recibido del cliente

    if(SocketFD == -1){
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr,0,sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    // Asigna una direccion IP y un nro de puerto a un socket
    if(-1 == bind(SocketFD, (const struct sockaddr*)&stSockAddr, sizeof(struct  sockaddr_in))){
        perror("error bind failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if(-1 == listen(SocketFD,10)) { // nro max en cola (clientes en fila)
        perror("error listen failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    for(;;){

        int ConnectFD = accept(SocketFD, NULL, NULL);
        // accept espera hasta que llegue un cliente
        // ponemos NULL si NO nos interesa saber quien se conecto
        // Se crea un socket unicamente para el cliente

        if(0 > ConnectFD){
            perror("error accept failed");
            close(SocketFD);
            exit(EXIT_FAILURE);
        }

        bzero(buffer, 256);
        n = read(ConnectFD, buffer, sizeof(buffer)-1);
        if(n<0) perror("ERROR reading form socket");
        printf("Here is the message: [%s]\n",buffer);
        const char* answer = "Holiii";
        n = write(ConnectFD,"I got your message",18);
        if(n<0) perror("ERROR writing to socket");

        shutdown(ConnectFD, SHUT_RDWR);
        close(ConnectFD);
    }
    close(SocketFD);
    return 0;
}