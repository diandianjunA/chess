#pragma once
#include <vector>
class Game {
public:
    Game() : current_round(1), white_in_check(false), black_in_check(false)
            , white_in_checkmate(false), black_in_checkmate(false), white_cap_count(0)
            , black_cap_count(0) , selected_piece(0), selected_x(0), selected_y(0)
            , predicted_moves(8, std::vector<int>(8, 0)), choose(false) {};
    ~Game() {};
    void double_mode_start();
    void single_mode_start();
    void restart();

private:
    void draw_ui(int start_y, int start_x, int cur_y, int cur_x);
    void draw_dashboard(int start_y, int start_x, int turn, int round);
    int calculate_score(int side);
    int selected_piece;
    int selected_x;
    int selected_y;
    std::vector<std::vector<int>> predicted_moves;
    int current_round;
    bool white_in_check;
    bool black_in_check;
    bool white_in_checkmate;
    bool black_in_checkmate;

    int white_captured[16]; 
    int black_captured[16];
    int white_cap_count;
    int black_cap_count;

    bool choose;
};
