#include "game.h"
#include "astar.h"
#include "enemy.h"
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>

#define true 1
#define false 0

int ghost_mode = false;
time_t last_z_use = 0;
const int Z_COOLDOWN = 13;
int z_usage_count = 0;

time_t level_start_time;
int time_left = 120;

int g_width = 0;
int g_height = 0;
char g_username[32] = {0};

short level = 1;
short score = 0;
short lifes = 3;

int player_has_key = false;
int player_has_breaker = false;
int player_speed_is = false;
int player_speed = 1;

char maze[MAZE_HEIGHT][MAZE_WIDTH];
int px = 1, py = 1;

int dir[4][2] = {{0,2},{2,0},{0,-2},{-2,0}};

int center_y_offset = 0;
int center_x_offset = 0;

int last_dir_y = 0, last_dir_x = 1;

char selected_items[2];  // 선택된 무작위 아이템 2개
int has_push_item = false;  // M 아이템 획득 여부

PlayerStyle player_style;

const char* style_names[] = {
    "Speedster (+10 speed)",
    "Tank (+1 life)",
    "Hacker (Portal without key)",
    "Breaker (Start with wall breaker)",
    "Trickster (Better mystery effects)"
};

void place_random_item(char item) {
    int attempts = 1000;
    while (attempts--) {
        int y = rand() % MAZE_HEIGHT;
        int x = rand() % MAZE_WIDTH;
        if (maze[y][x] == ' ') {
            maze[y][x] = item;
            return;
        }
    }
}

void apply_player_style_effects() {
    switch (player_style) {
        case STYLE_SPEEDSTER:
            player_speed_is = true;
            break;
        case STYLE_TANK:
            lifes += 1;
            break;
        case STYLE_HACKER:
            // 포탈에 키 없어도 통과 → 별도 조건에서 처리 필요
            break;
        case STYLE_BREAKER:
            player_has_breaker = true;
            break;
        case STYLE_TRICKSTER:
            // Mystery 효과 강화 → random_box_effect 내부에서 분기 필요
            break;
        default:
            break;
    }
}

void choose_player_style() {
    int choice = 0;
    timeout(-1);  // 키 입력 대기

    while (1) {
        erase();
        getmaxyx(stdscr, g_height, g_width);

        const char* title = "Choose Your Style:";
        mvprintw(g_height / 2 - STYLE_COUNT - 2, (g_width - strlen(title)) / 2, "%s", title);

        for (int i = 0; i < STYLE_COUNT; i++) {
            if (i == choice) attron(A_REVERSE);
            mvprintw(g_height / 2 - STYLE_COUNT / 2 + i, (g_width - strlen(style_names[i])) / 2, "%s", style_names[i]);
            if (i == choice) attroff(A_REVERSE);
        }

        const char* hint = "Use UP/DOWN and ENTER to select";
        mvprintw(g_height / 2 + STYLE_COUNT + 2, (g_width - strlen(hint)) / 2, "%s", hint);
        refresh();

        int key = getch();
        if (key == KEY_UP)
            choice = (choice - 1 + STYLE_COUNT) % STYLE_COUNT;
        else if (key == KEY_DOWN)
            choice = (choice + 1) % STYLE_COUNT;
        else if (key == '\n')
            break;
    }

    player_style = (PlayerStyle)choice;
}


void spawn_enemy_at(int x, int y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].alive) {
            enemies[i].x = x;
            enemies[i].y = y;
            enemies[i].alive = true;
            return;
        }
    }
}

void shuffle_array(char *array, int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        char temp = array[i];
        array[i] = array[j];
        array[j] = temp;
    }
}

void apply_random_box_effect() {
    int roll;
    if (player_style == STYLE_TRICKSTER) {
        roll = rand() % 10;  // 트릭스터는 10종으로 다양하게
    } else {
        roll = rand() % 6;  // 일반 플레이어는 6종
    }

    attron(COLOR_PAIR(6));
    switch (roll) {
        case 0:
            player_has_breaker = true;
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Mystery Box: Wall breaker acquired!");
            break;
        case 1:
            has_push_item = true;
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Mystery Box: Push item gained! Press 'z'");
            break;
        case 2:
            lifes++;
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Mystery Box: Lucky! +1 life!");
            break;
        case 3:
            spawn_enemy_at(px + 1, py);
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Mystery Box: Enemy spawned nearby!");
            break;
        case 4:
            player_has_key = false;
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Mystery Box: You dropped your key!");
            break;
        case 5:
            player_speed = (player_speed > 1) ? player_speed - 1 : 1;
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Mystery Box: You feel sluggish... Speed down!");
            break;
        case 6:
            px = 1; py = 1;
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Mystery Box: Teleported to start!");
            break;
        case 7:
            lifes--;
            if (lifes < 0) lifes = 0;
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Mystery Box: Ouch! You got hurt! -1 life");
            break;
        case 8:
            for (int y = 0; y < MAZE_HEIGHT; y++) {
                for (int x = 0; x < MAZE_WIDTH; x++) {
                    if (maze[y][x] == ' ' && rand() % 20 == 0)
                        maze[y][x] = '#';
                }
            }
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Mystery Box: Walls suddenly grew!");
            break;
        case 9:
            score = (score >= 50) ? score - 50 : 0;
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Mystery Box: You lost 50 points!");
            break;
    }
    attroff(COLOR_PAIR(6));
}


void pickup_key(int y, int x) {
    player_has_key = true;
    maze[y][x] = ' ';
    maze[MAZE_HEIGHT - 2][MAZE_WIDTH - 2] = 'P';  // 출구 열림
    mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "You picked up the key!");
}

void pickup_breaker(int y, int x) {
    player_has_breaker = true;
    maze[y][x] = ' ';
    mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Wall breaker acquired! Press 'w'");
}

void activate_switch() {
    for (int y = 0; y < MAZE_HEIGHT; y++) {
        for (int x = 0; x < MAZE_WIDTH; x++) {
            if (maze[y][x] == 'D')
                maze[y][x] = ' ';
        }
    }
    mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "You activated a switch!");
}

void collapse_tile(int y, int x) {
    maze[y][x] = '#';
    px = x;
    py = y;
    mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "The path collapsed behind you!");
}

void pickup_push_item(int y, int x) {
    has_push_item = true;
    maze[y][x] = ' ';
    mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Push item acquired! Press 'z'");
}


void draw_status_screen() {
    erase();
    getmaxyx(stdscr, g_height, g_width);
    center_y_offset = (g_height - MAZE_HEIGHT) / 2 - 1;
    center_x_offset = (g_width - MAZE_WIDTH * 2) / 2 - 1;

    const char *status1 = "WELCOME TO THE MAZE GAME";
    const char *status2 = "Press [TAB] to Start";
    const char *status3 = "Press [Q] to Quit";
    

    char info1[512], info2[512], info3[512],  info4[512], info5[512];
    snprintf(info1, sizeof(info1), "Speed: %d", player_speed);
    snprintf(info2, sizeof(info2), "Key: %s", player_has_key ? "Yes" : "No");
    snprintf(info3, sizeof(info3), "Wall Breaker: %s", player_has_breaker ? "Yes" : "No");
    snprintf(info4, sizeof(info4), "Random Items: %c, %c (Key is always included)", selected_items[0], selected_items[1]);
    snprintf(info5, sizeof(info5), "Your Style: %s", style_names[player_style]);

    int y = g_height / 2;
    mvprintw(y - 4, (g_width - strlen(status1)) / 2, "%s", status1);
    mvprintw(y - 3, (g_width - strlen(status2)) / 2, "%s", status2);
    mvprintw(y - 2, (g_width - strlen(status3)) / 2, "%s", status3);
    mvprintw(y,     (g_width - strlen(info1)) / 2, "%s", info1);
    mvprintw(y + 1, (g_width - strlen(info2)) / 2, "%s", info2);
    mvprintw(y + 2, (g_width - strlen(info3)) / 2, "%s", info3);
    mvprintw(y + 3, (g_width - strlen(info4)) / 2, "%s", info4);
    mvprintw(y + 4, (g_width - strlen(info5)) / 2, "%s", info5);
    refresh();
}

void wait_screen() {
    timeout(-1);
    while (1) {
        draw_status_screen();
        int key = getch();
        if (key == '\t') break;
    }
}

void place_switch_near_center() {
    int attempts = 1000;
    int cy = MAZE_HEIGHT / 2;
    int cx = MAZE_WIDTH / 2;

    while (attempts--) {
        int y = cy + (rand() % 5) - 2;  // 중심 주변
        int x = cx + (rand() % 5) - 2;
        if (y > 0 && y < MAZE_HEIGHT - 1 && x > 0 && x < MAZE_WIDTH - 1 && maze[y][x] == ' ') {
            maze[y][x] = 'S';
            return;
        }
    }

    place_random_item('S'); 
}

void place_door_near_exit() {
    int ex = MAZE_WIDTH - 2;
    int ey = MAZE_HEIGHT - 2;

    // 출구로 가는 길 중 하나를 골라 문을 설치
    int dx[] = {-1, 0, 1, 0};
    int dy[] = {0, -1, 0, 1};

    for (int i = 0; i < 4; i++) {
        int nx = ex + dx[i];
        int ny = ey + dy[i];
        if (nx > 0 && nx < MAZE_WIDTH - 1 && ny > 0 && ny < MAZE_HEIGHT - 1 &&
            maze[ny][nx] == ' ') {
            maze[ny][nx] = 'D'; // 열린 길을 문으로 막음
            return;
        }
    }

    // 백업: 주변에 빈공간이 없을 경우 랜덤
    place_random_item('D');
}


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
            char ch = maze[y][x];
            int color = 0;
            switch (ch) {
                case '#': color = 9; break;
                case 'K': color = 4; break;
                case 'X': color = 6; break;
                case 'P': color = 5; break;
                case 'B': color = 3; break;
                case 'S': color = 3; break;  // switch
                case 'D': color = 3; break;  // door
                case 'C': color = 3; break;  // collapsing tile
                case 'M': color = 3; break;
                case '?': color = 3; break;
            }
            if (color > 0) attron(COLOR_PAIR(color));
            mvprintw(center_y_offset + y + 1, center_x_offset + (x + 1) * 2, "%c ", ch);
            if (color > 0) attroff(COLOR_PAIR(color));
        }
    }
    attron(COLOR_PAIR(7));
    mvprintw(center_y_offset + py + 1, center_x_offset + (px + 1) * 2, "@ ");
    attroff(COLOR_PAIR(7));

    attron(COLOR_PAIR(10));
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].alive)
            mvprintw(center_y_offset + enemies[i].y + 1, center_x_offset + (enemies[i].x + 1) * 2, "E ");
    }
    attroff(COLOR_PAIR(10));

    int ui_x = center_x_offset - 28; // 미로 왼쪽에 위치하도록 설정
    if (ui_x < 0) ui_x = 0;          // 화면 왼쪽을 벗어나지 않게
    int ui_y = center_y_offset + 1;  // 미로 상단에 맞춰 정렬
    time_t now = time(NULL);
    int z_remaining = Z_COOLDOWN - (now - last_z_use);
    if (z_remaining < 0) z_remaining = 0;

    mvprintw(ui_y++, ui_x, "+------------------------+");
    mvprintw(ui_y++, ui_x, "|     Maze Game UI       |");
    mvprintw(ui_y++, ui_x, "+------------------------+");
    mvprintw(ui_y++, ui_x, "| Level     : %-3d        |", level);
    mvprintw(ui_y++, ui_x, "| Score     : %-3d        |", score);
    mvprintw(ui_y++, ui_x, "| Lifes     : %-3d        |", lifes);
    mvprintw(ui_y++, ui_x, "| Time Left : %-3d        |", time_left);
    mvprintw(ui_y++, ui_x, "+------------------------+");
    if (player_has_key)
        mvprintw(ui_y++, ui_x, "| Key        : Yes       |");
    if (player_has_breaker)
        mvprintw(ui_y++, ui_x, "| Wall Break : Ready     |");
    if (has_push_item)
        mvprintw(ui_y++, ui_x, "| Push Box   : Ready     |");
    if (ghost_mode)
        mvprintw(ui_y++, ui_x, "| Z Freeze   : Active    |");
    mvprintw(ui_y++, ui_x, "+------------------------+");

    ui_y++;
    mvprintw(ui_y++, ui_x, "[Controls]");
    mvprintw(ui_y++, ui_x, "Arrow Keys - Move");
    mvprintw(ui_y++, ui_x, "W - Wall Break");
    mvprintw(ui_y++, ui_x, "Z - Freeze Enemy");

    ui_y++;
    mvprintw(ui_y++, ui_x, "[Item Legend]");
    mvprintw(ui_y++, ui_x, "'K' = Key");
    mvprintw(ui_y++, ui_x, "'B' = Wall Breaker");
    mvprintw(ui_y++, ui_x, "'S' = Switch, 'D' = Door");
    mvprintw(ui_y++, ui_x, "'M' = Push Box (x)");
    mvprintw(ui_y++, ui_x, "'?' = Mystery");
    mvprintw(ui_y++, ui_x, "'C' = Collapsing Tile");
    mvprintw(ui_y++, ui_x, "'T' = +10 Time");
    mvprintw(ui_y++, ui_x, "'P' = Portal");
    mvprintw(ui_y++, ui_x, "'X' = Locked Portal");
    mvprintw(ui_y++, ui_x, "Reach 'P' in time!");

    mvprintw(ui_y++, ui_x, "+------------------------+");
    mvprintw(ui_y++, ui_x, "| Z Cooldown : %2ds       |", z_remaining);
    mvprintw(ui_y++, ui_x, "| Z Used     : %d times   |", z_usage_count);

}

void generate_maze_with_items() {
    memset(maze, '#', sizeof(maze));
    generate_maze(1, 1);  // DFS 기반 생성

    px = 1; py = 1;
    player_has_key = false;
    player_has_breaker = false;
    has_push_item = false;

    // 필수 아이템
    place_random_item('K');
    maze[MAZE_HEIGHT - 2][MAZE_WIDTH - 2] = 'X';

    // 해커면 포탈 즉시 열림
    if (player_style == STYLE_HACKER) {
        maze[MAZE_HEIGHT - 2][MAZE_WIDTH - 2] = 'P';
    }

    // 선택 아이템 후보 목록
    char pool[] = { 'B', 'S', 'C', 'M', '?', 'T' };
    shuffle_array(pool, 6);

    int has_S = 0;
    int count = 0;

    for (int i = 0; i < 2; i++) {
        char item = pool[i];
        selected_items[i] = item;

        if (item == 'S') {
            has_S = 1;
            place_switch_near_center();
        } else {
            place_random_item(item);
        }

        count++;
    }

    // level ≥ 3: 'C' or '?' 반드시 삽입
    if (level >= 3) {
        int ensured = 0;
        for (int i = 0; i < count; i++) {
            if (selected_items[i] == 'C' || selected_items[i] == '?') {
                ensured = 1;
                break;
            }
        }
        if (!ensured) place_random_item((rand() % 2 == 0) ? 'C' : '?');
    }

    // level ≥ 5: S와 D 구조 무조건 추가
    if (level >= 5 && !has_S) {
        place_switch_near_center();
        has_S = 1;
    }
    if (has_S) {
        place_door_near_exit();
    }

    // level ≥ 7: 'T' (+10초) 아이템 랜덤 삽입
    if (level >= 7 && rand() % 2 == 0) {
        place_random_item('T');
    }
}


void process_player_move(int key, bool *level_complete_ptr) {
    int nx = px, ny = py;
    if (key == KEY_UP)    { ny--; last_dir_y = -1; last_dir_x = 0; }
    if (key == KEY_DOWN)  { ny++; last_dir_y = 1; last_dir_x = 0; }
    if (key == KEY_LEFT)  { nx--; last_dir_y = 0; last_dir_x = -1; }
    if (key == KEY_RIGHT) { nx++; last_dir_y = 0; last_dir_x = 1; }

    if (nx >= 0 && nx < MAZE_WIDTH && ny >= 0 && ny < MAZE_HEIGHT) {
        char next = maze[ny][nx];
        if (next == '#') return;
        if (next == 'D') return;
        if (next == 'C') { collapse_tile(ny, nx); return; }
        if (next == 'S') activate_switch();
        if (next == 'K') pickup_key(ny, nx);
        if (next == 'B') pickup_breaker(ny, nx);
        if (next == 'M') pickup_push_item(ny, nx);
        if (next == '?') {
            maze[ny][nx] = ' ';
            apply_random_box_effect();
        }
        if (next == 'P' && (player_has_key || player_style == STYLE_HACKER)) {
            mvprintw(center_y_offset + MAZE_HEIGHT + 5, center_x_offset + 4,
                    "You cleared Level %d! Press any key...", level);
            refresh(); getch();

            // 생명 손실을 반영한 시간 보너스 계산
            int initial_lifes = 3;
            int total_time_per_life = 120;
            int time_used = total_time_per_life * (initial_lifes - lifes) + (total_time_per_life - time_left);
            int total_possible_time = total_time_per_life * initial_lifes;
            int time_saved = total_possible_time - time_used;
            if (time_saved < 0) time_saved = 0;

            int bonus_multiplier = 2 + level / 5;
            int bonus = time_saved * bonus_multiplier;
            score += 100 + bonus;  // 기본 점수 + 보너스
            lifes = 3;
            level++;

            mvprintw(center_y_offset + MAZE_HEIGHT + 6, center_x_offset + 4,
                    "Bonus for speed: +%d", bonus);
            refresh(); getch();

            *level_complete_ptr = true;
            return;
        }
        if (next == 'T') {
            time_left += 10;
            maze[ny][nx] = ' ';
            mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "You gained +10 seconds!");
        }
        px = nx;
        py = ny;
    }
}

void start_maze_game() {
    choose_player_style(); 
    apply_player_style_effects();
    if (player_speed_is) {
        player_speed = 1;
        player_speed += 9;
        player_speed_is = false;
    } else player_speed = 1;

    if (player_style == STYLE_HACKER) {
        player_has_key = true;
    }

    draw_maze();
    while (1) {
        getmaxyx(stdscr, g_height, g_width);
        center_y_offset = (g_height - MAZE_HEIGHT) / 2 - 1;
        center_x_offset = (g_width - MAZE_WIDTH * 2) / 2 - 1;

        srand(time(NULL) + level);
        generate_maze_with_items();
        level_start_time = time(NULL);
        int time_var = 120 - (level - 1) * 5;
            if (time_var < 60) time_var = 60;
        time_left = time_var;

        if (player_style == STYLE_HACKER) {
            for (int y = 0; y < MAZE_HEIGHT; y++) {
                for (int x = 0; x < MAZE_WIDTH; x++) {
                    if (maze[y][x] == 'K') maze[y][x] = ' ';
                }
            }
        }

        init_enemies();
        timeout(100);

        last_dir_y = 0; last_dir_x = 1;
        bool level_complete = false;

        ghost_mode = false;
        last_z_use = 0;

        while (!level_complete) {
            erase(); draw_maze(); refresh();

            int key;
            for (int speed_step = 0; speed_step < player_speed; speed_step++) {
                key = getch();
                if (key != ERR) {
                    process_player_move(key, &level_complete);
                    break;
                }
                usleep(30000 / player_speed);
            }

            // 🎯 키 입력 처리
            if (key == '\t') { wait_screen(); continue; }
            if (key == 'q') return;

            // 벽 부수기
            if (key == 'w' && player_has_breaker) {
                int dx[4] = {0, 0, -1, 1}, dy[4] = {-1, 1, 0, 0};
                for (int d = 0; d < 4; d++) {
                    int ax = px + dx[d], ay = py + dy[d];
                    if (ay >= 0 && ay < MAZE_HEIGHT && ax >= 0 && ax < MAZE_WIDTH && maze[ay][ax] == '#')
                        maze[ay][ax] = ' ';
                }
                player_has_breaker = false;
                continue;
            }

            // Freeze (Z)
            if (key == 'z' && time(NULL) - last_z_use >= Z_COOLDOWN) {
                int tx = px + last_dir_x, ty = py + last_dir_y;
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (enemies[i].alive && enemies[i].x == tx && enemies[i].y == ty) {
                        enemies[i].type = TYPE_FROZEN;
                        ghost_mode = true;
                        last_z_use = time(NULL);
                        z_usage_count++;
                        break;
                    }
                }
            }

            // Push (X)
            if ((key == 'z' || key == 'x') && has_push_item) {
                int tx = px + last_dir_x, ty = py + last_dir_y;
                int tx2 = px + last_dir_x * 2, ty2 = py + last_dir_y * 2;
                if (maze[ty][tx] == '#' && maze[ty2][tx2] == ' ') {
                    maze[ty2][tx2] = '#';
                    maze[ty][tx] = ' ';
                    has_push_item = false;
                    mvprintw(center_y_offset + MAZE_HEIGHT + 4, center_x_offset + 4, "Pushed wall forward!");
                }
            }

            // 적 충돌 (보스 + 일반 적 포함)
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (!enemies[i].alive || enemies[i].type == TYPE_FROZEN) continue;

                if (px == enemies[i].x && py == enemies[i].y) {
                    int damage = (enemies[i].type == TYPE_BOSS) ? 2 : 1;
                    lifes -= damage;

                    mvprintw(center_y_offset + MAZE_HEIGHT + 5, center_x_offset + 4,
                            (damage == 2) ? "Boss hit you! -2 life!" : "You were hit! Lifes left: %d", lifes);
                    refresh(); getch();

                    if (lifes <= 0) {
                        if (ask_retry()) {
                            lifes = 3; px = 1; py = 1;
                            destroy_enemies(); usleep(100000); init_enemies(); continue;
                        } else {
                            lifes = 3; destroy_enemies(); return;
                        }
                    }

                    px = 1; py = 1;
                    break;
                }
            }

            // 보스 레벨일 경우 생존 후 포탈 오픈
            if (level % 5 == 0 && time(NULL) - level_start_time >= 20) {
                maze[MAZE_HEIGHT - 2][MAZE_WIDTH - 2] = 'P';
            }

            // 시간 체크
            time_left = time_var - (time(NULL) - level_start_time);
            if (time_left <= 0) {
                lifes--;
                mvprintw(center_y_offset + MAZE_HEIGHT + 5, center_x_offset + 4, "Time's up! You lost 1 life.");
                refresh(); getch();

                if (lifes <= 0) {
                    if (ask_retry()) {
                        lifes = 3; px = 1; py = 1;
                        destroy_enemies(); usleep(100000); init_enemies(); continue;
                    } else {
                        lifes = 3; destroy_enemies(); return;
                    }
                }

                level_start_time = time(NULL); // 타이머 재시작
                continue;
            }
        }

        destroy_enemies();
    }
}
