#include "ai_player.h"
#include "piece.h"

// 针对黑棋的位置评估表（值越高越好）
int pawn_table[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0}, // 黑方底线
    { 5, 10, 10,-20,-20, 10, 10,  5}, // 初始位置（适度惩罚中心兵阻挡出子）
    { 5, -5,-10,  0,  0,-10, -5,  5},
    { 0,  0,  0, 20, 20,  0,  0,  0}, // 占据中心
    { 5,  5, 10, 25, 25, 10,  5,  5}, // 推进
    {10, 10, 20, 30, 30, 20, 10, 10}, // 威胁
    {50, 50, 50, 50, 50, 50, 50, 50}, // 接近升变
    { 0,  0,  0,  0,  0,  0,  0,  0}  // 升变行
};

int knight_table[8][8] = {
    {-50,-40,-30,-30,-30,-30,-40,-50}, // 避开角落
    {-40,-20,  0,  0,  0,  0,-20,-40},
    {-30,  0, 10, 15, 15, 10,  0,-30},
    {-30,  5, 15, 20, 20, 15,  5,-30},
    {-30,  0, 15, 20, 20, 15,  0,-30},
    {-30,  5, 10, 15, 15, 10,  5,-30},
    {-40,-20,  0,  5,  5,  0,-20,-40},
    {-50,-40,-30,-30,-30,-30,-40,-50}
};

int bishop_table[8][8] = {
    {-20,-10,-10,-10,-10,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5, 10, 10,  5,  0,-10},
    {-10,  5,  5, 10, 10,  5,  5,-10},
    {-10,  0, 10, 10, 10, 10,  0,-10},
    {-10, 10, 10, 10, 10, 10, 10,-10},
    {-10,  5,  0,  0,  0,  0,  5,-10},
    {-20,-10,-10,-10,-10,-10,-10,-20}
};

int rook_table[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5}, // 占据对方二线
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}  // 初始位置，鼓励出车
};

int queen_table[8][8] = {
    {-20,-10,-10, -5, -5,-10,-10,-20},
    {-10,  0,  0,  0,  0,  0,  0,-10},
    {-10,  0,  5,  5,  5,  5,  0,-10},
    { -5,  0,  5,  5,  5,  5,  0, -5},
    {  0,  0,  5,  5,  5,  5,  0, -5},
    {-10,  5,  5,  5,  5,  5,  0,-10},
    {-10,  0,  5,  0,  0,  0,  0,-10},
    {-20,-10,-10, -5, -5,-10,-10,-20}
};

int king_table[8][8] = {
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-30,-40,-40,-50,-50,-40,-40,-30},
    {-20,-30,-30,-40,-40,-30,-30,-20},
    {-10,-20,-20,-20,-20,-20,-20,-10},
    { 20, 20,  0,  0,  0,  0, 20, 20}, // 王翼或后翼易位后的安全区
    { 20, 30, 10,  0,  0, 10, 30, 20}  // 初始底线安全位置
};

int get_positional_score(int piece, int r, int c) {
    int type = abs(piece);
    bool is_white = (piece > 0);
    
    // 如果是白方，需要反转行号来读取表格（因为表格是按黑方视角写的）
    int table_r = is_white ? (7 - r) : r;
    int table_c = c;

    switch(type) {
        case 1: return pawn_table[table_r][table_c];
        case 2: return knight_table[table_r][table_c];
        case 3: return bishop_table[table_r][table_c];
        case 4: return rook_table[table_r][table_c];
        case 5: return queen_table[table_r][table_c];
        case 6: return king_table[table_r][table_c];
        default: return 0;
    }
}

int AIPlayer::make_move() {
    Move best_move = {-1, -1, -1, -1, -20000};
    std::vector<Move> equal_best_moves; // 存储得分相同的最佳走法

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] < 0) { 
                int piece_type = abs(board[i][j]);
                
                for (int ty = 0; ty < 8; ty++) {
                    for (int tx = 0; tx < 8; tx++) {
                        if (try_move(i, j, ty, tx)) {
                            int current_score = 0;

                            // --- 1. 进攻得分 (吃子) ---
                            if (board[ty][tx] > 0) {
                                current_score += get_piece_value(board[ty][tx]) * 10; 
                            }

                            // --- 2. 位置权重得分 ---
                            int piece_type = abs(board[i][j]);
                            current_score += (get_positional_score(board[i][j], ty, tx) - 
                                            get_positional_score(board[i][j], i, j));

                            // --- 3. 防守逻辑：检测移动后的安全性 ---
                            // 模拟执行移动，判断移动后目标点是否会被白方吃掉
                            // 暂时修改棋盘进行检测
                            int temp_piece = board[ty][tx];
                            board[ty][tx] = board[i][j];
                            board[i][j] = 0;

                            if (is_attacked(ty, tx, 1)) { // 如果被白方攻击
                                // 惩罚分数 = 被吃掉的棋子价值
                                // 这样 AI 就会意识到：虽然能吃个兵，但丢个后是不划算的
                                current_score -= get_piece_value(board[ty][tx]) * 10;
                            }
                            
                            // 特殊防守：如果这个棋子在原位本来就在被攻击，现在逃跑了，应该给加分
                            // (这可以让 AI 学会“逃跑”)
                            if (is_attacked(i, j, 1)) { 
                                current_score += get_piece_value(board[ty][tx]) * 5; 
                            }

                            // 还原棋盘
                            board[i][j] = board[ty][tx];
                            board[ty][tx] = temp_piece;

                            // --- 4. 随机扰动 ---
                            current_score += rand() % 2;

                            if (current_score > best_move.score) {
                                best_move.score = current_score;
                                equal_best_moves.clear();
                                Move m = {i, j, ty, tx, current_score};
                                equal_best_moves.push_back(m);
                            } else if (current_score == best_move.score) {
                                Move m = {i, j, ty, tx, current_score};
                                equal_best_moves.push_back(m);
                            }
                        }
                    }
                }
            }
        }
    }

    // 从所有得分最高的走法中随机选一个，彻底解决“重复走法”
    if (!equal_best_moves.empty()) {
        int idx = rand() % equal_best_moves.size();
        return execute_move(equal_best_moves[idx]);
    }
    return 0;
}

int AIPlayer::execute_move(Move& move) {
    int piece = board[move.dy][move.dx];
    board[move.dy][move.dx] = board[move.sy][move.sx];
    board[move.sy][move.sx] = EMPTY;
    return piece;
}

