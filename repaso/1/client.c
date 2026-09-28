// Red TCP/IP
// Se conecta a un servidor a traves de una IP y un puerto
// Enviamos un mensaje de texto y luego cierra la conexion 

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
    int Res;

    // (domain: Protocolo de red IPv4, type: conexion TCP, protocol: TCP)
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;

    if(-1 == SocketFD){
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
    //stSockAddr = 000000000000000000...

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);

    //int inet_pton(int af, const char *src, void *dst);
    Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);

    if(Res == 0){
        perror("char string (second parameter does not contain valid ipaddress");
        close(SocketFD);
        exit(EXIT_FAILURE);
    } else if(Res < 0){
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if(-1 == connect(SocketFD, (const struct sockaddr*)&stSockAddr, sizeof(struct sockaddr_in))){
        perror("connection failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    // ssize_t write(int fd, const void *buf, size_t count);
    n = write(SocketFD, "Holaaa", 6);
    if(n < 0){
        perror("error send message");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    n = read(SocketFD, buffer, sizeof(buffer)-1);
    if(n<0){
        perror("error reading form server");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    
    // for (int i = 0; i < n+2; i++) {
    //     printf("buffer[%d] = %d\n", i, buffer[i]);
    // }
    buffer[n] = '\0';
    printf("Response: [%s]\n",buffer);

    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);

}
