#include <iostream>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstdlib>
#include <cstring>
#include <unistd.h>

int main(int argc, char* argv[])
{
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;
    char buffer[256];

    if(argc != 3) {
        std::cout << argv[0] << " <ip> <puerto>\n";
        exit(EXIT_FAILURE);
    }

    if (-1 == SocketFD)
    {
        perror("cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[2]));

    Res = inet_pton(AF_INET, argv[1], &stSockAddr.sin_addr);

    if (0 > Res)
    {
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }
    else if (0 == Res)
    {
        perror("char string does not contain valid ip address");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(SocketFD,
                      (const struct sockaddr *)&stSockAddr,
                      sizeof(struct sockaddr_in)))
    {
        perror("connect failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    std::cout << "Connected to "
              << argv[1] << ":"
              << argv[2] << "\n";

    while(1)
    {
        std::cout << "Write your message: ";

        std::cin.getline(buffer, 256);

        n = write(SocketFD, buffer, strlen(buffer));

        if (n < 0)
        {
            perror("ERROR writing to socket");
            break;
        }

        if(strcmp(buffer, "END") == 0)
            break;

        n = read(SocketFD, buffer, 255);

        if (n < 0)
        {
            perror("ERROR reading from socket");
        }
        else if(n == 0)
        {
            std::cout << "The server closed the connection\n";
            break;
        }
        else
        {
            buffer[n] = '\0';

            if(strcmp(buffer, "END") == 0)
                break;

            std::cout << "Server: " << buffer << "\n";
        }
    }

    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);

    return 0;
}