/*
    Name: Harrison Day
    Date: 11/04/25
    Desc: Board state and functions that operate on board state.
          Check for valid moves, update the board state, resolve
          piece placements.
*/

#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>

namespace Board {

struct State;
bool checkVertical(int row, int col, State& state);
bool checkHorizontal(int row, int col, State& state);
bool checkDiagonal(int row, int col, State& state);
bool isValidMove(int row, int col, State& state);
State resolve(int row, int col, State& state);
void updateScore(State& state);
bool isGameOver(State& state);

struct State {
    char board[8][8];
    char turn = 'b';
    int white = 2;
    int black = 2;
    std::unordered_map<std::string, State> possibleStates;

    State() { clear(); }

    void clear() {
        for (int row = 0; row < 8; ++row)
            for (int col = 0; col < 8; ++col)
                board[row][col] = ' ';
        
        board[3][3] = 'w';
        board[3][4] = 'b';
        board[4][3] = 'b';
        board[4][4] = 'w';

        turn = 'b';
    }

    void updatePossibleStates() {
        possibleStates.clear();
        
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                if (isValidMove(row, col, *this)) {
                    State newState = resolve(row, col, *this);
                    newState.turn = (turn == 'b') ? 'w' : 'b';
                    updateScore(newState);
                    std::string key = std::to_string(row) + ":" + std::to_string(col);
                    possibleStates[key] = newState;
                }
            }
        }
    }
    
    void place(int row, int col) {
        std::string key = std::to_string(row) + ":" + std::to_string(col);
        if (possibleStates.find(key) != possibleStates.end())
            *this = possibleStates[key];
    }
};

bool checkVertical(int row, int col, State& state) {
    /*
        Shoot a "ray" upwards and downwards. Check for collision with
        a current turn piece. Then check if there are opponent pieces
        in between. Return true if both are true.
    */
    if (state.board[row][col] != ' ') return false;
    
    char currentPlayer = state.turn;
    char opponent = (currentPlayer == 'b') ? 'w' : 'b';
    bool foundCapture = false;
    
    int tempRow = row - 1;
    bool hasOpponentBetween = false;
    while (tempRow >= 0 && state.board[tempRow][col] == opponent) {
        hasOpponentBetween = true;
        tempRow--;
    }
    if (hasOpponentBetween && tempRow >= 0 && state.board[tempRow][col] == currentPlayer)
        foundCapture = true;
    
    tempRow = row + 1;
    hasOpponentBetween = false;
    while (tempRow < 8 && state.board[tempRow][col] == opponent) {
        hasOpponentBetween = true;
        tempRow++;
    }
    if (hasOpponentBetween && tempRow < 8 && state.board[tempRow][col] == currentPlayer)
        foundCapture = true;
    
    return foundCapture;
}

bool checkHorizontal(int row, int col, State& state) {
    /*
        Shoot a "ray" left and right. Check for collision with
        a current turn piece. Then check if there are opponent pieces
        in between. Return true if both are true.
    */
    if (state.board[row][col] != ' ') return false;
    
    char currentPlayer = state.turn;
    char opponent = (currentPlayer == 'b') ? 'w' : 'b';
    bool foundCapture = false;
    
    int tempCol = col - 1;
    bool hasOpponentBetween = false;
    while (tempCol >= 0 && state.board[row][tempCol] == opponent) {
        hasOpponentBetween = true;
        tempCol--;
    }
    if (hasOpponentBetween && tempCol >= 0 && state.board[row][tempCol] == currentPlayer)
        foundCapture = true;
    
    tempCol = col + 1;
    hasOpponentBetween = false;
    while (tempCol < 8 && state.board[row][tempCol] == opponent) {
        hasOpponentBetween = true;
        tempCol++;
    }
    if (hasOpponentBetween && tempCol < 8 && state.board[row][tempCol] == currentPlayer)
        foundCapture = true;
    
    return foundCapture;
}

bool checkDiagonal(int row, int col, State& state) {
    /*
        Shoot a "ray" in all 4 diagonal directions. Check for collision with
        a current turn piece. Then check if there are opponent pieces
        in between. Return true if both are true.
    */
    if (state.board[row][col] != ' ') return false;
    
    char currentPlayer = state.turn;
    char opponent = (currentPlayer == 'b') ? 'w' : 'b';
    bool foundCapture = false;
    
    int directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
    
    for (int dir = 0; dir < 4; dir++) {
        int rowDir = directions[dir][0];
        int colDir = directions[dir][1];
        
        int tempRow = row + rowDir;
        int tempCol = col + colDir;
        bool hasOpponentBetween = false;
        
        while (tempRow >= 0 && tempRow < 8 && tempCol >= 0 && tempCol < 8 && 
               state.board[tempRow][tempCol] == opponent) {
            hasOpponentBetween = true;
            tempRow += rowDir;
            tempCol += colDir;
        }
        
        if (hasOpponentBetween && tempRow >= 0 && tempRow < 8 && tempCol >= 0 && tempCol < 8 && 
            state.board[tempRow][tempCol] == currentPlayer) {
            foundCapture = true;
            break;
        }
    }
    
    return foundCapture;
}

bool isValidMove(int row, int col, State& state) {
    /*
        Check all directions. Return true if it is a valid position
        for any of the directions.
    */
    if (row < 0 || row >= 8 || col < 0 || col >= 8)
        return false;
    
    if (state.board[row][col] != ' ')
        return false;
    
    return checkVertical(row, col, state) || 
           checkHorizontal(row, col, state) || 
           checkDiagonal(row, col, state);
}

void printState(State& state) {
    /*
        Print a nicely formatted board state in terminal
    */
    std::cout << "   ";
    for (char col = 'A'; col <= 'H'; col++)
        std::cout << col << "  ";
    std::cout << "\n";
    
    for (int row = 0; row < 8; row++) {
        std::cout << (row + 1) << "  ";
        for (int col = 0; col < 8; col++)
            std::cout << state.board[row][col] << "  ";
        std::cout << "\n";
    }
}

State resolve(int row, int col, State& state) {
    /*
        
    */
    if (!isValidMove(row, col, state))
        return state;
    
    State newState = state;
    newState.possibleStates.clear();
    newState.board[row][col] = state.turn;
    
    char currentPlayer = state.turn;
    char opponent = (currentPlayer == 'b') ? 'w' : 'b';
    
    int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
    
    for (int dir = 0; dir < 8; dir++) {
        int rowDir = directions[dir][0];
        int colDir = directions[dir][1];
        
        int tempRow = row + rowDir;
        int tempCol = col + colDir;
        std::vector<std::pair<int, int>> toFlip;
        
        while (tempRow >= 0 && tempRow < 8 && tempCol >= 0 && tempCol < 8 && 
               newState.board[tempRow][tempCol] == opponent) {
            toFlip.push_back({tempRow, tempCol});
            tempRow += rowDir;
            tempCol += colDir;
        }
        
        if (!toFlip.empty() && tempRow >= 0 && tempRow < 8 && tempCol >= 0 && tempCol < 8 && 
            newState.board[tempRow][tempCol] == currentPlayer) {
            for (auto& pos : toFlip)
                newState.board[pos.first][pos.second] = currentPlayer;
        }
    }
    
    newState.turn = opponent;
    return newState;
}

void updateScore(State& state) {
    /*
        Count each white and black piece currently on a board.
        Update the State's white and black counts.
    */
    state.white = 0;
    state.black = 0;
    
    for (int row = 0; row < 8; row++)
        for (int col = 0; col < 8; col++)
            if (state.board[row][col] == 'w')
                state.white++;
            else if (state.board[row][col] == 'b')
                state.black++;
}

bool isGameOver(State& state) {
    /*
        Check if there are any possible moves for the current turn.
        If not, game has ended.
    */
    state.updatePossibleStates();
    if (!state.possibleStates.empty())
        return false;
    
    State tempState = state;
    tempState.turn = (state.turn == 'b') ? 'w' : 'b';
    tempState.updatePossibleStates();
    
    return tempState.possibleStates.empty();
}

}