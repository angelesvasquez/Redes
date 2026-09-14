/* Client code in C++ */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

string zeroPad(int number, int size) {
    string str = to_string(number);
    if (str.length() >= size) return str;
    return string(size - str.length(), '0') + str;
}

void threadReadServer(int SocketFD) {
    char buff[1000];
    int n, msgSize;

    while (true) {
        n = read(SocketFD, buff, 1);
        if (n <= 0) {
            cout << "\n[The server closed the connection]" << endl;
            exit(0);
        }

        if (buff[0] == 'm' || buff[0] == 'b') {
            char messageType = buff[0];

            n = read(SocketFD, buff, 7);
            buff[n] = '\0';
            msgSize = atoi(buff);

            n = read(SocketFD, buff, msgSize);
            buff[n] = '\0';
            string sender = buff;

            n = read(SocketFD, buff, 11);
            buff[n] = '\0';
            msgSize = atoi(buff);

            n = read(SocketFD, buff, msgSize);
            buff[n] = '\0';
            string message = buff;

            if (messageType == 'm') {
                cout << "\n[Private from " << sender << "]: " << message << "\n> " << flush;
            } else if (messageType == 'b') {
                cout << "\n[Broadcast from " << sender << "]: " << message << "\n> " << flush;
            }
        }
    }
}

int main(int argc, char* argv[]) {
    struct sockaddr_in stSockAddr;
    int Res;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (argc != 3) {
        printf("Usage: %s <ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (-1 == SocketFD) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));
    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[2]));
    Res = inet_pton(AF_INET, argv[1], &stSockAddr.sin_addr);

    if (0 > Res) {
        perror("Error: Invalid IP address");
        close(SocketFD);
        exit(EXIT_FAILURE);
    } else if (0 == Res) {
        perror("Error: String does not contain a valid IP address");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if (-1 == connect(SocketFD, (const struct sockaddr *)&stSockAddr, sizeof(struct sockaddr_in))) {
        perror("Connection failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }


    string nickname;
    cout << "Enter your Nickname: ";
    getline(cin, nickname);

    string packetN = "N" + zeroPad(nickname.size(), 7) + nickname;
    write(SocketFD, packetN.c_str(), packetN.size());

    thread t_read(threadReadServer, SocketFD);
    t_read.detach();

    string option;
    cout << "\nAvailable commands:\n [M] Private Message\n [B] Broadcast\n [Q] Quit\n";

    while (true) {
        cout << "> ";
        getline(cin, option);

        if (option == "Q" || option == "q") {
            write(SocketFD, "Q", 1);
            break;
        } 
        else if (option == "B" || option == "b") {
            string msg;
            cout << "Message for everyone: ";
            getline(cin, msg);
            
            string packetB = "B" + zeroPad(msg.size(), 11) + msg;
            write(SocketFD, packetB.c_str(), packetB.size());
        } 
        else if (option == "M" || option == "m") {
            string dest, msg;
            cout << "Recipient: ";
            getline(cin, dest);
            cout << "Message: ";
            getline(cin, msg);
            
            string packetM = "M" + zeroPad(dest.size(), 7) + dest + zeroPad(msg.size(), 11) + msg;
            write(SocketFD, packetM.c_str(), packetM.size());
        } 
        else {
            cout << "Unrecognized command.\n";
        }
    }

    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);
    return 0;
}