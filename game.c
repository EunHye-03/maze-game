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
#define bool int
#define true 1
#define false 0

// Key Kode
#define vk_space  32
#define vk_endter 10
int key_pressed = 0;

// Const Color
const short c_wall   = 1;
const short c_star   = 2;
const short c_space  = 3;
const short c_plus   = 4;
const short c_minus  = 5;
const short c_player = 6;
const short c_enemy  = 7;

// Const state
bool EXIT = false;

// Game Global var
short level = 1;
short score = 0;
short lifes = 3;

int star_in_level = 0;

// Level size
int current_lvl_x;
int current_lvl_y;

// Window width & Height
int w, h;

// index map object
#define i_wall  1
#define i_star  2
#define i_space 3
#define i_plus  4
#define i_minus 5
#define i_exit  6
#define i_enemy_v 7
#define i_enemy_h 8