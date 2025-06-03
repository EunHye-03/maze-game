#ifndef GAME_H
#define GAME_H

#define COLOR_HUD 1
#define SAVE_DIR "assets/save"

// 전역 변수
extern int g_width;
extern int g_height;
extern char g_username[32];
extern short level;
extern short score;
extern short lifes;

#define MAZE_WIDTH 41
#define MAZE_HEIGHT 23

void start_maze_game();
void generate_maze(int y, int x);
void draw_maze();

extern char maze[MAZE_HEIGHT][MAZE_WIDTH];
extern int player_has_key;

// 게임 상태 열거형
typedef enum {
    STATE_MENU,
    STATE_GAME,
    STATE_SETTINGS,
    STATE_INFORMATION,
    STATE_EXIT
} GameState;

// 구조체 정의
typedef struct {
    int x, y;
} Point;

typedef struct {
    char symbol[20];
    int x, y;
} GameObject;

#endif
