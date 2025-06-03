#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include "game.h"
#include "render.h"
#include "save.h"

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
                                if (load_game()) {
                                    mvprintw(g_height / 2 + 8, g_width / 2 - 6, "Load success!");
                                } else {
                                    mvprintw(g_height / 2 + 8, g_width / 2 - 6, "Load failed.");
                                }
                                refresh(); napms(800);
                                break;
                            case 1:
                                save_game();
                                mvprintw(g_height / 2 + 8, g_width / 2 - 8, "Saved successfully!");
                                refresh(); napms(800);
                                break;
                            case 2:
                                input_username_screen();
                                intro_animation();
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

                mvprintw(g_height / 2 - 2, g_width / 2 - 25, "[ CURRENT PLAYER INFO ]");
                mvprintw(g_height / 2, g_width / 2 - 30, "Username: %s", g_username);
                mvprintw(g_height / 2 + 1, g_width / 2 - 30, "Level: %d", level);
                mvprintw(g_height / 2 + 2, g_width / 2 - 30, "Score: %d", score);
                mvprintw(g_height / 2 + 3, g_width / 2 - 30, "Lives: %d", lifes);

                struct dirent *entry;
                DIR *dp = opendir("assets/save");

                int list_y = g_height / 2 - 2;
                int list_x = g_width / 2 + 5;

                if (dp) {
                    mvprintw(list_y - 2, list_x, "[ SAVED SCORES ]");
                    while ((entry = readdir(dp)) != NULL) {
                #ifdef DT_REG
                        if (entry->d_type == DT_REG) {
                #else
                        struct stat st;
                        char path[512];
                        snprintf(path, sizeof(path), "assets/save/%s", entry->d_name);
                        stat(path, &st);
                        if (S_ISREG(st.st_mode)) {
                #endif
                            char path[512];
                            snprintf(path, sizeof(path), "assets/save/%s", entry->d_name);
                            FILE *fp = fopen(path, "r");
                            if (fp) {
                                char name[32];
                                int saved_level, saved_score, saved_lifes;
                                if (fscanf(fp, "%31s %d %d %d", name, &saved_level, &saved_score, &saved_lifes) == 4) {
                                    mvprintw(list_y++, list_x, "%-10s  Score: %d", name, saved_score);
                                }
                                fclose(fp);
                            }
                        }
                    }
                    closedir(dp);
                } else {
                    mvprintw(list_y, list_x, "No saved users.");
                }

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
