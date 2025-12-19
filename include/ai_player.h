#pragma once

typedef struct {
    int sy, sx, dy, dx;
    int score;
} Move;

class AIPlayer {
public:
    AIPlayer() {}
    ~AIPlayer() {}
    int make_move();
private:
    int execute_move(Move& move);
};