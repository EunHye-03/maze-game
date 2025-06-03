#include "game.h"
#include "render.h"
#include "save.h"
#include <ncurses.h>
#include <string.h>
#include <unistd.h> 
#include <ctype.h>
#include <stdlib.h>

char g_old_username[32] = {0};

const char *menu_logo[] = {
    " # #       # #           #          # # # # # #   # # # # #",
    " #   #   #   #         #   #                #     #",
    " #     #     #      #    #    #           #       # # # # #",
    " #           #    #            #        #         #",
    " #           #  #                #  # # # # # #   # # # # #"
};

void init_colors() {
    init_pair(COLOR_HUD, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLACK); // title
    init_pair(3, COLOR_CYAN, COLOR_BLACK);  // text
}

void check_terminal_size() {
    int min_width = 155;
    int min_height = 40;

    while (1) {
        erase();
        getmaxyx(stdscr, g_height, g_width);

        char size_msg[64];
        snprintf(size_msg, sizeof(size_msg), "Current size: %dx%d", g_width, g_height);
        mvprintw(1, g_width / 2 - strlen(size_msg) / 2, "%s", size_msg);

        if (g_width >= min_width && g_height >= min_height) {
            const char *ok_msg = "Terminal size is sufficient. Starting game...";
            mvprintw(3, g_width / 2 - strlen(ok_msg) / 2, "%s", ok_msg);
            refresh();
            napms(1000);
            break;
        } else {
            const char *warn1 = "[!] Terminal size too small!";
            const char *warn2 = "Resize to at least 155x40 and press any key...";
            mvprintw(3, g_width / 2 - strlen(warn1) / 2, "%s", warn1);
            mvprintw(4, g_width / 2 - strlen(warn2) / 2, "%s", warn2);
            refresh();
            timeout(-1); getch(); timeout(0);
        }
    }
}

void input_username_screen() {
    timeout(-1);
    echo();
    curs_set(1);

    // 기존 이름 백업
    strncpy(g_old_username, g_username, sizeof(g_old_username) - 1);

    char input[32] = {0};
    int idx = 0;

    erase();
    getmaxyx(stdscr, g_height, g_width);
    mvprintw(g_height / 2 - 2, g_width / 2 - 20, "Enter your name (ENG only, max 31 chars):");
    mvprintw(g_height / 2, g_width / 2 - 10, "> ");
    refresh();

    int x = g_width / 2 - 8;
    move(g_height / 2, x);

    int ch;
    while ((ch = getch()) != '\n') {
        if (isalpha(ch) && idx < 31) {
            input[idx++] = ch;
            mvaddch(g_height / 2, x++, ch);
        } else if ((ch == KEY_BACKSPACE || ch == 127 || ch == '\b') && idx > 0) {
            idx--;
            x--;
            input[idx] = '\0';
            mvaddch(g_height / 2, x, ' ');
            move(g_height / 2, x);
        }
        refresh();
    }

    if (idx == 0) {
        strncpy(g_username, "Guest", sizeof(g_username) - 1);
    } else {
        strncpy(g_username, input, sizeof(g_username) - 1);
    }

    g_username[sizeof(g_username) - 1] = '\0';

    // 이전 이름의 파일 삭제
    if (strlen(g_old_username) > 0 && strcmp(g_old_username, g_username) != 0) {
        char old_path[256];
        snprintf(old_path, sizeof(old_path), "assets/save/%s.txt", g_old_username);
        remove(old_path);  // 삭제 시도 (실패해도 무방)
    }

    save_game();

    noecho();
    curs_set(0);
    timeout(0);
}


void intro_animation() {
    char welcome_msg[128];
    snprintf(welcome_msg, sizeof(welcome_msg), "Welcome to the Maze, %s!", g_username);

    int len = strlen(welcome_msg);
    erase();
    for (int i = 0; i <= len; ++i) {
        erase();
        mvprintw(g_height / 2, g_width / 2 - len / 2, "%.*s", i, welcome_msg);
        refresh();
        napms(50);
    }

    napms(1000);
}

void draw_logo() {
    static int logo_reveal_index = 0;
    static int tick = 0;
    if (logo_reveal_index < (int)(sizeof(menu_logo) / sizeof(menu_logo[0]))) {
        if (++tick % 5 == 0) logo_reveal_index++;
    }
    attron(COLOR_PAIR(2));
    attron(A_BOLD);
    int logo_h = sizeof(menu_logo) / sizeof(menu_logo[0]);
    int logo_w = 0;
    for (int i = 0; i < logo_h; ++i) {
        int len = strlen(menu_logo[i]);
        if (len > logo_w) logo_w = len;
    }
    for (int i = 0; i < logo_h && i < logo_reveal_index; ++i) {
        mvprintw(3 + i, g_width / 2 - logo_w / 2, "%s", menu_logo[i]);
    }

    const char *quotes[] = {
        "\"In the silence of the maze, your heart finds its path.\"",
        "\"You are not lost. You're searching.\"",
        "\"Every wall hides a choice. Every turn, a story.\"",
        "\"To escape the maze, first step into it.\""
    };
    static int quote_index = 0;
    static int quote_delay = 0;
    if (++quote_delay > 150) { quote_index = (quote_index + 1) % 4; quote_delay = 0; }
    const char *line = quotes[quote_index];
    mvprintw(10, g_width / 2 - strlen(line) / 2, "%s", line);

    attroff(A_BOLD);
    attroff(COLOR_PAIR(2));
}

void draw_game_info_menu(int selected) {
    const char *items[] = {
        "Load Game",
        "Save Game",
        "Change User",
        "Back"
    };

    int count = sizeof(items) / sizeof(items[0]);
    int spacing = 2;
    int start_y = g_height / 2 - count;

    for (int i = 0; i < count; ++i) {
        char temp[64], buf[80];
        if (i == selected) {
            for (int j = 0; j < (int)strlen(items[i]) && j < (int)(sizeof(temp) - 1); ++j)
                temp[j] = toupper(items[i][j]);
            temp[strlen(items[i])] = '\0';
            snprintf(buf, sizeof(buf), "> %s <", temp);
        } else {
            snprintf(buf, sizeof(buf), "%s", items[i]);
        }

        if (i == selected) attron(A_REVERSE);
        mvprintw(start_y + i * spacing, g_width / 2 - strlen(buf) / 2, "%s", buf);
        if (i == selected) attroff(A_REVERSE);
    }

    const char *title = "MANAGE GAME INFO";
    attron(COLOR_PAIR(2));
    mvprintw(start_y - 6, g_width / 2 - strlen(title) / 2, "%s", title);
    attroff(COLOR_PAIR(2));

    box(stdscr, 0, 0);
}

void draw_menu(int selected) {
    const char *items[] = {
        "start game", "settings", "information", "exit"
    };
    const char *hover_quotes[] = {
        "Begin your journey.",
        "Manage save data or change user.",
        "Learn about the game.",
        "See you again!"
    };
    int spacing = 2;
    int start_y = g_height / 2 + 2;

    for (int i = 0; i < 4; ++i) {
        char upper[64], buf[80];
        if (i == selected) {
            snprintf(upper, sizeof(upper), "%s", items[i]);
            for (int j = 0; upper[j]; ++j) upper[j] = toupper(upper[j]);
            snprintf(buf, sizeof(buf), ">   %s   <", upper);
        } else {
            snprintf(buf, sizeof(buf), "    %s    ", items[i]);
        }

        attron(A_BOLD);
        attron(COLOR_PAIR(2));
        mvprintw(start_y + i * spacing, g_width / 2 - strlen(buf) / 2, "%s", buf);
        attroff(COLOR_PAIR(2));
        attroff(A_BOLD);
    }

    char welcome_buf[64];
    snprintf(welcome_buf, sizeof(welcome_buf), "Welcome, %s!", g_username);
    attron(COLOR_PAIR(3));
    mvprintw(g_height - 5, g_width / 2 - strlen(welcome_buf) / 2, "%s", welcome_buf);
    mvprintw(g_height - 3, g_width / 2 - 26, "Use UP/DOWN arrows   Enter: Select   Ctrl+C: Quit");
    const char *credit_msg = "Developed by hyunsu, eunhye";
    mvprintw(g_height - 2, g_width / 2 - strlen(credit_msg) / 2, "%s", credit_msg);
    mvprintw(g_height - 6, g_width / 2 - strlen(hover_quotes[selected]) / 2, "%s", hover_quotes[selected]);
    attroff(COLOR_PAIR(3));

    box(stdscr, 0, 0);
}