#include <ncurses.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "menu.h"
#include "game.h"
#include "save.h"
#include <render.h>

// 메뉴 상태 정의
typedef enum {
    MENU_SELECTING,
    MENU_CONFIRM_DELETE,
    MENU_DONE
} MenuState;

void select_save_file_menu() {
    char save_files[100][256];
    int save_count = 0;
    list_save_files(save_files, &save_count);
    
    int choice = 0;
    MenuState state = MENU_SELECTING;
    int confirm_choice = -1;

    timeout(-1); // 키 입력 대기 모드

    while (state != MENU_DONE) {
        erase();
        getmaxyx(stdscr, g_height, g_width);

        const char *title = "[ Select Save File ]";
        mvprintw(g_height / 2 - save_count - 3, g_width / 2 - strlen(title) / 2, "%s", title);

        // 저장 목록 출력
        for (int i = 0; i < save_count; i++) {
            int y = g_height / 2 - save_count / 2 + i;
            char name[256];
            strncpy(name, save_files[i], sizeof(name) - 1);
            name[sizeof(name) - 1] = '\0';

            char *dot = strstr(name, ".txt");
            if (dot) *dot = '\0';

            if (i == choice && state == MENU_SELECTING) attron(A_REVERSE);
            mvprintw(y, g_width / 2 - strlen(name) / 2, "%s", name);
            if (i == choice && state == MENU_SELECTING) attroff(A_REVERSE);
        }

        // Back 옵션
        const char *back = "Back";
        int back_y = g_height / 2 + save_count / 2 + 2;
        if (choice == save_count && state == MENU_SELECTING) attron(A_REVERSE);
        mvprintw(back_y, g_width / 2 - strlen(back) / 2, "%s", back);
        if (choice == save_count && state == MENU_SELECTING) attroff(A_REVERSE);

        const char *hint = "Enter: Load   D: Delete Save   UP/DOWN: Move";
        mvprintw(back_y + 2, g_width / 2 - strlen(hint) / 2, "%s", hint);

        // 삭제 확인 메시지
        if (state == MENU_CONFIRM_DELETE) {
            char confirm_msg[512];
            char tmp_name[256];
            strncpy(tmp_name, save_files[confirm_choice], sizeof(tmp_name) - 1);
            tmp_name[sizeof(tmp_name) - 1] = '\0';
            char *dot = strstr(tmp_name, ".txt");
            if (dot) *dot = '\0';

            snprintf(confirm_msg, sizeof(confirm_msg), "Delete '%s'? [ENTER=yes / any key=no]", tmp_name);
            mvprintw(back_y + 4, g_width / 2 - strlen(confirm_msg) / 2, "%s", confirm_msg);
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
                } else if (key == '\n') {
                    if (choice == save_count) {
                        // "Back"을 선택한 경우: 그냥 메뉴 종료
                        state = MENU_DONE;
                    } else {
                        // 저장 파일 선택 시에만 게임을 로드
                        if (load_specific_game(save_files[choice])) {
                            intro_animation();
                        }
                        state = MENU_DONE;
                    }
                } else if (key == 27 || key == KEY_BACKSPACE || key == 127) {
                    state = MENU_DONE;
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
                        mvprintw(back_y + 6, g_width / 2 - strlen(fail) / 2, "%s", fail);
                        refresh();
                        napms(1000);
                    }
                }
                state = MENU_SELECTING;
                break;
            case MENU_DONE:
                break;  // nothing to do
        }
    }
}

extern int g_height, g_width;

void handle_save_game(void) {
    char savefile[64], savename[32] = {0};
    input_save_name_screen(savename);

    if (strlen(savename) == 0)
        strcpy(savename, "default_save");

    time_t now = time(NULL);
    struct tm *local = localtime(&now);

    snprintf(savefile, sizeof(savefile),
             "%s_%02d%02d_%02d%02d.txt",
             savename,
             local->tm_mon + 1,
             local->tm_mday,
             local->tm_hour,
             local->tm_min);

    save_game(savefile);
}

void handle_intro(void) {
    input_username_screen();
    intro_animation();
}

void handle_return_to_menu(int *state) {
    *state = STATE_MENU;
}