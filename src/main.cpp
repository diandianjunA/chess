#include "game.h"
#include <cstring>
#include <ncurses.h>
#include <locale.h>

typedef enum { MENU, SINGLE_PLAYER, DOUBLE_PLAYER, EXIT } GameState;

void draw_menu(int highlighted) {
    clear();
    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    // 定义你选定的标题
    const char* title[] = {
        "  _|_|_|  _|    _|  _|_|_|_|    _|_|_|    _|_|_|  ",
        "_|        _|    _|  _|        _|        _|        ",
        "_|        _|_|_|_|  _|_|_|      _|_|      _|_|    ",
        "_|        _|    _|  _|              _|        _|  ",
        "  _|_|_|  _|    _|  _|_|_|_|  _|_|_|    _|_|_|    ",
        "             [ - - - CHESS - - - ]                "
    };
    int title_rows = 6;
    
    // 关键点：使用第一行的长度作为基准进行居中
    int title_width = strlen(title[0]);
    int start_x = (cols - title_width) / 2;
    if (start_x < 0) start_x = 0; 

    // 绘制标题
    attron(A_BOLD | COLOR_PAIR(3));
    for (int i = 0; i < title_rows; i++) {
        mvprintw(rows/4 + i, start_x, "%s", title[i]);
    }
    attroff(A_BOLD | COLOR_PAIR(3));

    // --- 按钮部分 ---
    const char* options[] = {
        " [  Single Player Mode  ] ",
        " [  Double Player Mode  ] ",
        " [      Exit Game       ] "
    };

    for (int i = 0; i < 3; i++) {
        int opt_width = strlen(options[i]);
        int opt_x = (cols - opt_width) / 2;
        int opt_y = rows/2 + i * 2 + 1;

        if (i == highlighted) {
            attron(A_REVERSE | COLOR_PAIR(3) | A_BOLD);
            mvprintw(opt_y, opt_x, "%s", options[i]);
            attroff(A_REVERSE | COLOR_PAIR(3) | A_BOLD);
        } else {
            mvprintw(opt_y, opt_x, "%s", options[i]);
        }
    }

    // 底部提示语
    const char* hint = "Use arrow keys to navigate, Enter to select.";
    mvprintw(rows - 2, (cols - strlen(hint))/2, "%s", hint);

    refresh();
}

int main() {
    setlocale(LC_ALL, "");
    initscr();
    start_color();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);

    // 开启鼠标支持
    if (mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL)) {
        mvprintw(0, 0, "Mouse support enabled.");
    } else {
        mvprintw(0, 0, "Error: Terminal does not support mouse!");
        refresh();
        getch(); // 等待用户按键
        endwin();
        return 1;
    }

    init_pair(1, COLOR_BLACK, COLOR_WHITE);   // 浅色背景
    init_pair(2, COLOR_WHITE, COLOR_BLACK);   // 深色背景
    init_pair(3, COLOR_BLACK, COLOR_YELLOW);  // 选中背景
    init_pair(4, COLOR_BLACK, COLOR_CYAN); // 预测移动背景
    init_pair(5, COLOR_WHITE, COLOR_RED);     // 预计被吃背景

    // --- 白方棋子 (假设用亮白色 COLOR_BLUE) ---
    init_pair(11, COLOR_BLUE, COLOR_WHITE);   // 在浅色格
    init_pair(12, COLOR_BLUE, COLOR_BLACK);   // 在深色格
    init_pair(13, COLOR_BLUE, COLOR_YELLOW);  // 在选中格
    init_pair(14, COLOR_BLUE, COLOR_CYAN); // 在预测格
    init_pair(15, COLOR_BLUE, COLOR_RED);     // 在被吃格

    // --- 黑方棋子 (假设用绿色 COLOR_GREEN) ---
    init_pair(21, COLOR_GREEN, COLOR_WHITE);   // 在浅色格
    init_pair(22, COLOR_GREEN, COLOR_BLACK);   // 在深色格
    init_pair(23, COLOR_GREEN, COLOR_YELLOW);  // 在选中格
    init_pair(24, COLOR_GREEN, COLOR_CYAN); // 在预测格
    init_pair(25, COLOR_GREEN, COLOR_RED);     // 在被吃格

    // 31: Dashboard 基础背景 & 边框 (蓝底白字)
    init_pair(31, COLOR_WHITE, COLOR_BLUE); 
    // 32: Dashboard 内的白方信息 (蓝底亮白色/青色)
    init_pair(32, COLOR_CYAN, COLOR_BLUE); 
    // 33: Dashboard 内的黑方信息 (蓝底绿色)
    init_pair(33, COLOR_GREEN, COLOR_BLUE); 
    // 34: Dashboard 内的警告信息 (蓝底红色，或红底白字)
    init_pair(34, COLOR_WHITE, COLOR_RED);

    GameState state = MENU;
    int highlighted = 0;

    Game board;
    while (state != EXIT) {
        if (state == MENU) {
            draw_menu(highlighted);
            int ch = getch();
            switch (ch) {
                case KEY_UP:
                    highlighted = (highlighted + 2) % 3;
                    break;
                case KEY_DOWN:
                    highlighted = (highlighted + 1) % 3;
                    break;
                case '\n': // 回车
                    if (highlighted == 0) state = SINGLE_PLAYER;
                    else if (highlighted == 1) state = DOUBLE_PLAYER;
                    else state = EXIT;
                    break;
            }
        } else if (state == SINGLE_PLAYER) {
            // 游戏结束后，可以将 state 设回 MENU
            board.restart();
            board.single_mode_start();
            state = MENU; 
        } else if (state == DOUBLE_PLAYER) {
            // 游戏结束后，可以将 state 设回 MENU
            board.restart();
            board.double_mode_start();
            state = MENU; 
        }
    }

    endwin();
    return 0;
}