#include <iostream>
#include <thread>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <cstdlib>
#include <cstring>
#include <unistd.h>

void handle_client(int ConnectFD)
{
    char buffer[256];
    int n;

    while(1)
    {
        n = read(ConnectFD, buffer, 255);

        if (n < 0)
        {
            perror("ERROR reading from socket");
            break;
        }
        else if(n == 0)
        {
            std::cout << "The client closed the connection\n";
            break;
        }
        else
        {
            buffer[n] = '\0';
            std::cout << "Client: " << buffer << "\n";
        }

        if(strcmp(buffer, "END") == 0)
            break;

        std::cout << "Response: ";

        std::cin.getline(buffer, 256);

        n = write(ConnectFD, buffer, strlen(buffer));

        if (n < 0)
        {
            perror("ERROR writing to socket");
            break;
        }

        if(strcmp(buffer, "END") == 0)
            break;
    }

    shutdown(ConnectFD, SHUT_RDWR);
    close(ConnectFD);
}

int main(int argc, char* argv[])
{
    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if(argc != 2)
    {
        std::cout << argv[0] << " <puerto>\n";
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

    if(-1 == bind(SocketFD,
                  (const struct sockaddr *)&stSockAddr,
                  sizeof(struct sockaddr_in)))
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

    std::cout << "Server listening on port "
              << argv[1] << "...\n";

    for(;;)
    {
        int ConnectFD = accept(SocketFD, NULL, NULL);

        if(ConnectFD < 0)
        {
            perror("error accept failed");
            continue;
        }

        std::cout << "Client connected.\n";

        std::thread client_thread(handle_client, ConnectFD);
        client_thread.detach();
    }

    close(SocketFD);

    return 0;
}