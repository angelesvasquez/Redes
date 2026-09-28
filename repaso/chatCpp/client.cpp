// Red TCP/IP
// Se conecta a un servidor a traves de una IP y un puerto
// Enviamos un mensaje de texto y luego cierra la conexion 

#include <iostream>
#include <cstring>

// para trabajar con sockets
// socket(), connect(), shutdown()
#include <sys/socket.h> 
#include <netinet/in.h> // Estructura sockaddr_in
#include <arpa/inet.h> // conversion de IPs, inet_pton()

#include <unistd.h> // write(), close()

using namespace std;

int main(int argc, char *argv[]){
    
    if(argc != 3) {
        printf("%s <ip> <puerto>\n", argv[0]);
        return 1;
    }
    
    struct sockaddr_in stSockAddr;

    // (domain: Protocolo de red IPv4, type: conexion TCP, protocol: TCP)
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    int Res;
    int n;

    if(-1 == SocketFD){
        perror("cannot create socket");
        return 1;
    }
    

    //stSockAddr = 000000000000000000...
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[2]));

    //int inet_pton(int af, const char *src, void *dst);
    Res = inet_pton(AF_INET, argv[1], &stSockAddr.sin_addr);

    if(Res == 0){
        perror("char string (second parameter does not contain valid ipaddress");
        close(SocketFD);
        return 1;
    } else if(Res < 0){
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        return 1;
    }

    if(-1 == connect(SocketFD, (const struct sockaddr*)&stSockAddr, sizeof(struct sockaddr_in))){
        perror("connection failed");
        close(SocketFD);
        return 1;
    }

    char buffer[256];
    for(;;){
        cout<<"Write your message: ";
        cin.getline(buffer,sizeof(buffer));

        if(strcmp(buffer, "EXIT") == 0) break;

        n = write(SocketFD, &buffer, sizeof(buffer)-1);

        if(n < 0){
            perror("error send message");
            break;
        }

        n = read(SocketFD, buffer, sizeof(buffer)-1);

        if(n<0){
            perror("error reading form server");
            break;
        }
        
        //buffer[n] = '\0';

        cout<<"Response: ["<<buffer<<"]\n";

        
    }
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);

    return 0;
}
