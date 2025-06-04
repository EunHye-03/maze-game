#include "enemy.h"
#include "game.h"
#include "astar.h"
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>

Enemy enemies[MAX_ENEMIES];

void *enemy_thread(void *arg) {
    Enemy *e = (Enemy *)arg;

    while (e->alive) {
        usleep(500000); 

        int (*path)[2] = malloc(sizeof(int[100][2]));
        if (!path) pthread_exit(NULL);

        int pathLen = 0;
        int dx = px - e->x, dy = py - e->y;
        int dist = abs(dx) + abs(dy);

        switch (e->type) {
            case ENEMY_AGGRESSIVE:
                astar_find_path(e->y, e->x, py, px, path, &pathLen);
                break;

            case ENEMY_STALKER:
                if (dist <= 6)
                    astar_find_path(e->y, e->x, py, px, path, &pathLen);
                break;

            case ENEMY_COWARD:
                if (dist <= 5) {
                    int tx = e->x - dx, ty = e->y - dy;
                    astar_find_path(e->y, e->x, ty, tx, path, &pathLen);
                }
                break;

            case ENEMY_RANDOM: {
                int dirs[4][2] = {{0,1},{0,-1},{1,0},{-1,0}};
                int r = rand() % 4;
                int nx = e->x + dirs[r][1], ny = e->y + dirs[r][0];

                if (ny >= 0 && ny < MAZE_HEIGHT && nx >= 0 && nx < MAZE_WIDTH) {
                    if (maze[ny][nx] == ' ') {
                        e->x = nx;
                        e->y = ny;
                    }
                }

                free(path);
                path = NULL;
                continue;
            }
            case TYPE_FROZEN:
                usleep(100000); // 정지 상태로 짧게 슬립
                continue; // 아무 것도 안 하고 반복
        }

        if (pathLen > 1 && path != NULL) {
            int nextY = path[1][0];
            int nextX = path[1][1];
            if (nextY >= 0 && nextY < MAZE_HEIGHT && nextX >= 0 && nextX < MAZE_WIDTH) {
                if (maze[nextY][nextX] == ' ') {
                    e->y = nextY;
                    e->x = nextX;
                }
            }
        }

        free(path);
        path = NULL;
    }

    return NULL;
}

void init_enemies() {
    destroy_enemies();
    srand(time(NULL)); // 랜덤 초기화

    // ✅ 먼저 보스 레벨인지 확인
    if (level % 5 == 0) {
        for (int i = 0; i < MAX_ENEMIES; i++) {
            enemies[i].alive = 0;
            enemies[i].thread = 0;
        }

        enemies[0].x = MAZE_WIDTH - 3;
        enemies[0].y = MAZE_HEIGHT - 3;
        enemies[0].type = TYPE_BOSS;
        enemies[0].alive = 1;
        enemies[0].hp = 3;
        pthread_create(&enemies[0].thread, NULL, enemy_thread, &enemies[0]);
        return; // ✅ 일반 적 생성은 스킵
    }

    // ✅ 일반 적 생성
    int num_enemies = 1 + (level - 1); // 레벨에 따라 적 수 증가
    if (num_enemies > MAX_ENEMIES) num_enemies = MAX_ENEMIES;

    for (int i = 0; i < num_enemies; i++) {
        int tries = 0;
        do {
            enemies[i].x = rand() % MAZE_WIDTH;
            enemies[i].y = rand() % MAZE_HEIGHT;
            tries++;
        } while ((maze[enemies[i].y][enemies[i].x] != ' ') && tries < 100);

        enemies[i].type = i % 4;
        enemies[i].alive = 1;
        pthread_create(&enemies[i].thread, NULL, enemy_thread, &enemies[i]);
    }

    for (int i = num_enemies; i < MAX_ENEMIES; i++) {
        enemies[i].alive = 0;
        enemies[i].thread = 0;
    }
}



void destroy_enemies() {
    pthread_t self = pthread_self();

    for (int i = 0; i < MAX_ENEMIES; i++) {
        enemies[i].alive = 0;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].thread && !pthread_equal(enemies[i].thread, self)) {
            pthread_join(enemies[i].thread, NULL);
            enemies[i].thread = 0;
        }
    }
}


// 라이프 소진 시 재도전 여부 확인
int ask_retry() {
    clear();
    mvprintw(LINES / 2 - 1, (COLS - 30) / 2, "You died. Retry this level?");
    mvprintw(LINES / 2 + 1, (COLS - 30) / 2, "Press Y to retry, N to exit.");
    refresh();
    timeout(-1);
    int ch;
    while (1) {
        ch = getch();
        if (ch == 'Y' || ch == 'y') return 1;
        if (ch == 'N' || ch == 'n') return 0;
    }
}