/*
    ####--------------------------------####
    #--# Author:   by hyunsu, eunhye    #--#
    #--# License:  GNU GPLv3            #--#
    #--# Telegram: @main_moderator      #--#
    #--# E-mail:   zmfnwj119@gmail.com  #--#
    ####--------------------------------####
*/

// Includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ncurses.h>
#include <time.h>
#include <stdbool.h>

// Constants
// Const define
#define ROW 20 // Maze rows
#define COL 27 // Maze columns
#define true 1
#define false 0

// Const state, key kode
#define vk_space  32
#define vk_endter 10
bool EXIT = false;
int key_pressed = 0;

// Const colors
#define c_wall   1
#define c_star   2
#define c_space  3
#define c_plus   4
#define c_minus  5
#define c_player 6
#define c_enemy  7
#define c_hud    1

// Directions (up, down, left, right)
int dx[] = {-1, 1, 0, 0};
int dy[] = {0, 0, -1, 1};

// Global Variables
short maze[ROW][COL];       // Maze map
bool visited[ROW][COL];     // Visited check array
short level = 1;            // Current level
short score = 0;            // Player score
short lifes = 3;            // Player lives
int star_in_level = 0;      // Stars in the level
int current_lvl_x, current_lvl_y; // Level size
int w, h;                   // Window width and height

////////////////////
// Utility Functions
////////////////////

// Shuffle directions randomly
void shuffle(int arr[], int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// Get string length
int str_len(const char* str) {
    int size = 0;
    while (*str++) ++size;
    return size;
}

void SetColor() {
    start_color();
    init_pair(c_wall,   COLOR_WHITE,     COLOR_BLACK);
    init_pair(c_star,   COLOR_YELLOW,    COLOR_BLACK);
    init_pair(c_space,  COLOR_RED,       COLOR_BLACK);   
    init_pair(c_plus,   COLOR_BLUE,      COLOR_BLACK);
    init_pair(c_minus,  COLOR_GREEN,     COLOR_BLACK);
    init_pair(c_player, COLOR_MAGENTA,   COLOR_BLACK);
    init_pair(c_enemy,  COLOR_RED,       COLOR_BLACK);
    init_pair(c_hud,    COLOR_BLUE,     COLOR_BLACK); // HUD color
}

////////////////////
// Maze Functions
////////////////////

// Generate maze using BFS
void generate_maze(int start_x, int start_y) {
    int queue[ROW * COL][2];
    int front = 0, rear = 0;

    // Add starting point to the queue
    queue[rear][0] = start_x;
    queue[rear][1] = start_y;
    rear++;
    visited[start_x][start_y] = true;
    maze[start_x][start_y] = 0; // Set as a path

    while (front < rear) {
        int x = queue[front][0];
        int y = queue[front][1];
        front++;

        // Randomize directions
        int directions[] = {0, 1, 2, 3};
        shuffle(directions, 4);

        for (int i = 0; i < 4; i++) {
            int nx = x + dx[directions[i]];
            int ny = y + dy[directions[i]];

            // Check if within maze bounds and not visited
            if (nx > 0 && nx < ROW - 1 && ny > 0 && ny < COL - 1 && !visited[nx][ny]) {
                // Skip 2 cells and create a path
                int wall_x = x + dx[directions[i]] / 2;
                int wall_y = y + dy[directions[i]] / 2;

                visited[nx][ny] = true;
                maze[nx][ny] = 0; // Create path
                maze[wall_x][wall_y] = 0; // Remove wall

                queue[rear][0] = nx;
                queue[rear][1] = ny;
                rear++;
            }
        }
    }
}

// Print the maze
void print_maze() {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            if (maze[i][j] == 1) {
                mvprintw(i + 10, j, "#"); // Wall
            } else {
                mvprintw(i + 10, j, " "); // Path
            }
        }
    }
}

////////////////////
// Menu Functions
////////////////////

const char *menu_logo[5] = {
    " # #       # #           #          # # # # # #   # # # # #",
    " #   #   #   #         #   #                #     #",
    " #     #     #      #    #    #           #       # # # # #",
    " #           #    #            #        #         #",
    " #           #  #                #  # # # # # #   # # # # #",
};

// Get logo size
int logo_h_size = sizeof(menu_logo)/sizeof(menu_logo[0]);

// Get logo width size
int get_logo_w_size(void) {
    int logo_w_size = 1;

    for (int i = 0; i < logo_h_size; i++) {
        int len = str_len(menu_logo[i]);
        if (len > logo_w_size) {
            logo_w_size = len;
        }
    }
    return logo_w_size;
}

// Draw the logo

int logo_w_size = 1;
void draw_logo(int h, int w) {  
    // Get w size
    if (logo_w_size == 1) {
        logo_w_size = get_logo_w_size() / 2;
    }

    // Draw
    attron(COLOR_PAIR(c_hud));
    for (int i = 0; i < logo_h_size; i++) {
        mvprintw(3 + i /* Logo Y pos */, w / 2 - logo_w_size, "%s", menu_logo[i]);
    }
    attroff(COLOR_PAIR(c_hud));
}

////////////////////
// Main Function
////////////////////

int main() {

    // Start curses mode
    initscr();
    keypad(stdscr, TRUE);
    savetty();
    cbreak();
    noecho();
    timeout(0);
    leaveok(stdscr, TRUE);
    start_color();
    curs_set(0);
    init_pair(c_hud, COLOR_WHITE, COLOR_BLACK); // Initialize color pair

    srand(time(NULL));

    ////////////////////
    // Enum game state
    ///////////////////
    typedef enum {
        STATE_MENU,
        STATE_INFO,
        STATE_GAME,
        STATE_EXIT,
    } game_states;

    // Init current state
    game_states current_state;
    current_state = STATE_MENU;


    //////////////
    // init obj
    //////////////

    // Initialize: Set all cells as walls
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            maze[i][j] = 1;
            visited[i][j] = false;
        }
    }

    ////////////////
    // Main loop
    ///////////////

    // Menu item
    // Item start game
    const char *item_start_game[2] = {
        "> START GAME <",
        "start game",
    };

    // Item info
    const char *item_info[2] = {
        "> INFO <",
        "info",
    };

    // Item exit
    const char *item_exit[2] = {
        "> EXIT <",
        "exit",
    };

    while (!EXIT) {

        // Color
        SetColor();

        // Get window width & Height
        getmaxyx(stdscr, h, w);

        // Menu state
        static int menu_item = 0;
        if (key_pressed == KEY_UP)   menu_item--;
        if (key_pressed == KEY_DOWN) menu_item++;

        if (menu_item >= 2) menu_item = 2;
        if (menu_item <= 0) menu_item = 0;

        // In menu state
        switch(current_state) {
            // Menu
            case STATE_MENU:
                // Logo
                draw_logo(h, w);

                ///////////
                // Items
                //////////
                // Item start game
                int select_start_game = menu_item == 0 ? 0 : 1;
                mvprintw(h/2 - logo_h_size + 9, w/2 - str_len(item_start_game[select_start_game])/2, "%s", item_start_game[select_start_game]);

                // Item info
                int select_info = menu_item == 1 ? 0 : 1;
                mvprintw(h/2 - logo_h_size + 11, w/2 - str_len(item_info[select_info])/2, "%s", item_info[select_info]);

                // Item exit
                int select_exit = menu_item == 2 ? 0 : 1;
                mvprintw(h/2 - logo_h_size + 13, w/2 - str_len(item_exit[select_exit])/2, "%s", item_exit[select_exit]);

                // By dev
                mvprintw(h-2, 2, "%s", "Develop: hyunsu, eunhye");

                // Draw box
                attron(COLOR_PAIR(c_hud));
                box(stdscr, 0, 0);
                attron(COLOR_PAIR(c_hud));

                // Click handler
                if (key_pressed == vk_endter) {
                    switch(menu_item) {
                        case 0:
                            current_state = STATE_GAME;
                        break;
                        
                        case 1:
                            // Info page is dev
                            current_state = STATE_INFO;
                        break;

                        case 2:
                            current_state = STATE_EXIT;
                        break;
                    }
                }
            break;

            // Info
            case STATE_INFO:
                static int len_xoff = 31;
                static int len_yoff = 2;
                mvprintw(h/2-len_yoff,   w/2-len_xoff, "%s", "This is a small game written in C.");
                mvprintw(h/2-len_yoff+1, w/2-len_xoff, "%s", "Your task is to collect all the apples while avoiding enemies.");
                mvprintw(h/2-len_yoff+2, w/2-len_xoff, "%s", "I wrote this game just for fun :)");
                mvprintw(h/2-len_yoff+3, w/2-len_xoff, "%s", "I do not recommend using the source code for learning C.");
                mvprintw(h/2-len_yoff+4, w/2-len_xoff, "%s", "Have a good game!");

                // To menu
                mvprintw(h-4, w/2-ceil(len_xoff/2), "%s", "press 'q' to exit menu");

                // By dev
                mvprintw(h-2, 2, "%s", "Develop: uriid1");

                box(stdscr, 0, 0);

                // Exit to menu
                if (key_pressed == 'q') {
                    current_state = STATE_MENU;
                    erase();
                }
            break;

            // Game
            case STATE_GAME:
                // level_init(level);
                // draw_hud();
                box(stdscr, 0, 0);
            break;

            // Exit
            case STATE_EXIT:
                endwin();
                EXIT = TRUE;
            break;
        }

        // Get key pressed
        key_pressed = wgetch(stdscr);

        // Clear
        erase();
    }

    // End curses mode
    endwin();

    return 0;
}