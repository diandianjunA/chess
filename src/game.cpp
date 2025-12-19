#include "game.h"
#include "piece.h"
#include <ncurses.h>
#include "ai_player.h"

#define BOARD_SIZE 8
#define CELL_WIDTH 6
#define CELL_HEIGHT 3

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

            // 高亮逻辑（光标选中）
            if (i == cur_y && j == cur_x) {
                attron(COLOR_PAIR(3));
                background_color = COLOR_YELLOW;
            };
            if (predicted_moves[i][j] != 0) {
                attron(COLOR_PAIR(4));
                background_color = COLOR_CYAN;
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

        // 溢出检查
        if (max_y < TOTAL_H + 2 || max_x < TOTAL_W + 2) {
            mvprintw(max_y / 2, (max_x - 18) / 2, "Window too small!");
            refresh();
            if (getch() == 'q') break;
            continue;
        }

        int start_y = (max_y - TOTAL_H) / 2;
        int start_x = (max_x - TOTAL_W) / 2 + 2; // +2 是为了给左侧坐标轴留位置

        draw_ui(start_y, start_x, cur_y, cur_x);
        draw_dashboard(start_y, start_x, current_round % 2 == 1 ? 1 : -1, current_round);
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
                if (event.bstate & BUTTON1_CLICKED) {
                    // 坐标转换逻辑
                    int mouse_grid_y = (event.y - start_y) / CELL_HEIGHT;
                    int mouse_grid_x = (event.x - start_x) / CELL_WIDTH;

                    // 检查点击是否在棋盘范围内
                    if (mouse_grid_y >= 0 && mouse_grid_y < 8 && 
                        mouse_grid_x >= 0 && mouse_grid_x < 8) {
                        cur_y = mouse_grid_y;
                        cur_x = mouse_grid_x;
                        
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

        // 溢出检查
        if (max_y < TOTAL_H + 2 || max_x < TOTAL_W + 2) {
            mvprintw(max_y / 2, (max_x - 18) / 2, "Window too small!");
            refresh();
            if (getch() == 'q') break;
            continue;
        }

        int start_y = (max_y - TOTAL_H) / 2;
        int start_x = (max_x - TOTAL_W) / 2 + 2; // +2 是为了给左侧坐标轴留位置

        draw_ui(start_y, start_x, cur_y, cur_x);
        draw_dashboard(start_y, start_x, current_round % 2 == 1 ? 1 : -1, current_round);
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
                    if (event.bstate & BUTTON1_CLICKED) {
                        // 坐标转换逻辑
                        int mouse_grid_y = (event.y - start_y) / CELL_HEIGHT;
                        int mouse_grid_x = (event.x - start_x) / CELL_WIDTH;

                        // 检查点击是否在棋盘范围内
                        if (mouse_grid_y >= 0 && mouse_grid_y < 8 && 
                            mouse_grid_x >= 0 && mouse_grid_x < 8) {
                            cur_y = mouse_grid_y;
                            cur_x = mouse_grid_x;
                            
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
                }
            }
        } else {
            mvprintw(start_y, start_x, "AI Thinking...");
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
    int dx = start_x + (8 * CELL_WIDTH) + 6;
    int width = 36; 
    int current_y = start_y; // 动态坐标指针

    // 1. 整体背景刷色 (确保高度覆盖所有区域)
    attron(COLOR_PAIR(31));
    for (int h = 0; h < 24; h++) {
        mvhline(start_y + h, dx, ' ', width);
    }

    // 2. 标题区
    mvprintw(current_y++, dx, "+----------------------------------+");
    mvprintw(current_y++, dx, "|         CHESS DASHBOARD          |");
    mvprintw(current_y++, dx, "+----------------------------------+");
    current_y++; // 留空一行

    // 3. 状态区
    mvprintw(current_y++, dx + 2, "ROUND: %-3d  TURN: %s", round, (turn == 1 ? "WHITE" : "BLACK"));
    
    // 优势条
    mvprintw(current_y++, dx + 2, "ADV: ");
    int w_val = calculate_score(1);
    int b_val = calculate_score(-1);
    int bar_len = 20;
    int w_bar = (w_val * bar_len) / (w_val + b_val + 1);
    attron(A_DIM); // 稍微暗一点
    for(int i=0; i<bar_len; i++) addch(i < w_bar ? '#' : '-');
    attroff(A_DIM);
    current_y += 2; // 留空

    // 4. 白方吃子池 (显示黑棋子)
    mvprintw(current_y++, dx + 2, "White's Trophies:");
    attron(COLOR_PAIR(32) | A_BOLD);
    for (int i = 0; i < white_cap_count; i++) {
        mvprintw(current_y + (i/10), dx + 2 + (i%10)*3, "%s", get_piece_letter(white_captured[i]));
    }
    attroff(COLOR_PAIR(32) | A_BOLD);
    current_y += 3; // 固定占用 3 行空间

    // 5. 黑方吃子池 (显示白棋子)
    attron(COLOR_PAIR(31));
    mvprintw(current_y++, dx + 2, "Black's Trophies:");
    attron(COLOR_PAIR(33) | A_BOLD);
    for (int i = 0; i < black_cap_count; i++) {
        mvprintw(current_y + (i/10), dx + 2 + (i%10)*3, "%s", get_piece_letter(black_captured[i]));
    }
    attroff(COLOR_PAIR(33) | A_BOLD);
    current_y += 3;

    // 6. 警示区
    if (is_in_check(turn)) {
        attron(COLOR_PAIR(34) | A_BOLD | A_BLINK);
        mvprintw(current_y, dx + 4, " !!!  UNDER CHECK  !!! ");
        attroff(COLOR_PAIR(34) | A_BOLD | A_BLINK);
    }
    current_y += 2;

    // 7. 底部快捷键
    attron(COLOR_PAIR(31));
    mvprintw(current_y++, dx, "+----------------------------------+");
    mvprintw(current_y++, dx + 2, "U:Undo  S:Save  Q:Exit  R:Resign");
    mvprintw(current_y++, dx, "+----------------------------------+");

    attroff(COLOR_PAIR(31));
}