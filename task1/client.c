 // Client
 
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <string.h>
  #include <unistd.h>
 
  int main(int argc, char* argv[])
  {
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;
    char buffer[256];

    if(argc != 3) {
      printf("%s <ip> <puerto>\n", argv[0]);
      exit(EXIT_FAILURE);
    }

    if (-1 == SocketFD)
    {
      perror("cannot create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[2])); //45000
    Res = inet_pton(AF_INET, argv[1], &stSockAddr.sin_addr);
 
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
 
    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in)))
    {
      perror("connect failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
    while(1){

        printf("Write your message: ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        n = write(SocketFD,buffer,strlen(buffer));
        if (n < 0) perror("ERROR writing from socket");
        if(strcmp(buffer, "END") == 0) break;

        n = read(SocketFD, buffer, 255);
        if (n < 0) perror("ERROR reading from socket");
        else if(n==0){
            printf("The server closed the connection");
            break;
        }
        else {
            buffer[n]='\0';
            if(strcmp(buffer, "END") == 0) break;
            printf("Server: %s\n", buffer);
        }

    }
    
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    return 0;
  }