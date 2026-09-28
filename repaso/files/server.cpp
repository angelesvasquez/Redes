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
#include <mutex>
#include <map>

using namespace std;

map<string,int> ListOfCli;
mutex map_mutex;

// 5 -> 0005
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
            // for(map<string,int>::iterator it = ListOfCli.begin(); it != ListOfCli.end(); it++){
            //     if(it->second != IdSocket) write(it->second,data.c_str(),data.size());
            // }
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
            file = atoll(buffer); // cadena -> long long int

            remaining = fsize;
            while(remaining > 0){
                n = read(IdSocket, buffer, remaining > 1000 ? 1000 : remaining);
                if(n <= 0) break;
                file.append(buffer, n);
                remaining -= n;
            }
            if(remaining > 0){   // el cliente se cayó a mitad del archivo
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

        int ClientSocket = accept(SocketFD, NULL, NULL);
        // accept espera hasta que llegue un cliente
        // ponemos NULL si NO nos interesa saber quien se conecto
        // Se crea un socket unicamente para el cliente

        if(0 > ClientSocket){
            perror("error accept failed");
            continue;
        }

        
        thread(ThreadReadClient,ClientSocket).detach();

    }
    close(SocketFD);
    return 0;
}