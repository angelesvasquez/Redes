// Servidor TCP
// Crea un socket TCP, espera conexiones en el puerto 1100,
// recibe un mensaje del cliente, lo imprime y responde luego cierra la conexion
// y vuelve a esperar otro cliente.

#include <iostream>
#include <cstring>
#include <sys/types.h>
// para trabajar con sockets
// socket(), connect(), shutdown()
#include <sys/socket.h> 
#include <netinet/in.h> // Estructura sockaddr_in
#include <arpa/inet.h> // conversion de IPs, inet_pton()
#include <stdio.h> // input output como perror()
#include <stdlib.h> // exit()
#include <string> // memset()
#include <unistd.h> // write(), close()

#include <thread>

using namespace std;

void ThreadReadClient(int IdSocket){
    char buffer[256];
    int n;

    while(1){
        n = read(IdSocket,buffer,sizeof(buffer)-1);
        if(n < 0){
            perror("error read socket");
            break;
        }
        if(n == 0){
            perror("Cliente desconectado");
            break;
        }
    
        buffer[n] = '\0';
    
        cout<<"Mensaje de cliente: "<<buffer<<endl;
    }

    shutdown(IdSocket,SHUT_RDWR);
}

void ThreadWriteClient(int IdSocket){
    char buffer[256];

    while(1){
//        cout<<"Respuesta: ";
        cin.getline(buffer,sizeof(buffer));
        if(strcmp(buffer, "END") == 0) break;

        int n = write(IdSocket,buffer,strlen(buffer));
        if(n < 0){
            perror("Error send message");
            break;
        }
    }
    shutdown(IdSocket,SHUT_RDWR);
}

int main(int argc, char* argv[]){

    if(argc != 2) {
      printf("%s <puerto>\n", argv[0]);
      exit(EXIT_FAILURE);
    }

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
    stSockAddr.sin_port = htons(atoi(argv[1]));
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    // Asigna una direccion IP y un nro de puerto a un socket
    if(-1 == bind(SocketFD, (const struct sockaddr*)&stSockAddr, sizeof(sockaddr_in))){
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
            continue;
        }

        
        thread tRead(ThreadReadClient,ConnectFD);
        thread tWrite(ThreadWriteClient,ConnectFD);

        tRead.join();
        tWrite.join();
        close(ConnectFD);

    }
    close(SocketFD);
    return 0;
}