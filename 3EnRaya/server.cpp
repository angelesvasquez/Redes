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
mutex ttt_mutex;
Game* game = 0;
int clientWaiting = -1;
string clientWaitingNick = "";

string zeroPad(int number, int size){
    string str = to_string(number);
    if(str.length() >= (size_t)size) return str;
    return string(size - str.length(), '0')+str;
}

void avisarTurno(){
    string d = "Tt";
    d += game->current;
    write(game->p1, d.c_str(), d.size());
    write(game->p2, d.c_str(), d.size());
    for(int s : game->spectators) write(s, d.c_str(), d.size());
}

void salirDePartida(int fd){
    lock_guard<mutex> lock(ttt_mutex);
    if(fd == clientWaiting){ clientWaiting = -1; clientWaitingNick = ""; }
    if(game != 0){
        if(fd == game->p1 || fd == game->p2){
            int otro = (fd == game->p1) ? game->p2 : game->p1;
            write(otro, "TW", 2);
            delete game;
            game = 0;
        } else {
            auto& v = game->spectators;
            v.erase(remove(v.begin(), v.end(), fd), v.end());
        }
    }
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
             {
                lock_guard<mutex> lock(ttt_mutex);
                if(IdSocket == clientWaiting){ clientWaiting = -1; clientWaitingNick = ""; }
            }
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
                    if(!V.empty()) V+= ", ";
                    string estado = "Nada";
                     if(game != 0 && p.second == game->p1){
                        estado = "X";
                    }
                    else if(game != 0 && p.second == game->p2){
                        estado = "O";
                    }
                    else if(clientWaiting == p.second){
                        estado = "Esperando contrincante";
                    }
                    else if(game != 0){
                        for(int sp : game->spectators){
                            if(p.second == sp){
                                estado = "V";
                                break;
                            }
                        }
                    }

                    V += p.first + ":" + "[" + estado + "]";
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

            lock_guard<mutex> lock(ttt_mutex);

            if(s == 'P') { // Play
                if(game!= 0){
                    msg = "Ya hay una partida en curso";
                    data = "e" + zeroPad(msg.size(),11) + msg;
                    write(IdSocket,data.c_str(),data.size());
                }
                else if(clientWaiting == -1) {
                    clientWaiting = IdSocket; 
                    clientWaitingNick = nickname;
                
                }
                else if(clientWaiting == IdSocket){
                    msg = "Ya estas esperando";
                    data = "e" + zeroPad(msg.size(),11) + msg;
                    write(IdSocket,data.c_str(),data.size());
                }
                else{
                    //if(game != 0) delete game;
                    game = new Game(clientWaitingNick,nickname,clientWaiting,IdSocket);
               
                    write(game->p1, "TtX", 3);
                    write(game->p2, "TtO", 3);
                    data = "TT";
                    data.append(game->board,9);
                    write(game->p1,data.c_str(),data.size());
                    write(game->p2,data.c_str(),data.size());

                    avisarTurno();

                    clientWaiting = -1;
                    clientWaitingNick = "";


                    
                }
            } else if(s == 'M'){
                n = read(IdSocket,buffer,1);
                buffer[n] = '\0';
                int pos = (buffer[0] - '0') - 1;

                if(game == 0 || (IdSocket != game->p1 && IdSocket != game->p2)){
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
                        for(int sp : game->spectators) write(sp, data.c_str(), data.size());

                        if(game->checkWin(symbol)){
                            int loserId = (IdSocket == game->p1) ? game->p2 : game->p1;
                            write(IdSocket,"TW",2);
                            write(loserId,"TO",2);
                            //game->active = false;
                            delete game;
                            game = 0;
                        } else{
                            bool full = true;
                            for(int i = 0; i < 9; i++) if(game->board[i] == ' ') full = false;
                            if(full){
                                write(game->p1, "TO",2);
                                write(game->p2, "TO",2);
                                delete game;
                                game = 0;
                            } else{
                                game->current = (game->current == 'X') ? 'O': 'X';
                                avisarTurno();
                            }
                        }
                    }
                }

            } else if(s == 'V'){
                if(game == 0){
                    msg = "No hay partida activa";
                    data = "e" + zeroPad(msg.size(),11) + msg;
                    write(IdSocket,data.c_str(),data.size());
                } 
                else if(IdSocket == game->p1 || IdSocket == game->p2){
                    msg = "Ya eres jugador";
                    data = "e" + zeroPad(msg.size(),11) + msg;
                    write(IdSocket, data.c_str(), data.size());
                }
                else{
                    game->addSpectator(IdSocket);
                    data = "TT";
                    data.append(game->board,9);
                    write(IdSocket,data.c_str(),data.size());
                }
            }
        }
        else if(action == 'Q'){
            salirDePartida(IdSocket);
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