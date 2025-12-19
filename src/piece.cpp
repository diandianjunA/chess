#include "piece.h"
#include <cstdlib>
#include <ncurses.h>

int board[8][8] = {
    {-4, -2, -3, -5, -6, -3, -2, -4}, // 黑方大子 (第8行)
    {-1, -1, -1, -1, -1, -1, -1, -1}, // 黑方兵   (第7行)
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 1,  1,  1,  1,  1,  1,  1,  1}, // 白方兵   (第2行)
    { 4,  2,  3,  5,  6,  3,  2,  4}  // 白方大子 (第1行)
};

bool is_in_board(int y, int x) {
    return y >= 0 && y < 8 && x >= 0 && x < 8;
}

// 棋子分值定义
int get_piece_value(int piece) {
    switch (abs(piece)) {
        case 1: return 10;  // 兵
        case 2: return 30;  // 马
        case 3: return 30;  // 象
        case 4: return 50;  // 车
        case 5: return 90;  // 后
        case 6: return 900; // 王
        default: return 0;
    }
}

void predict_move(int y, int x, std::vector<std::vector<int>>& predicted_moves) {
    int piece_val = board[y][x];
    switch (std::abs(piece_val)) {
        // 兵只能前进一步，不能后退
        // 如果小兵的左上和右上有棋子，则可以吃对方棋子
        case PAWN: {
            if (piece_val > 0) {
                // 初始位置可以前进两步
                if (is_in_board(y - 2, x) && try_move(y, x, y - 2, x)) {
                    predicted_moves[y - 2][x] = PREDICTED_MOVE;
                }
                if (is_in_board(y - 1, x) && try_move(y, x, y - 1, x)) {
                    predicted_moves[y - 1][x] = PREDICTED_MOVE;
                }
                if (is_in_board(y - 1, x - 1) && try_move(y, x, y - 1, x - 1)) {
                    predicted_moves[y - 1][x - 1] = PREDICTED_MOVE;
                }
                if (is_in_board(y - 1, x + 1) && try_move(y, x, y - 1, x + 1)) {
                    predicted_moves[y - 1][x + 1] = PREDICTED_MOVE;
                }
            } else {
                if (is_in_board(y + 2, x) && try_move(y, x, y + 2, x)) {
                    predicted_moves[y + 2][x] = PREDICTED_MOVE;
                }
                if (is_in_board(y + 1, x) && try_move(y, x, y + 1, x)) {
                    predicted_moves[y + 1][x] = PREDICTED_MOVE;
                }
                if (is_in_board(y + 1, x - 1) && try_move(y, x, y + 1, x - 1)) {
                    predicted_moves[y + 1][x - 1] = PREDICTED_MOVE;
                }
                if (is_in_board(y + 1, x + 1) && try_move(y, x, y + 1, x + 1)) {
                    predicted_moves[y + 1][x + 1] = PREDICTED_MOVE;
                }
            }
            break;
        }
        case KNIGHT: {
            int dy[] = {-2, -2, -1, -1,  1,  1,  2,  2};
            int dx[] = {-1,  1, -2,  2, -2,  2, -1,  1};
            for (int i = 0; i < 8; i++) {
                int ny = y + dy[i];
                int nx = x + dx[i];
                if (is_in_board(ny, nx) && try_move(y, x, ny, nx)) {
                    predicted_moves[ny][nx] = PREDICTED_MOVE;
                }
            }
            break;
        }
        case BISHOP: {
            int dy[] = {-1, -1,  1,  1};
            int dx[] = {-1,  1, -1,  1};
            for (int i = 0; i < 4; i++) {
                int ny = y + dy[i];
                int nx = x + dx[i];
                while (is_in_board(ny, nx)) {
                    if (try_move(y, x, ny, nx)) {
                        predicted_moves[ny][nx] = PREDICTED_MOVE;
                    }
                    if (board[ny][nx] != EMPTY) {
                        break;
                    }
                    ny += dy[i];
                    nx += dx[i];
                }
            }
            break;
        }
        case ROOK: {
            int dy[] = {-1,  0,  1,  0};
            int dx[] = { 0,  1,  0, -1};
            for (int i = 0; i < 4; i++) {
                int ny = y + dy[i];
                int nx = x + dx[i];
                while (is_in_board(ny, nx)) {
                    if (try_move(y, x, ny, nx)) {
                        predicted_moves[ny][nx] = PREDICTED_MOVE;
                    }
                    if (board[ny][nx] != EMPTY) {
                        break;
                    }
                    ny += dy[i];
                    nx += dx[i];
                }
            }
            break;
        }
        case QUEEN: {
            int dy[] = {-1, -1,  0,  1,  1,  1,  0, -1};
            int dx[] = { 0,  1,  1,  1,  0, -1, -1, -1};
            for (int i = 0; i < 8; i++) {
                int ny = y + dy[i];
                int nx = x + dx[i];
                while (is_in_board(ny, nx)) {
                    if (try_move(y, x, ny, nx)) {
                        predicted_moves[ny][nx] = PREDICTED_MOVE;
                    }
                    if (board[ny][nx] != EMPTY) {
                        break;
                    }
                    ny += dy[i];
                    nx += dx[i];
                }
            }
            break;
        }
        case KING: {
            int dy[] = {-1, -1,  0,  1,  1,  1,  0, -1};
            int dx[] = { 0,  1,  1,  1,  0, -1, -1, -1};
            for (int i = 0; i < 8; i++) {
                int ny = y + dy[i];
                int nx = x + dx[i];
                if (is_in_board(ny, nx) && try_move(y, x, ny, nx)) {
                    predicted_moves[ny][nx] = PREDICTED_MOVE;
                }
            }
            break;
        }
    }
}

bool is_legal_move(int from_y, int from_x, int to_y, int to_x) {
    int piece_val = board[from_y][from_x];
    switch (std::abs(piece_val)) {
        case PAWN: {
            // 兵只能前进一步，不能后退
            // 如果小兵的左上和右上有棋子，则可以吃对方棋子
            if (piece_val > 0) {
                // 初始位置可以前进两步
                if (from_y == 6 && to_y == 4 && from_x == to_x && board[from_y - 1][from_x] == EMPTY && board[to_y][to_x] == EMPTY) {
                    return true;
                }
                if (from_y > 0 && to_y == from_y - 1 && from_x == to_x && board[to_y][to_x] == EMPTY) {
                    return true;
                }
                if (from_x > 0 && to_y == from_y - 1 && to_x == from_x - 1 && board[to_y][to_x] < 0) {
                    return true;
                }
                if (from_x < 7 && to_y == from_y - 1 && to_x == from_x + 1 && board[to_y][to_x] < 0) {
                    return true;
                }
            } else {
                if (from_y == 1 && to_y == 3 && from_x == to_x && board[from_y + 1][from_x] == EMPTY && board[to_y][to_x] == EMPTY) {
                    return true;
                }
                if (from_y < 7 && to_y == from_y + 1 && from_x == to_x && board[to_y][to_x] == EMPTY) {
                    return true;
                }
                if (from_x > 0 && to_y == from_y + 1 && to_x == from_x - 1 && board[to_y][to_x] > 0) {
                    return true;
                }
                if (from_x < 7 && to_y == from_y + 1 && to_x == from_x + 1 && board[to_y][to_x] > 0) {
                    return true;
                }
            }
            break;
        }
        case KNIGHT: {
            int dy[] = {-2, -2, -1, -1,  1,  1,  2,  2};
            int dx[] = {-1,  1, -2,  2, -2,  2, -1,  1};
            for (int i = 0; i < 8; i++) {
                int ny = from_y + dy[i];
                int nx = from_x + dx[i];
                if (is_in_board(ny, nx) && ny == to_y && nx == to_x && piece_val * board[ny][nx] <= 0) {
                    return true;
                }
            }
            break;
        }
        case BISHOP: {
            int dy[] = {-1, -1,  1,  1};
            int dx[] = {-1,  1, -1,  1};
            for (int i = 0; i < 4; i++) {
                int ny = from_y + dy[i];
                int nx = from_x + dx[i];
                while (is_in_board(ny, nx) && piece_val * board[ny][nx] <= 0) {
                    if (ny == to_y && nx == to_x) {
                        return true;
                    }
                    if (board[ny][nx] != EMPTY) {
                        break;
                    }
                    ny += dy[i];
                    nx += dx[i];
                }
            }
            break;
        }
        case ROOK: {
            int dy[] = {-1,  0,  1,  0};
            int dx[] = { 0,  1,  0, -1};
            for (int i = 0; i < 4; i++) {
                int ny = from_y + dy[i];
                int nx = from_x + dx[i];
                while (is_in_board(ny, nx) && piece_val * board[ny][nx] <= 0) {
                    if (ny == to_y && nx == to_x) {
                        return true;
                    }
                    if (board[ny][nx] != EMPTY) {
                        break;
                    }
                    ny += dy[i];
                    nx += dx[i];
                }
            }
            break;
        }
        case QUEEN: {
            int dy[] = {-1, -1,  0,  1,  1,  1,  0, -1};
            int dx[] = { 0,  1,  1,  1,  0, -1, -1, -1};
            for (int i = 0; i < 8; i++) {
                int ny = from_y + dy[i];
                int nx = from_x + dx[i];
                while (is_in_board(ny, nx) && piece_val * board[ny][nx] <= 0) {
                    if (ny == to_y && nx == to_x) {
                        return true;
                    }
                    if (board[ny][nx] != EMPTY) {
                        break;
                    }
                    ny += dy[i];
                    nx += dx[i];
                }
            }
            break;
        }
        case KING: {
            int dy[] = {-1, -1,  0,  1,  1,  1,  0, -1};
            int dx[] = { 0,  1,  1,  1,  0, -1, -1, -1};
            for (int i = 0; i < 8; i++) {
                int ny = from_y + dy[i];
                int nx = from_x + dx[i];
                if (is_in_board(ny, nx) && ny == to_y && nx == to_x && piece_val * board[ny][nx] <= 0) {
                    return true;
                }
            }
            break;
        }
    }
    return false;
}

bool is_attacked(int ty, int tx, int attacker_side) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            int piece = board[i][j];
            // 如果这个格子有对方的棋子
            if (piece != 0 && (piece * attacker_side > 0)) {
                // 检查该棋子是否能合法移动到 (ty, tx)
                if (is_legal_move(i, j, ty, tx)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool is_in_check(int side) {
    int king_y = -1, king_x = -1;
    int target_king = (side == 1) ? 6 : -6; // 寻找对应的国王

    // 1. 找到国王的位置
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] == target_king) {
                king_y = i;
                king_x = j;
                break;
            }
        }
        if (king_y != -1) break;
    }

    // 2. 检查国王位置是否被对方攻击
    // 如果 side 是 1 (白), 攻击方就是 -1 (黑)
    return is_attacked(king_y, king_x, -side);
}

bool try_move(int sy, int sx, int dy, int dx) {
    // 1. 基本合法性检查
    if (!is_legal_move(sy, sx, dy, dx)) return false;

    int piece = board[sy][sx];

    // 2. 模拟移动（备份）
    int temp_target = board[dy][dx];
    int temp_source = board[sy][sx];
    board[dy][dx] = temp_source;
    board[sy][sx] = 0;

    // 3. 检查自己是否被将军
    // 如果 side 是 1 (白), 攻击方就是 -1 (黑)
    bool self_check = is_in_check(piece > 0 ? 1 : -1);

    // 4. 还原棋盘
    board[sy][sx] = temp_source;
    board[dy][dx] = temp_target;

    // 如果会导致自己被将军，则移动无效
    return !self_check;
}

bool is_checkmate(int side) {
    // 1. 如果当前没有被将军，那肯定不是将死
    if (!is_in_check(side)) {
        return false;
    }

    // 2. 遍历棋盘上所有属于 side 方的棋子
    for (int sy = 0; sy < 8; sy++) {
        for (int sx = 0; sx < 8; sx++) {
            if (board[sy][sx] * side > 0) { 
                
                // 3. 尝试这个棋子可以移动到的所有目标位置
                for (int dy = 0; dy < 8; dy++) {
                    for (int dx = 0; dx < 8; dx++) {
                        // 跳过自己的位置
                        if (dy == sy && dx == sx) continue;
                        
                        // 4. 使用之前提到的 try_move 逻辑
                        // try_move 内部会模拟移动并检查是否依然被将军
                        if (try_move(sy, sx, dy, dx)) {
                            // 只要找到任何一个移动能解除将军，就不是将死
                            return false; 
                        }
                    }
                }
            }
        }
    }

    // 5. 找遍了所有棋子的所有走法都无法解围，判定为将死
    return true;
}