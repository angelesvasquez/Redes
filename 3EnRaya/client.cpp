// Client
#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <unistd.h>

#include <thread>

using namespace std;

bool observador = 0;

string zeroPad(int number, int size){
    string str = to_string(number);
    if(str.length() >= (size_t)size) return str;
    return string(size-str.length(), '0') + str;        
}

void ThreadReadServer(int IdSocket){
    char MySymbol = ' ';
    char buffer[1000];
    int n, size;
    long long fsize, remaining;

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

            n = read(IdSocket,buffer,size);
            buffer[n] = '\0';
            string err = buffer;
            if(err == "No hay partida activa" || err == "Ya eres jugador") observador = false;
            cout<<"Error: "<<buffer<<endl;
            cout<<"> "<<flush;
        }
        else if(action == 'f'){
            string fileName, file;

            n = read(IdSocket,buffer,13);
            buffer[n] = '\0';
            size = atoi(buffer);

            n = read(IdSocket,buffer,size);
            buffer[n] = '\0';
            origin = buffer;

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

            fileName = fileName.substr(fileName.find_last_of('/') + 1);

            ofstream out("recv_" + fileName, ios::binary);
            out.write(file.data(), file.size());
            cout<<"File '"<<fileName<<"' received from "<<origin<<" ("<<file.size()<<" bytes)"<<endl;
        }
        else if(action == 'T'){
            n = read(IdSocket,buffer,1);
            char s = buffer[0];

            if(s == 't'){
                n = read(IdSocket,buffer,1);
                buffer[n] = '\0';
                if(observador){
                    cout<<"Turno de "<<buffer[0]<<endl;
                }
                else if(MySymbol == ' '){
                    MySymbol = buffer[0];
                    cout<<"Partida creada. Tu ficha es: "<<buffer[0]<<endl;
                }
                else if(MySymbol == buffer[0]){
                    cout<<"Es tu turno ("<<MySymbol<<")"<<endl;
                }
                else{
                    cout<<"Turno del rival(" <<buffer[0]<<")"<<endl;
                }
                cout << "> " << flush;
            } else if(s == 'T'){
                n = read(IdSocket,buffer,9);
                buffer[n] = '\0';
                cout << endl;
                cout << " " << buffer[0] << " | " << buffer[1] << " | " << buffer[2] << "   (1, 2, 3)\n";
                cout << "-----------\n";
                cout << " " << buffer[3] << " | " << buffer[4] << " | " << buffer[5] << "   (4, 5, 6)\n";
                cout << "-----------\n";
                cout << " " << buffer[6] << " | " << buffer[7] << " | " << buffer[8] << "   (7, 8, 9)\n\n";
                cout << "> " << flush;
            } else if(s=='W'){
                MySymbol = ' '; 
                cout<<"Ganaste la partida!!"<<endl;
                cout << "> " << flush;
            } else if(s=='O'){
                MySymbol = ' ';
                cout<<"Perdiste la partida.."<<endl;
                cout << "> " << flush;
            }
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
    

    memset(&stSockAddr, 0, sizeof(struct sockaddr_in));

    stSockAddr.sin_family = AF_INET;
    stSockAddr.sin_port = htons(atoi(argv[2]));

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
    
    cout<<"[B]: Broadcast, [M]: Private Message, [F]: File, [Q]: Quit, [L] List of Clients, \nTic-Tac-Toe\n[P] Play, [V] Spectator, [1-9] Movimiento\n";
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
        else if(option == "F" || option == "f"){
            string path, fileName, file;
            cout<<"Destination nickname: ";
            getline(cin,destination);
            cout<<"File path: ";
            getline(cin,path);

            ifstream in(path, ios::binary);
            if(!in){
                cout<<"Cannot open file"<<endl;
                continue;
            }
            stringstream ss;
            ss << in.rdbuf();
            file = ss.str();

            fileName = path.substr(path.find_last_of('/') + 1);

            data = "F" + zeroPad(destination.size(),13) + destination
                + zeroPad(fileName.size(),13) + fileName
                + zeroPad(file.size(),25) + file;
            write(SocketFD, data.c_str(), data.size());
        }
        else if(option == "P" || option == "p"){
            observador = 0;
            write(SocketFD,"TP",2);
            cout<<"Uniendose a una partida.."<<endl;
        }
        else if(option == "V" || option == "v"){
            observador = 1;
            write(SocketFD,"TV",2);
            cout<<"Entrando como observador.."<<endl;
        }
        else if(option.size() == 1 && option[0] >= '1' && option[0] <= '9'){
            data = "TM" + option.substr(0,1);
            write(SocketFD,data.c_str(),data.size());
        }
        else cout<<"Invalid option"<<endl;
        
    }
    shutdown(SocketFD, SHUT_RDWR);
    close(SocketFD);

    return 0;
}
