#pragma once
#include <cstdlib>
#include <vector>

// 棋子代号定义
#define EMPTY  0
#define PAWN   1 // 兵
#define KNIGHT 2 // 马
#define BISHOP 3 // 象
#define ROOK   4 // 车
#define QUEEN  5 // 后
#define KING   6 // 王
#define PREDICTED_MOVE 7 // 预测移动

extern int board[8][8];

inline const char* get_piece_letter(int piece_val) {
    switch (abs(piece_val)) {
        case PAWN:   return " P ";
        case KNIGHT: return " N ";
        case BISHOP: return " B ";
        case ROOK:   return " R ";
        case QUEEN:  return " Q ";
        case KING:   return " K ";
        default: return " ";
    }
}

// 棋子分值定义
int get_piece_value(int piece);

void predict_move(int y, int x, std::vector<std::vector<int>>& predicted_moves);

bool is_legal_move(int from_y, int from_x, int to_y, int to_x);

bool is_attacked(int ty, int tx, int attacker_side);

bool is_in_check(int side);

bool try_move(int sy, int sx, int dy, int dx);

bool is_checkmate(int side);