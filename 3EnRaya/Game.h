// Example program
#include <iostream>
#include <string>

using namespace std;

struct Board{
    char board[9] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    char current;
    bool winner;
    
    char& operator[](int pos){return board[pos];}
    const char& operator[](int pos) const {return board[pos];}

    bool setMove(char s, int pos){
        if(pos >8 || pos < 0){
            cout<<"Fuera del rango"<<endl;
            return 0;
        }
        if(board[pos] != ' '){
            cout<<"Casilla ocupada"<<endl;
            return 0;
        } else {
            board[pos] = s;
            return 1;
        }
    }
public:
    Board(){
        current = 'X';
        winner = 0;
    }
    
};


class Game{
    Board b;
    string player1;
    string player2;
    
    bool checkWin(char p){
        int wins[8][3] = {
            {0,1,2}, {3,4,5}, {6,7,8},
            {0,3,6}, {1,4,7}, {2,5,8},
            {0,4,8}, {2,4,6}
        };
        for (auto& w : wins) {
            if (b[w[0]] == p && b[w[1]] == p && b[w[2]] == p) return true;
        }
        return false;
    }
    
public:
    Game(string p1, string p2){
        player1 = p1;
        player2 = p2;
    }
    void printBoard(){
        cout << " " << b[0] << " | " << b[1] << " | " << b[2] << "   (0, 1, 2)\n";
        cout << "-----------\n";
        cout << " " << b[3] << " | " << b[4] << " | " << b[5] << "   (3, 4, 5)\n";
        cout << "-----------\n";
        cout << " " << b[6] << " | " << b[7] << " | " << b[8] << "   (6, 7, 8)\n\n";
    }
};