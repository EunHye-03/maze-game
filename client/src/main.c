#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/stat.h>
#include "game.h"
#include "render.h"
#include "save.h"
#include "menu.h"

int main() {
    initscr();
    keypad(stdscr, TRUE);
    cbreak();
    noecho();
    curs_set(0);
    timeout(0);
    start_color();

    init_colors();
    check_terminal_size();

    if (!load_game()) {
        input_username_screen();
    }
    intro_animation();

    GameState state = STATE_MENU;
    int key;
    int menu_item = 0;
    int info_item = 0;

    while (state != STATE_EXIT) {
        erase();
        getmaxyx(stdscr, g_height, g_width);

        switch (state) {
            case STATE_MENU:
                draw_logo();
                draw_menu(menu_item);

                key = getch();
                if (key == KEY_UP) menu_item--;
                if (key == KEY_DOWN) menu_item++;
                if (menu_item < 0) menu_item = 0;
                if (menu_item > 3) menu_item = 3;

                if (key == '\n') {
                    switch (menu_item) {
                        case 0: state = STATE_GAME; break;
                        case 1: state = STATE_SETTINGS; break;
                        case 2: state = STATE_INFORMATION; break;
                        case 3: state = STATE_EXIT; break;
                    }
                }
                break;

            case STATE_SETTINGS:
                while (1) {
                    erase();
                    getmaxyx(stdscr, g_height, g_width);
                    draw_game_info_menu(info_item);
                    refresh();

                    int info_key = getch();
                    if (info_key == KEY_UP) info_item = (info_item - 1 + 4) % 4;
                    else if (info_key == KEY_DOWN) info_item = (info_item + 1) % 4;
                    else if (info_key == '\n') {

                        switch (info_item) {
                            case 0:
                                select_save_file_menu();
                                break;
                            case 1: // Save Game
                                handle_save_game();
                                break;
                            case 2:
                                handle_intro();
                                break;
                            case 3:
                                state = STATE_MENU;
                                goto end_info_menu;
                        }
                    }
                }
                end_info_menu:
                break;

            case STATE_INFORMATION:
                erase();
                getmaxyx(stdscr, g_height, g_width);

                char title[] = "[ CURRENT PLAYER INFO ]";
                char buf[100];
                
                // Title
                mvprintw(g_height / 2 - 2, (g_width - strlen(title)) / 2, "%s", title);

                // Username
                snprintf(buf, sizeof(buf), "Username: %s", g_username);
                mvprintw(g_height / 2, (g_width - strlen(buf)) / 2, "%s", buf);

                // Level
                snprintf(buf, sizeof(buf), "Level: %d", level);
                mvprintw(g_height / 2 + 1, (g_width - strlen(buf)) / 2, "%s", buf);

                // Score
                snprintf(buf, sizeof(buf), "Score: %d", score);
                mvprintw(g_height / 2 + 2, (g_width - strlen(buf)) / 2, "%s", buf);

                mvprintw(g_height - 2, g_width / 2 - 15, "Press any key to return.");
                refresh();

                timeout(-1);
                getch();
                timeout(0);
                state = STATE_MENU;
                break;

            case STATE_GAME:
                start_maze_game();
                state = STATE_MENU;
                break;

            default:
                state = STATE_EXIT;
        }

        refresh();
        napms(30);
    }

    endwin();
    return 0;
}
