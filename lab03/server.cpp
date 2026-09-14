//*Server

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <thread>
#include <string>
#include <map>

std::map<std::string, int> ListOfCli;

std::string zeroPad(int number, int size)
{
    std::string str = std::to_string(number);

    if (str.length() >= size)
        return str;

    return std::string(size - str.length(), '0') + str;
}

void thread_readClient(int connectFD){
    std::string nickname;
    char buffer[256];
    int n;

    while(1){

         n = read(connectFD,buffer,255);
         if (n < 0) {perror("ERROR reading from socket"); break;}
         else if(n==0){
            printf("The client closed the connection");
            break;
         }
         else{
            buffer[n]='\0';
            printf("Client: %s\n",buffer);
        }
        if(strcmp(buffer, "END") == 0) break;
        printf("Response: ");
        fgets(buffer, sizeof(buffer),stdin);
        buffer[strcspn(buffer, "\n")] = '\0';
        n = write(connectFD,buffer, strlen(buffer));
        if (n < 0) {perror("ERROR writing to socket"); break;}
        if(strcmp(buffer, "END") == 0) break;
           
    }
    shutdown(connectFD, SHUT_RDWR);
 
    close(connectFD);
} 

int main(int argc, char* argv[])
{
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
 
    if(argc != 2) {
      printf("%s <puerto>\n", argv[0]);
      exit(EXIT_FAILURE);
    }
    if(-1 == SocketFD)
    {
      perror("can not create socket");
      exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[1]));
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
    if(-1 == bind(SocketFD,(const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) // El servidor usa el puerto
    {
      perror("error bind failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    if(-1 == listen(SocketFD, 10))
    {
      perror("error listen failed");
      close(SocketFD);
      exit(EXIT_FAILURE);
    }
 
    for(;;)
    {
        int ConnectFD = accept(SocketFD, NULL, NULL);

        if(0 > ConnectFD)
        {
            perror("error accept failed");
            continue;
        }
        
        std::thread threadClient(thread_readClient, ConnectFD);
        
        threadClient.detach();
        //bzero(buffer,256);

        //  while(1){
        //      n = read(ConnectFD,buffer,255); // Recibe
        //      //buffer[n]='\0';  // revisar
        //      if (n < 0) perror("ERROR reading from socket");
        //      else if(n==0){
        //         printf("The client closed the connection");
        //         break;
        //      }
        //      else{
        //         buffer[n]='\0'; // revisar
        //         printf("Client: %s\n",buffer);
        //     }
        //     if(strcmp(buffer, "END") == 0) break;
        //     printf("Response: ");
        //     fgets(buffer, sizeof(buffer),stdin);
        //     buffer[strcspn(buffer, "\n")] = '\0';
        //     n = write(ConnectFD,buffer, strlen(buffer)); // Envia
        //     if (n < 0) perror("ERROR writing to socket");
        //     if(strcmp(buffer, "END") == 0) break;
            
        //      /* perform read write operations ... */
            
        //  }
        //   shutdown(ConnectFD, SHUT_RDWR);

        // close(ConnectFD);
    }
 
    close(SocketFD);
    return 0;
}