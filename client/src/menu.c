#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>

#include "menu.h"
#include "game.h"
#include "save.h"
#include "render.h"

// 메뉴 상태 정의
typedef enum {
    MENU_SELECTING,
    MENU_CONFIRM_DELETE,
    MENU_DONE
} MenuState;

extern int g_height, g_width;
extern const char* get_save_progress(const char* save_name);
extern void clear_all_save_files(void);

void select_save_file_menu() {
    char save_files[100][256];
    int save_count = 0;
    list_save_files(save_files, &save_count);

    int choice = 0;
    MenuState state = MENU_SELECTING;
    int confirm_choice = -1;

    timeout(-1);

    while (state != MENU_DONE) {
        erase();
        getmaxyx(stdscr, g_height, g_width);

        const char *title = "[ Select Save File ]";
        mvprintw(g_height / 2 - save_count - 4, (g_width - strlen(title)) / 2, "%s", title);

        for (int i = 0; i < save_count; i++) {
            int y = g_height / 2 - save_count / 2 + i;
            char name[256];
            strncpy(name, save_files[i], sizeof(name) - 1);
            name[sizeof(name) - 1] = '\0';

            char *dot = strstr(name, ".txt");
            if (dot) *dot = '\0';

            const char* progress = get_save_progress(name);
            int name_x = g_width / 2 - 20;
            int progress_x = g_width / 2 + 10;

            if (i == choice && state == MENU_SELECTING) {
                attron(A_REVERSE);
                mvprintw(y, name_x, "> %s <", name);
                attroff(A_REVERSE);
            } else {
                mvprintw(y, name_x + 2, "%s", name);
            }
            mvprintw(y, progress_x, "Progress: %s", progress);
        }

        const char *back_label = (choice == save_count && state == MENU_SELECTING) ? "> Back <" : "  Back  ";
        int back_y = g_height / 2 + save_count / 2 + 2;
        mvprintw(back_y, (g_width - strlen(back_label)) / 2, "%s", back_label);

        const char *hint = "Enter: Load   D: Delete Save   R: Reset All   UP/DOWN: Move";
        mvprintw(back_y + 2, (g_width - strlen(hint)) / 2, "%s", hint);

        if (state == MENU_CONFIRM_DELETE) {
            char confirm_msg[512];
            char tmp_name[256];
            strncpy(tmp_name, save_files[confirm_choice], sizeof(tmp_name) - 1);
            tmp_name[sizeof(tmp_name) - 1] = '\0';
            char *dot = strstr(tmp_name, ".txt");
            if (dot) *dot = '\0';

            snprintf(confirm_msg, sizeof(confirm_msg), "Delete '%s'? [ENTER=yes / any key=no]", tmp_name);
            mvprintw(back_y + 4, (g_width - strlen(confirm_msg)) / 2, "%s", confirm_msg);
        }

        refresh();
        int key = getch();

        switch (state) {
            case MENU_SELECTING:
                if (key == KEY_UP) {
                    choice = (choice - 1 + (save_count + 1)) % (save_count + 1);
                } else if (key == KEY_DOWN) {
                    choice = (choice + 1) % (save_count + 1);
                } else if (tolower(key) == 'd' && choice < save_count) {
                    confirm_choice = choice;
                    state = MENU_CONFIRM_DELETE;
                } else if (tolower(key) == 'r') {
                    clear_all_save_files();
                    list_save_files(save_files, &save_count);
                    choice = 0;
                } else if (key == '\n') {
                    if (choice == save_count) {
                        return;
                    } else {
                        if (load_specific_game(save_files[choice])) {
                            intro_animation();
                        }
                        state = MENU_DONE;
                    }
                } else if (key == 27 || key == KEY_BACKSPACE || key == 127) {
                    return;
                }
                break;

            case MENU_CONFIRM_DELETE:
                if (key == '\n') {
                    char filepath[512];
                    snprintf(filepath, sizeof(filepath), "assets/save/%s", save_files[confirm_choice]);
                    if (remove(filepath) == 0) {
                        for (int j = confirm_choice; j < save_count - 1; j++) {
                            strcpy(save_files[j], save_files[j + 1]);
                        }
                        save_count--;
                        if (choice >= save_count) choice = save_count;
                    } else {
                        const char *fail = "Failed to delete file.";
                        mvprintw(back_y + 6, (g_width - strlen(fail)) / 2, "%s", fail);
                        refresh();
                        napms(1000);
                    }
                }
                state = MENU_SELECTING;
                break;
            case MENU_DONE:
                break;
        }
    }
}

void handle_save_game(void) {
    char savename[32] = {0};
    if (!input_save_name_screen(savename)) return;

    if (strlen(savename) == 0)
        strcpy(savename, "default_save");

    char check_path[256];
    snprintf(check_path, sizeof(check_path), "assets/save/%s.txt", savename);
    FILE *check = fopen(check_path, "r");
    if (check) {
        fclose(check);
        mvprintw(g_height / 2 + 2, (g_width - 40) / 2, "Overwrite existing save? (y/n)");
        refresh();
        int ch = getch();
        if (ch != 'y' && ch != 'Y') return;
    }

    save_game(check_path);
}

void handle_intro(void) {
    input_username_screen();
    intro_animation();
}

void handle_return_to_menu(int *state) {
    *state = STATE_MENU;
}
