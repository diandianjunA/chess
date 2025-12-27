#include "game.h"
#include "piece.h"
#include <ncurses.h>
#include "ai_player.h"

#define BOARD_SIZE 8
#define CELL_WIDTH 4
#define CELL_HEIGHT 2

// 总宽度：棋盘(8*6)
#define TOTAL_W (BOARD_SIZE * CELL_WIDTH)
// 总高度：棋盘(8*3) + 底部坐标轴空间(2)
#define TOTAL_H (BOARD_SIZE * CELL_HEIGHT)

void Game::draw_ui(int start_y, int start_x, int cur_y, int cur_x) {
    // 绘制坐标轴
    for (int i = 0; i < BOARD_SIZE; i++) {
        mvprintw(start_y + i * CELL_HEIGHT + CELL_HEIGHT/2, start_x - 2, "%d", 8 - i);
    }
    // 绘制字母坐标轴
    for (int j = 0; j < BOARD_SIZE; j++) {
        mvprintw(start_y + BOARD_SIZE * CELL_HEIGHT, start_x + j * CELL_WIDTH + CELL_WIDTH/2, "%c", 'A' + j);
    }

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            int py = start_y + (i * CELL_HEIGHT);
            int px = start_x + (j * CELL_WIDTH);

            int background_color = 0;
            // 颜色逻辑
            if ( (i + j) % 2) {
                attron(COLOR_PAIR(1));
                background_color = COLOR_WHITE;
            } else  {
                attron(COLOR_PAIR(2));
                background_color = COLOR_BLACK;
            }

            if (predicted_moves[i][j] != 0) {
                attron(COLOR_PAIR(4));
                background_color = COLOR_CYAN;
            };
            // 高亮逻辑（光标选中）
            if (i == cur_y && j == cur_x) {
                attron(COLOR_PAIR(3));
                background_color = COLOR_YELLOW;
            };
            if (board[i][j] != EMPTY && board[i][j] * selected_piece < 0 && predicted_moves[i][j] != 0) {
                attron(COLOR_PAIR(5));
                background_color = COLOR_RED;
            };
            if (board[i][j] == KING && white_in_check) {
                attron(COLOR_PAIR(5));
                background_color = COLOR_RED;
            }
            if (board[i][j] == -KING && black_in_check) {
                attron(COLOR_PAIR(5));
                background_color = COLOR_RED;
            }

            // 填充格子
            for (int h = 0; h < CELL_HEIGHT; h++) {
                mvhline(py + h, px, ' ', CELL_WIDTH);
            }

            // 绘制棋子
            int piece = board[i][j];
            if (piece != EMPTY) {
                if (piece > 0) {
                    switch (background_color) {
                        case COLOR_WHITE:
                            attron(COLOR_PAIR(11));
                            break;
                        case COLOR_BLACK:
                            attron(COLOR_PAIR(12));
                            break;
                        case COLOR_YELLOW:
                            attron(COLOR_PAIR(13));
                            break;
                        case COLOR_CYAN:
                            attron(COLOR_PAIR(14));
                            break;
                        case COLOR_RED:
                            attron(COLOR_PAIR(15));
                            break;
                    }
                } else {
                    switch (background_color) {
                        case COLOR_WHITE:
                            attron(COLOR_PAIR(21));
                            break;
                        case COLOR_BLACK:
                            attron(COLOR_PAIR(22));
                            break;
                        case COLOR_YELLOW:
                            attron(COLOR_PAIR(23));
                            break;
                        case COLOR_CYAN:
                            attron(COLOR_PAIR(24));
                            break;
                        case COLOR_RED:
                            attron(COLOR_PAIR(25));
                            break;
                    }
                }
                // 居中打印字母
                mvprintw(py + CELL_HEIGHT / 2, px + (CELL_WIDTH - 2) / 2, "%s", get_piece_letter(piece));
                
                attroff(COLOR_PAIR(11)); attroff(COLOR_PAIR(12)); attroff(COLOR_PAIR(13)); attroff(COLOR_PAIR(14)); attroff(COLOR_PAIR(15));
                attroff(COLOR_PAIR(21)); attroff(COLOR_PAIR(22)); attroff(COLOR_PAIR(23)); attroff(COLOR_PAIR(24)); attroff(COLOR_PAIR(25));
            }

            // 关闭颜色属性
            attroff(COLOR_PAIR(1)); attroff(COLOR_PAIR(2)); attroff(COLOR_PAIR(3)); attroff(COLOR_PAIR(4)); attroff(COLOR_PAIR(5));
        }
    }
}

void Game::double_mode_start() {
    int cur_y = 0, cur_x = 0;
    int max_y, max_x;

    while (1) {
        getmaxyx(stdscr, max_y, max_x);
        clear();

        int start_y = 2;
        int start_x = 4; // +2 是为了给左侧坐标轴留位置

        int dash_y = start_y;
        int dash_x = start_x + 32 + 2; 

        draw_ui(start_y, start_x, cur_y, cur_x);
        draw_dashboard(dash_y, dash_x, current_round % 2 == 1 ? 1 : -1, current_round);
        refresh();

        // 检查是否游戏结束
        if (white_in_checkmate) {
            mvprintw(max_y / 2, (max_x - 18) / 2, "Checkmate! Black wins! Press q or Q to exit.");
            refresh();
            getch();
            break;
        }

        if (black_in_checkmate) {
            mvprintw(max_y / 2, (max_x - 18) / 2, "Checkmate! White wins! Press q or Q to exit.");
            refresh();
            getch();
            break;
        }

        int ch = getch();
        if (ch == 'q' || ch == 'Q') {
            return;
        };
        // 处理鼠标点击
        if (ch == KEY_MOUSE) {
            MEVENT event;
            if (getmouse(&event) == OK) {
                if (event.bstate & (BUTTON1_CLICKED | BUTTON1_PRESSED)) {
                    // 坐标转换逻辑
                    int mouse_grid_y = (event.y - start_y) / CELL_HEIGHT;
                    int mouse_grid_x = (event.x - start_x) / CELL_WIDTH;

                    // 检查点击是否在棋盘范围内
                    if (mouse_grid_y >= 0 && mouse_grid_y < 8 && 
                        mouse_grid_x >= 0 && mouse_grid_x < 8) {
                        cur_y = mouse_grid_y;
                        cur_x = mouse_grid_x;
                    }

                    if (selected_piece == 0) {
                        selected_piece = board[cur_y][cur_x];
                        selected_x = cur_x;
                        selected_y = cur_y;
                        if (selected_piece != EMPTY && selected_piece * (current_round % 2 == 1 ? 1 : -1) > 0) {
                            predicted_moves = {
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            };
                            predict_move(cur_y, cur_x, predicted_moves);
                        }
                    } else {
                        int target_piece = board[cur_y][cur_x];
                        if (predicted_moves[cur_y][cur_x] != 0 && (target_piece * (current_round % 2 == 1 ? 1 : -1) < 0 || target_piece == 0)) {
                            // 捕获棋子
                            if (target_piece * (current_round % 2 == 1 ? 1 : -1) < 0) {
                                if (current_round % 2 == 1) {
                                    black_captured[black_cap_count++] = target_piece;
                                } else {
                                    white_captured[white_cap_count++] = target_piece;
                                }
                            }

                            board[cur_y][cur_x] = selected_piece;
                            board[selected_y][selected_x] = EMPTY;

                            selected_piece = 0;
                            selected_x = 0;
                            selected_y = 0;
                            current_round++;

                            predicted_moves = {
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                            };

                            // 检查是否将军
                            if (is_in_check(1)) {
                                if (is_checkmate(1)) {
                                    white_in_checkmate = true;
                                } else {
                                    white_in_check = true;
                                }
                            } else {
                                white_in_check = false;
                            }

                            if (is_in_check(-1)) {
                                if (is_checkmate(-1)) {
                                    black_in_checkmate = true;
                                } else {
                                    black_in_check = true;
                                }
                            } else {
                                black_in_check = false;
                            }
                        } else {
                            selected_piece = board[cur_y][cur_x];
                            selected_x = cur_x;
                            selected_y = cur_y;
                            predicted_moves = {
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            };
                            if (selected_piece != EMPTY && selected_piece * (current_round % 2 == 1 ? 1 : -1) > 0) {
                                predict_move(cur_y, cur_x, predicted_moves);
                            }
                        }
                    }
                }
            }
        } else if (ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_DOWN || ch == '\n') { //处理键盘方向键
            if (ch == KEY_LEFT) {
                if (cur_x > 0) {
                    cur_x--;
                }
            } else if (ch == KEY_RIGHT) {
                if (cur_x < 7) {
                    cur_x++;
                }
            } else if (ch == KEY_UP) {
                if (cur_y > 0) {
                    cur_y--;
                }
            } else if (ch == KEY_DOWN) {
                if (cur_y < 7) {
                    cur_y++;
                }
            }
            if (!choose) {
                selected_piece = board[cur_y][cur_x];
                selected_x = cur_x;
                selected_y = cur_y;
                if (selected_piece != EMPTY && selected_piece * (current_round % 2 == 1 ? 1 : -1) > 0) {
                    predicted_moves = {
                    {0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0},
                    {0, 0, 0, 0, 0, 0, 0, 0},
                    };
                    predict_move(cur_y, cur_x, predicted_moves);
                }
            }
            if (ch == '\n') { //处理键盘确认键
                if (choose) {
                    int target_piece = board[cur_y][cur_x];
                    if (predicted_moves[cur_y][cur_x] != 0 && (target_piece * (current_round % 2 == 1 ? 1 : -1) < 0 || target_piece == 0)) {
                        // 捕获棋子
                        if (target_piece * (current_round % 2 == 1 ? 1 : -1) < 0) {
                            if (current_round % 2 == 1) {
                                black_captured[black_cap_count++] = target_piece;
                            } else {
                                white_captured[white_cap_count++] = target_piece;
                            }
                        }

                        board[cur_y][cur_x] = selected_piece;
                        board[selected_y][selected_x] = EMPTY;

                        selected_piece = 0;
                        selected_x = 0;
                        selected_y = 0;
                        choose = false;
                        current_round++;

                        predicted_moves = {
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                        };

                        // 检查是否将军
                        if (is_in_check(1)) {
                            if (is_checkmate(1)) {
                                white_in_checkmate = true;
                            } else {
                                white_in_check = true;
                            }
                        } else {
                            white_in_check = false;
                        }

                        if (is_in_check(-1)) {
                            if (is_checkmate(-1)) {
                                black_in_checkmate = true;
                            } else {
                                black_in_check = true;
                            }
                        } else {
                            black_in_check = false;
                        }
                    } else {
                        selected_piece = board[cur_y][cur_x];
                        selected_x = cur_x;
                        selected_y = cur_y;
                        predicted_moves = {
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        };
                        if (selected_piece != EMPTY && selected_piece * (current_round % 2 == 1 ? 1 : -1) > 0) {
                            predict_move(cur_y, cur_x, predicted_moves);
                        }
                    }
                } else {
                    choose = true;
                    selected_piece = board[cur_y][cur_x];
                    selected_x = cur_x;
                    selected_y = cur_y;
                }
            }
        }
    }

    // 按q或Q退出游戏
    while (true) {
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;
    }
}

void Game::single_mode_start() {
    int cur_y = 0, cur_x = 0;
    int max_y, max_x;

    AIPlayer ai_player;

    while (1) {
        getmaxyx(stdscr, max_y, max_x);
        clear();

        int start_y = 2;
        int start_x = 4; // +2 是为了给左侧坐标轴留位置

        int dash_y = start_y;
        int dash_x = start_x + 32 + 2; 

        draw_ui(start_y, start_x, cur_y, cur_x);
        draw_dashboard(dash_y, dash_x, current_round % 2 == 1 ? 1 : -1, current_round);
        refresh();

        // 检查是否游戏结束
        if (white_in_checkmate) {
            mvprintw(max_y / 2, (max_x - 18) / 2, "Checkmate! Black wins! Press q or Q to exit.");
            refresh();
            getch();
            break;
        }

        if (black_in_checkmate) {
            mvprintw(max_y / 2, (max_x - 18) / 2, "Checkmate! White wins! Press q or Q to exit.");
            refresh();
            getch();
            break;
        }

        if (current_round % 2 == 1) {
            int ch = getch();
            if (ch == 'q' || ch == 'Q') {
                return;
            };
            // 处理鼠标点击
            if (ch == KEY_MOUSE) {
                MEVENT event;
                if (getmouse(&event) == OK) {
                    if (event.bstate & (BUTTON1_CLICKED | BUTTON1_PRESSED)) {
                        // 坐标转换逻辑
                        int mouse_grid_y = (event.y - start_y) / CELL_HEIGHT;
                        int mouse_grid_x = (event.x - start_x) / CELL_WIDTH;

                        // 检查点击是否在棋盘范围内
                        if (mouse_grid_y >= 0 && mouse_grid_y < 8 && 
                            mouse_grid_x >= 0 && mouse_grid_x < 8) {
                            cur_y = mouse_grid_y;
                            cur_x = mouse_grid_x;
                        }

                        if (selected_piece == 0) {
                            selected_piece = board[cur_y][cur_x];
                            selected_x = cur_x;
                            selected_y = cur_y;
                            if (selected_piece != EMPTY && selected_piece * (current_round % 2 == 1 ? 1 : -1) > 0) {
                                predicted_moves = {
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                };
                                predict_move(cur_y, cur_x, predicted_moves);
                            }
                        } else {
                            int target_piece = board[cur_y][cur_x];
                            if (predicted_moves[cur_y][cur_x] != 0 && (target_piece * (current_round % 2 == 1 ? 1 : -1) < 0 || target_piece == 0)) {
                                // 捕获棋子
                                if (target_piece * (current_round % 2 == 1 ? 1 : -1) < 0) {
                                    if (current_round % 2 == 1) {
                                        black_captured[black_cap_count++] = target_piece;
                                    } else {
                                        white_captured[white_cap_count++] = target_piece;
                                    }
                                }

                                board[cur_y][cur_x] = selected_piece;
                                board[selected_y][selected_x] = EMPTY;

                                selected_piece = 0;
                                selected_x = 0;
                                selected_y = 0;
                                current_round++;

                                predicted_moves = {
                                    {0, 0, 0, 0, 0, 0, 0, 0},
                                    {0, 0, 0, 0, 0, 0, 0, 0},
                                    {0, 0, 0, 0, 0, 0, 0, 0},
                                    {0, 0, 0, 0, 0, 0, 0, 0},
                                    {0, 0, 0, 0, 0, 0, 0, 0},
                                    {0, 0, 0, 0, 0, 0, 0, 0},
                                    {0, 0, 0, 0, 0, 0, 0, 0},
                                    {0, 0, 0, 0, 0, 0, 0, 0},
                                };

                                // 检查是否将军
                                if (is_in_check(1)) {
                                    if (is_checkmate(1)) {
                                        white_in_checkmate = true;
                                    } else {
                                        white_in_check = true;
                                    }
                                } else {
                                    white_in_check = false;
                                }

                                if (is_in_check(-1)) {
                                    if (is_checkmate(-1)) {
                                        black_in_checkmate = true;
                                    } else {
                                        black_in_check = true;
                                    }
                                } else {
                                    black_in_check = false;
                                }
                            } else {
                                selected_piece = board[cur_y][cur_x];
                                selected_x = cur_x;
                                selected_y = cur_y;
                                predicted_moves = {
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                };
                                if (selected_piece != EMPTY && selected_piece * (current_round % 2 == 1 ? 1 : -1) > 0) {
                                    predict_move(cur_y, cur_x, predicted_moves);
                                }
                            }
                        }
                    }
                }
            } else if (ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_UP || ch == KEY_DOWN || ch == '\n') { //处理键盘方向键
                if (ch == KEY_LEFT) {
                    if (cur_x > 0) {
                        cur_x--;
                    }
                } else if (ch == KEY_RIGHT) {
                    if (cur_x < 7) {
                        cur_x++;
                    }
                } else if (ch == KEY_UP) {
                    if (cur_y > 0) {
                        cur_y--;
                    }
                } else if (ch == KEY_DOWN) {
                    if (cur_y < 7) {
                        cur_y++;
                    }
                }
                if (!choose) {
                    selected_piece = board[cur_y][cur_x];
                    selected_x = cur_x;
                    selected_y = cur_y;
                    if (selected_piece != EMPTY && selected_piece * (current_round % 2 == 1 ? 1 : -1) > 0) {
                        predicted_moves = {
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        {0, 0, 0, 0, 0, 0, 0, 0},
                        };
                        predict_move(cur_y, cur_x, predicted_moves);
                    }
                }
                if (ch == '\n') { //处理键盘确认键
                    if (choose) {
                        int target_piece = board[cur_y][cur_x];
                        if (predicted_moves[cur_y][cur_x] != 0 && (target_piece * (current_round % 2 == 1 ? 1 : -1) < 0 || target_piece == 0)) {
                            // 捕获棋子
                            if (target_piece * (current_round % 2 == 1 ? 1 : -1) < 0) {
                                if (current_round % 2 == 1) {
                                    black_captured[black_cap_count++] = target_piece;
                                } else {
                                    white_captured[white_cap_count++] = target_piece;
                                }
                            }

                            board[cur_y][cur_x] = selected_piece;
                            board[selected_y][selected_x] = EMPTY;

                            selected_piece = 0;
                            selected_x = 0;
                            selected_y = 0;
                            choose = false;
                            current_round++;

                            predicted_moves = {
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                                {0, 0, 0, 0, 0, 0, 0, 0},
                            };

                            // 检查是否将军
                            if (is_in_check(1)) {
                                if (is_checkmate(1)) {
                                    white_in_checkmate = true;
                                } else {
                                    white_in_check = true;
                                }
                            } else {
                                white_in_check = false;
                            }

                            if (is_in_check(-1)) {
                                if (is_checkmate(-1)) {
                                    black_in_checkmate = true;
                                } else {
                                    black_in_check = true;
                                }
                            } else {
                                black_in_check = false;
                            }
                        } else {
                            selected_piece = board[cur_y][cur_x];
                            selected_x = cur_x;
                            selected_y = cur_y;
                            predicted_moves = {
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            {0, 0, 0, 0, 0, 0, 0, 0},
                            };
                            if (selected_piece != EMPTY && selected_piece * (current_round % 2 == 1 ? 1 : -1) > 0) {
                                predict_move(cur_y, cur_x, predicted_moves);
                            }
                        }
                    } else {
                        choose = true;
                        selected_piece = board[cur_y][cur_x];
                        selected_x = cur_x;
                        selected_y = cur_y;
                    }
                }
            }
        } else {
            mvprintw(0, 0, "AI Thinking...");
            int captured_piece = ai_player.make_move();
            if (captured_piece != 0) {
                if (captured_piece) {
                    if (current_round % 2 == 1) {
                        black_captured[black_cap_count++] = captured_piece;
                    } else {
                        white_captured[white_cap_count++] = captured_piece;
                    }
                }
            }
            
            current_round++;
            // 检查是否将军
            if (is_in_check(1)) {
                if (is_checkmate(1)) {
                    white_in_checkmate = true;
                } else {
                    white_in_check = true;
                }
            } else {
                white_in_check = false;
            }

            if (is_in_check(-1)) {
                if (is_checkmate(-1)) {
                    black_in_checkmate = true;
                } else {
                    black_in_check = true;
                }
            } else {
                black_in_check = false;
            }

            mvprintw(0, 0, "AI Waiting...");
        }
    }

    // 按q或Q退出游戏
    while (true) {
        int ch = getch();
        if (ch == 'q' || ch == 'Q') break;
    }
}

int Game::calculate_score(int side) {
    int total = 0;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (board[i][j] * side > 0) total += get_piece_value(board[i][j]);
        }
    }
    return total;
}

void Game::draw_dashboard(int start_y, int start_x, int turn, int round) {
    int width = 24; // 压缩宽度
    int current_y = start_y;

    // 1. 刷背景
    attron(COLOR_PAIR(31));
    for (int h = 0; h < 12; h++) {
        mvhline(start_y + h, start_x, ' ', width);
    }

    // 2. 简易标题
    mvprintw(current_y++, start_x, "--- DASHBOARD ---");
    
    // 3. 状态信息 (一行显示)
    mvprintw(current_y++, start_x, "R:%d | T:%s", round, (turn == 1 ? "W" : "B"));
    current_y++;

    // 4. 战力条 (缩短)
    int w_val = calculate_score(1);
    int b_val = calculate_score(-1);
    int bar_len = 16;
    int w_bar = (w_val * bar_len) / (w_val + b_val + 1);
    mvprintw(current_y++, start_x, "ADV:[");
    for(int i=0; i<bar_len; i++) addch(i < w_bar ? '#' : '-');
    printw("]");
    current_y += 1;

    // 5. 吃子池 (每行 6 个，更紧凑)
    mvprintw(current_y++, start_x, "W trophies:");
    attron(COLOR_PAIR(32) | A_BOLD);
    for (int i = 0; i < white_cap_count; i++) {
        mvprintw(current_y + (i/6), start_x + (i%6)*3, "%s", get_piece_letter(white_captured[i]));
    }
    attroff(COLOR_PAIR(32) | A_BOLD);
    current_y += 1;

    attron(COLOR_PAIR(31));
    mvprintw(current_y++, start_x, "B trophies:");
    attron(COLOR_PAIR(33) | A_BOLD);
    for (int i = 0; i < black_cap_count; i++) {
        mvprintw(current_y + (i/6), start_x + (i%6)*3, "%s", get_piece_letter(black_captured[i]));
    }
    attroff(COLOR_PAIR(33) | A_BOLD);
    current_y += 1;

    if (is_in_check(turn)) {
        attron(COLOR_PAIR(34) | A_BOLD | A_BLINK);
        mvprintw(current_y, start_x + 4, " !!!  UNDER CHECK  !!! ");
        attroff(COLOR_PAIR(34) | A_BOLD | A_BLINK);
    }
    current_y += 1;

    // 7. 快捷键提示 (极简)
    mvprintw(current_y++, start_x, "U:Undo S:Save Q:Exit");
    attroff(COLOR_PAIR(31));
    attrset(A_NORMAL);
}

void Game::restart() {
    current_round = 1;
    white_in_check = false;
    black_in_check = false;
    white_in_checkmate = false;
    black_in_checkmate = false;
    white_cap_count = 0;
    black_cap_count = 0;
    selected_piece = 0;
    selected_x = 0;
    selected_y = 0;
    predicted_moves = std::vector<std::vector<int>>(8, std::vector<int>(8, 0));
    choose = false;

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            board[i][j] = initialBoard[i][j];
        }
    }
}