/* Server code in C++ */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <map>
#include <string>
#include <thread> 

using namespace std; 

map<string, int> ListOfCli;

string zeroPad(int number, int size) {
    string str = to_string(number);
    if (str.length() >= size)
        return str;
    return string(size - str.length(), '0') + str;
}

void threadReadClient(int clientSocket) {
    string nickname;
    char buff[1000];
    int n, msgSize;
    
    // 1. Look and wait for nickname
    read(clientSocket, buff, 1);
    while (buff[0] != 'N') {
        read(clientSocket, buff, 1);
    }
    
    // Read the size of the nickname (7 bytes)
    n = read(clientSocket, buff, 7); 
    buff[n] = '\0';
    msgSize = atoi(buff);
    
    // Read the actual nickname
    n = read(clientSocket, buff, msgSize); 
    buff[n] = '\0';
    nickname = buff;
    
    // Register the client in the map
    ListOfCli[buff] = clientSocket;

    string destination;
    string dataStructure;
    string msg;
    
    // 2. Infinite loop to listen to this specific client
    for (;;) {
        n = read(clientSocket, buff, 1); 
        
        if (n <= 0) {
            ListOfCli.erase(nickname);
            close(clientSocket);
            break;
        }

        if (buff[0] == 'M') { // Private Message
            // Read destination size
            n = read(clientSocket, buff, 7); 
            buff[n] = '\0';
            msgSize = atoi(buff);
            
            // Read destination name
            n = read(clientSocket, buff, msgSize); 
            buff[n] = '\0';
            destination = buff;

            // Read message size
            n = read(clientSocket, buff, 11); 
            buff[n] = '\0';
            msgSize = atoi(buff);
            
            // Read actual message
            n = read(clientSocket, buff, msgSize); 
            buff[n] = '\0';
            msg = buff;

            // Build the packet to forward to the destination
            dataStructure = "";
            dataStructure += 'm';
            dataStructure += zeroPad(nickname.size(), 7); 
            dataStructure += nickname;
            dataStructure += zeroPad(msg.size(), 11);
            dataStructure += msg;

            // Send to specific client
            if (ListOfCli.find(destination) != ListOfCli.end()) {
                write(ListOfCli[destination], dataStructure.c_str(), dataStructure.size());
            }

        } else if (buff[0] == 'B') {
            n = read(clientSocket, buff, 11); 
            buff[n] = '\0';
            msgSize = atoi(buff);
            
            n = read(clientSocket, buff, msgSize); 
            buff[n] = '\0';
            msg = buff;

            dataStructure = "";
            dataStructure += 'b';
            dataStructure += zeroPad(nickname.size(), 7);
            dataStructure += nickname; 
            dataStructure += zeroPad(msg.size(), 11);
            dataStructure += msg;

            for (map<string, int>::iterator it = ListOfCli.begin(); it != ListOfCli.end(); ++it) {
                write(it->second, dataStructure.c_str(), dataStructure.size());
            }

        } else if (buff[0] == 'Q') {
            ListOfCli.erase(nickname); 
            close(clientSocket);
            break;
        }      
    }
}

int main(void) {
    struct sockaddr_in stSockAddr;
    int ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    if(-1 == ServerSocket) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }
 
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
 
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(1100);
    stSockAddr.sin_addr.s_addr = INADDR_ANY;
 
    if(-1 == bind(ServerSocket, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("Error bind failed");
        close(ServerSocket);
        exit(EXIT_FAILURE);
    }
 
    if(-1 == listen(ServerSocket, 10)) {
        perror("Error listen failed");
        close(ServerSocket);
        exit(EXIT_FAILURE);
    }
 
    for(;;) {
        int ClientSocket = accept(ServerSocket, NULL, NULL);
        if(0 > ClientSocket) {
            perror("Error accept failed");
            close(ServerSocket);
            exit(EXIT_FAILURE);
        }
        thread(threadReadClient, ClientSocket).detach();
    }
 
    close(ServerSocket);
    return 0; 
}