// Example program
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>

using namespace std;


class Game{   
    public:
    bool active = 0;
    string p1N;
    string p2N;
    int p1,p2;
    vector<int> spectators;
    
    char board[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    char current;
    char winner;
    
    Game(string ps1, string ps2, int id1, int id2){
        p1N = ps1;
        p2N = ps2;
        p1 = id1; p2 = id2;
        active = 1;
        current = 'X';
        winner = ' ';
    }

    bool setMove(int pos){
        if(pos >8 || pos < 0){
            //cout<<"Fuera del rango"<<endl;
            return 0;
        }
        if(board[pos] != ' '){
            //cout<<"Casilla ocupada"<<endl;
            return 0;
        } 
        board[pos] = current;
        
        // if(checkWin(current)){
        //     winner = current;
        //     active = 0;
        //     return 1;
        // }
        return 1;
    }
    
    bool checkWin(char p){
        int wins[8][3] = {
            {0,1,2}, {3,4,5}, {6,7,8},
            {0,3,6}, {1,4,7}, {2,5,8},
            {0,4,8}, {2,4,6}
        };
        for (auto& w : wins) {
            if (board[w[0]] == p && board[w[1]] == p && board[w[2]] == p) return true;
        }
        return false;
    }

    // int getP1(){
    //     return p1;
    // }
    // int getP2(){
    //     return p2;
    // }
    void addSpectator(int idSocket){
        for(int s : spectators) if(s == idSocket) return;
        spectators.push_back(idSocket);
    }
    void removeSpectator(int idSocket){
        spectators.erase(remove(spectators.begin(),spectators.end(),idSocket),spectators.end());
    }
    // vector<int>& getSpectators(){
    //     return spectators;
    // }

    void printBoard(){
        cout << " " << board[0] << " | " << board[1] << " | " << board[2] << "   (0, 1, 2)\n";
        cout << "-----------\n";
        cout << " " << board[3] << " | " << board[4] << " | " << board[5] << "   (3, 4, 5)\n";
        cout << "-----------\n";
        cout << " " << board[6] << " | " << board[7] << " | " << board[8] << "   (6, 7, 8)\n\n";
    }
};
