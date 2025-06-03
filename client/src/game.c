#include "game.h"
#include "network.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#define true 1
#define false 0

int g_width = 0;
int g_height = 0;
char g_username[32] = {0};

short level = 1;
short score = 0;
short lifes = 3;

int player_has_key = false;
char maze[MAZE_HEIGHT][MAZE_WIDTH];

int dir[4][2] = {{0,2},{2,0},{0,-2},{-2,0}};

// DFS 미로 생성
void generate_maze(int y, int x) {
    maze[y][x] = ' ';
    int order[] = {0, 1, 2, 3};
    for (int i = 0; i < 4; ++i) {
        int r = rand() % 4;
        int tmp = order[i];
        order[i] = order[r];
        order[r] = tmp;
    }

    for (int i = 0; i < 4; i++) {
        int ny = y + dir[order[i]][0];
        int nx = x + dir[order[i]][1];

        if (ny > 0 && ny < MAZE_HEIGHT - 1 && nx > 0 && nx < MAZE_WIDTH - 1 && maze[ny][nx] == '#') {
            maze[y + dir[order[i]][0]/2][x + dir[order[i]][1]/2] = ' ';
            generate_maze(ny, nx);
        }
    }
}

void draw_maze() {
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            mvaddch(y + 1, x + 1, maze[y][x]);
        }
    }
}

void start_maze_game() {
    for (int y = 0; y < MAZE_HEIGHT; y++)
        for (int x = 0; x < MAZE_WIDTH; x++)
            maze[y][x] = '#';

    srand(time(NULL));
    generate_maze(1, 1);

    int px = 1, py = 1;
    player_has_key = false;

    // 키와 포탈 배치
    int key_x = MAZE_WIDTH - 4;
    int key_y = 1;
    int portal_x = MAZE_WIDTH - 2;
    int portal_y = MAZE_HEIGHT - 2;
    maze[key_y][key_x] = 'K';     // 키
    maze[portal_y][portal_x] = 'X'; // 잠긴 포탈

    timeout(-1);
    while (1) {
        erase();
        draw_maze();
        mvaddch(py + 1, px + 1, '@');

        mvprintw(MAZE_HEIGHT + 2, 2, "Find the key (K) → then go to portal (P). Press q to quit.");
        if (player_has_key)
            mvprintw(MAZE_HEIGHT + 3, 2, "You have the key!");

        refresh();

        int key = getch();
        if (key == 'q') break;

        int nx = px, ny = py;
        if (key == KEY_UP) ny--;
        if (key == KEY_DOWN) ny++;
        if (key == KEY_LEFT) nx--;
        if (key == KEY_RIGHT) nx++;

        char next = maze[ny][nx];
        if (next == ' ' || next == 'K' || next == 'P' || next == 'X') {
            px = nx;
            py = ny;
        }

        if (maze[py][px] == 'K') {
            player_has_key = true;
            maze[py][px] = ' ';  // 키 먹음
            maze[portal_y][portal_x] = 'P';  // 포탈 열림
        }

        if (maze[py][px] == 'P' && player_has_key) {
            mvprintw(MAZE_HEIGHT + 4, 2, "You escaped the maze! Press any key...");
            send_score(g_username, score, level);
            getch();
            break;
        }
    }
    timeout(0);
}
