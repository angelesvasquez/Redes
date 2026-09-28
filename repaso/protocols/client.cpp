// Red TCP/IP
// Se conecta a un servidor a traves de una IP y un puerto
// Enviamos un mensaje de texto y luego cierra la conexion 

#include <iostream>
#include <cstring>

// para trabajar con sockets
// socket(), connect(), shutdown()
#include <sys/socket.h> 
#include <netinet/in.h> // Estructura sockaddr_in
#include <arpa/inet.h> // conversion de IPs, inet_pton()

#include <unistd.h> // write(), close()

#include <thread>

using namespace std;

string zeroPad(int number, int size){
    string str = to_string(number);
    if(str.length() >= (size_t)size) return str;
    return string(size-str.length(), '0') + str;        
}

void ThreadReadServer(int IdSocket){
    char buffer[1000];
    int n, size;


    while(1){   
        n = read(IdSocket, buffer,1);
        if(n<=0) break;
        
        char action = buffer[0];

        string origin, msg;
        if(action == 'm' || action == 'b'){
            n = read(IdSocket,buffer,7);
            buffer[n] = '\0';
            size = atoi(buffer);

            n = read(IdSocket,buffer,size);
            buffer[n] = '\0';
            origin = buffer;

            n = read(IdSocket,buffer,11);
            buffer[n] = '\0';
            size = atoi(buffer);

            n = read(IdSocket,buffer,size);
            buffer[n] = '\0';
            msg = buffer;

            if(action == 'm') cout<<"Private ["<<origin<<"]: "<<msg<<endl;
            if(action == 'b') cout<<"["<<origin<<"]: "<<msg<<endl;
        }
        else if(action == 'l'){
            n = read(IdSocket,buffer,17);
            buffer[n] = '\0';
            size = atoi(buffer);

            n = read(IdSocket,buffer,size);
            buffer[n] = '\0';
            cout<<"Clients: "<<buffer<<endl;
        }
        else if(action == 'e'){
            n = read(IdSocket,buffer,11);
            buffer[n] = '\0';
            size = atoi(buffer);

            read(IdSocket,buffer,size);
            buffer[n] = '\0';
            cout<<"Error: "<<buffer<<endl;
        }
    }
}

int main(int argc, char *argv[]){
    
    if(argc != 3) {
        printf("%s <ip> <puerto>\n", argv[0]);
        return 1;
    }
    
    struct sockaddr_in stSockAddr;

    // (domain: Protocolo de red IPv4, type: conexion TCP, protocol: TCP)
    int SocketFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    int Res;
    int n;

    if(-1 == SocketFD){
        perror("cannot create socket");
        return 1;
    }
    

    //stSockAddr = 000000000000000000...
    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[2]));

    //int inet_pton(int af, const char *src, void *dst);
    Res = inet_pton(AF_INET, argv[1], &stSockAddr.sin_addr);

    if(Res == 0){
        perror("char string (second parameter does not contain valid ipaddress");
        close(SocketFD);
        return 1;
    } else if(Res < 0){
        perror("error: first parameter is not a valid address family");
        close(SocketFD);
        return 1;
    }

    if(-1 == connect(SocketFD, (const struct sockaddr*)&stSockAddr, sizeof(struct sockaddr_in))){
        perror("connection failed");
        close(SocketFD);
        return 1;
    }

    string nickname;
    cout<<"Nickname: ";
    getline(cin, nickname);

    string data = "N" + zeroPad(nickname.size(),7) + nickname;
    n = write(SocketFD, data.c_str(),data.size());

    thread(ThreadReadServer,SocketFD).detach();
    
    cout<<"[B]: Broadcast, [M]: Private Message, [Q] Quit, [L] List of Clients\n";
    string option;
    for(;;){
        cout<<"> ";
        getline(cin,option);
        
        string msg, destination, size;
        if(option == "Q" || option == "q"){
            write(SocketFD, "Q", 1);
            break;
        }
        else if(option == "B" || option == "b"){
            cout<<"Message: "; 
            getline(cin, msg);
            data = "B" + zeroPad(msg.size(),11) + msg;

            write(SocketFD, data.c_str(), data.size());
        }
        else if(option == "M" || option == "m"){
            cout<<"Destination nickname: ";
            getline(cin,destination);
            cout<<"Private message: ";
            getline(cin,msg);
            data = "M" + zeroPad(destination.size(),7) + destination + zeroPad(msg.size(),11) + msg;
            write(SocketFD, data.c_str(), data.size());
        } 
        else if(option == "L" || option == "l"){
            write(SocketFD, "L", 1);
        }
        else cout<<"Invalid option"<<endl;
        
    }
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);

    return 0;
}
