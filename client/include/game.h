#ifndef GAME_H
#define GAME_H

#define COLOR_HUD 1
#define SAVE_DIR "assets/save"
#include <stdbool.h>

// 전역 변수
extern int g_width;
extern int g_height;
extern char g_username[32];
extern short level;
extern short score;
extern short lifes;

extern int px, py;
extern int player_has_key;
extern int player_has_breaker;
extern int player_speed;

extern bool g_return_to_menu;

#define MAZE_WIDTH 45
#define MAZE_HEIGHT 29

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

typedef enum {
    STYLE_SPEEDSTER,   // 속도 +1
    STYLE_TANK,        // 생명 +1
    STYLE_HACKER,      // 열쇠 없이 포탈 가능
    STYLE_BREAKER,     // 시작 시 벽 부수기 아이템 보유
    STYLE_TRICKSTER,   // 미스터리 박스 효과 향상
    STYLE_COUNT
} PlayerStyle;

#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

#endif
