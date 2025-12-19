#pragma once
#include <vector>
class Game {
public:
    Game() {};
    ~Game() {};
    void double_mode_start();
    void single_mode_start();

private:
    void draw_ui(int start_y, int start_x, int cur_y, int cur_x);
    void draw_dashboard(int start_y, int start_x, int turn, int round);
    int calculate_score(int side);
    int selected_piece = 0;
    int selected_x = 0;
    int selected_y = 0;
    std::vector<std::vector<int>> predicted_moves = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
    };
    int current_round = 1;
    bool white_in_check = false;
    bool black_in_check = false;
    bool white_in_checkmate = false;
    bool black_in_checkmate = false;

    int white_captured[16]; 
    int black_captured[16];
    int white_cap_count = 0;
    int black_cap_count = 0;
};
