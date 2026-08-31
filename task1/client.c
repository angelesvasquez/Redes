 /* Tarea 1
    Chat de mensajes con cliente-servidor
    Cliente se conecta al sevidor
    Escribe un mensaje
    El servidor recibe el mensaje, puede escribir una respuesta
    Cliente recibe la respuesta y asi se repite.

 */
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
 
  int main(void)
  {
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // Crea la conexion
    int n;
    char buffer[256];
    //char message[256];

    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(45000); //45000
    Res = inet_pton(AF_INET, "127.0.0.1", &stSockAddr.sin_addr);
 
    if (0 > Res)
    {
      perror("error: first parameter is not a valid address family");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
      perror("char string (second parameter does not contain valid ipaddress");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) // El cliente se conecta
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    while(1){

        printf("Escribe tu mensaje: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // revisar
        n = write(SocketFD,buffer,strlen(buffer));
        if (n < 0) perror("ERROR writing from socket");
        // hasta aqui 
        /* perform read write operations ... */
        if(strcmp(buffer, "END") == 0) break;
        n = read(SocketFD, buffer, 255);
        if (n < 0) perror("ERROR reading from socket");
        else if(n==0){
            printf("The server closed the connection");
            break;
        }
        else {
            buffer[n]='\0';
            printf("Servidor: %s", buffer);
        }
            //printf("Here is the message: [%s]\n",buffer);


    }
    
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    return 0;
  }