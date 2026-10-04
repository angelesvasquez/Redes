// Server
#include <iostream>
#include "Game.h"
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <unistd.h>
#include <vector>
#include <thread>
#include <mutex>
#include <map>

using namespace std;

map<string,int> ListOfCli;
mutex map_mutex;
Game* game = 0;
int clientWaiting = -1;
string clientWaitingNick = "";

string zeroPad(int number, int size){
    string str = to_string(number);
    if(str.length() >= (size_t)size) return str;
    return string(size - str.length(), '0')+str;
}

void ThreadReadClient(int IdSocket){
    string nickname;
    char buffer[1000];
    int n, size;
    long long fsize, remaining;

    read(IdSocket,buffer,1);
    while(buffer[0] != 'N'){
        read(IdSocket,buffer,1);
    }
    n = read(IdSocket,buffer,7);
    buffer[n] = '\0';
    size = atoi(buffer);

    n = read(IdSocket,buffer,size);
    buffer[n] = '\0';
    nickname = buffer;

    {
        lock_guard<mutex> lock(map_mutex);
        ListOfCli[nickname] = IdSocket;
    }

    cout<<"Cliente registrado: "<<nickname<<endl;

    string data, destination, msg;

    while(1){
        n = read(IdSocket,buffer,1);
        if(n <= 0){
            lock_guard<mutex> lock(map_mutex);
            ListOfCli.erase(nickname);
            close(IdSocket);
            break;
        } 
        char action = buffer[0];
        data = "";
        if(action == 'M'){
            n = read(IdSocket,buffer,7);
            buffer[n] = '\0';
            size = atoi(buffer);

            n = read(IdSocket,buffer,size);
            buffer[n] = '\0';
            destination = buffer;

            n = read(IdSocket,buffer,11);
            buffer[n] = '\0';
            size = atoi(buffer);

            n = read(IdSocket,buffer,size);
            buffer[n] = '\0';
            msg = buffer;

            data = "m" + zeroPad(nickname.size(), 7) + nickname + zeroPad(msg.size(),11) + msg;

            lock_guard<mutex> lock(map_mutex);
            if(ListOfCli.find(destination) != ListOfCli.end()){
                write(ListOfCli[destination],data.c_str(), data.size());
            } else{
                msg = "User not found";
                data = "e" + zeroPad(msg.size(),11) + msg;
                write(IdSocket, data.c_str(), data.size());
            }
        }
        else if(action == 'B'){
            n = read(IdSocket,buffer,11);
            buffer[n] = '\0';
            size = atoi(buffer);

            n = read(IdSocket,buffer,size);
            buffer[n] = '\0';
            msg = buffer;

            data = 'b' + zeroPad(nickname.size(),7) + nickname + zeroPad(msg.size(),11) + msg;


            lock_guard<mutex> lock(map_mutex);

            for(auto& p: ListOfCli){
                if(p.second != IdSocket){
                    write(p.second, data.c_str(), data.size());
                }
            }
        }
        else if(action == 'L'){
            string V;
            {
                lock_guard<mutex> lock(map_mutex);
                for(auto& p: ListOfCli){
                    if(!V.empty()) V+= ",";
                    V += p.first;
                }
            }
            data = "l" + zeroPad(V.size(), 17) + V;
            write(IdSocket,data.c_str(), data.size());
        }
        else if(action == 'F'){
            string fileName, file;
            n = read(IdSocket, buffer, 13);
            buffer[n] = '\0';
            size = atoi(buffer);

            n = read(IdSocket,buffer,size);
            buffer[n] = '\0';
            destination = buffer;

            n = read(IdSocket,buffer,13);
            buffer[n] = '\0';
            size = atoi(buffer);

            n = read(IdSocket,buffer,size);
            buffer[n] = '\0';
            fileName = buffer;

            n = read(IdSocket,buffer,25);
            buffer[n] = '\0';
            fsize = atoll(buffer);

            remaining = fsize;
            while(remaining > 0){
                n = read(IdSocket, buffer, remaining > 1000 ? 1000 : remaining);
                if(n <= 0) break;
                file.append(buffer, n);
                remaining -= n;
            }
            if(remaining > 0){
                lock_guard<mutex> lock(map_mutex);
                ListOfCli.erase(nickname);
                close(IdSocket);
                break;
            }

            lock_guard<mutex> lock(map_mutex);
            if(ListOfCli.find(destination) != ListOfCli.end()){
                data = "f" + zeroPad(nickname.size(),13) + nickname
                        + zeroPad(fileName.size(),13) + fileName
                        + zeroPad(file.size(),25) + file;
                write(ListOfCli[destination], data.c_str(), data.size());
            }
            else{
                msg = "User not found";
                data = "e" + zeroPad(msg.size(),11) + msg;
                write(IdSocket, data.c_str(), data.size());
            }
        }
        else if(action == 'T'){
            n = read(IdSocket,buffer,1);
            buffer[n] = '\0';
            char s = buffer[0];

            if(s == 'P') { // Play
                if(clientWaiting == -1) {
                    clientWaiting = IdSocket; 
                    clientWaitingNick = nickname;
                
                }
                else{

                    game = new Game(clientWaitingNick,nickname,clientWaiting,IdSocket);
                    data = "TtX";  
                    write(game->p1,data.c_str(),data.size());
                    data = "TtO";
                    write(game->p2,data.c_str(),data.size());

                    data = "TT";
                    data.append(game->board,9);
                    write(game->p1,data.c_str(),data.size());
                    write(game->p2,data.c_str(),data.size());

                    clientWaiting = -1;
                    clientWaitingNick = "";


                    
                }
            } else if(s == 'M'){
                n = read(IdSocket,buffer,1);
                buffer[n] = '\0';
                int pos = (buffer[0] - '0') - 1;

                if(game == 0 || !game->active || (IdSocket != game->p1 && IdSocket != game->p2)){
                    msg = "No estas en una partida";
                    data = "e" + zeroPad(msg.size(),11) + msg;
                    write(IdSocket,data.c_str(),data.size());
                } else{
                    char symbol;
                    if(IdSocket == game->p1) symbol = 'X';
                    if(IdSocket == game->p2) symbol = 'O';

                    if(symbol != game->current){
                        msg = "No es tu turno";
                        data = "e" + zeroPad(msg.size(),11) + msg;
                        write(IdSocket, data.c_str(), data.size());
                    }
                    else if(!game->setMove(pos)){
                        msg = "Movimiento Invalido";
                        data = "e" + zeroPad(msg.size(),11) + msg;
                        write(IdSocket,data.c_str(),data.size());
                    }
                    else{
                        data = "TT";
                        data.append(game->board, 9);
                        write(game->p1, data.c_str(), data.size());
                        write(game->p2, data.c_str(), data.size());
                        for(int s : game->spectators) write(s, data.c_str(), data.size());

                        if(game->checkWin(symbol)){
                            int loserId = (IdSocket == game->p1) ? game->p2 : game->p1;
                            write(IdSocket,"TW",2);
                            write(loserId,"TO",2);
                            // game active = false
                        
                        } else{
                            game->current = (game->current == 'X') ? 'O': 'X';
                            int nextTurn = (game->current == 'X') ? game->p1 : game->p2;
                            data = "Tt"; 
                            data += game->current;
                            write(nextTurn,data.c_str(),data.size());
                        }
                    }
                }

            } else if(s == 'V'){
                if(game != 0){
                    game->addSpectator(IdSocket);
                    data = "TT";
                    data.append(game->board,9);
                    write(IdSocket,data.c_str(),data.size());
                } else{
                    msg = "No hay partida activa";
                    data = "e" + zeroPad(msg.size(),11) + msg;
                    write(IdSocket,data.c_str(),data.size());
                }
            }
        }
        else if(action == 'Q'){
            lock_guard<mutex> lock(map_mutex);
            ListOfCli.erase(nickname);
            close(IdSocket);
            break;
        }
    }

}

int main(int argc, char* argv[]){

    if(argc != 2) {
      printf("%s <puerto>\n", argv[0]);
      exit(EXIT_FAILURE);
    }

    struct sockaddr_in stSockAddr;
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    int n;
    char buffer[256];

    if(SocketFD == -1){
        perror("can not create socket");
        exit(EXIT_FAILURE);
    }

    memset(&stSockAddr,0,sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[1]));
    stSockAddr.sin_addr.s_addr = INADDR_ANY;

    if(-1 == bind(SocketFD, (const struct sockaddr*)&stSockAddr, sizeof(sockaddr_in))){
        perror("error bind failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    if(-1 == listen(SocketFD,10)) {
        perror("error listen failed");
        close(SocketFD);
        exit(EXIT_FAILURE);
    }

    for(;;){

        int ClientSocket = accept(SocketFD, NULL, NULL);

        if(0 > ClientSocket){
            perror("error accept failed");
            continue;
        }
        
        thread(ThreadReadClient,ClientSocket).detach();

    }
    close(SocketFD);
    return 0;
}