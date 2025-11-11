/*
    Name: Harrison Day
    Date: 11/04/25
    Desc: main function for program. Contains game loop
*/

#include <Othello.hpp>

int main() {
    Othello game;

    while (game.isRunning()) {
        game.update();
        game.render();
    }

    return 0;
}