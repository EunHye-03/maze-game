#ifndef RENDER_H
#define RENDER_H

void init_colors();
void check_terminal_size();
void input_username_screen();
void intro_animation();
void draw_logo();
void draw_menu(int selected);
void draw_game_info_menu(int selected);
void draw_maze(int px, int py);
void animate_character_through_maze();

#endif
