#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>

bool save_game(const char *filename);
int load_game();  // 이건 최신 세이브 자동 불러오기 (원하면 삭제 가능)

void list_save_files(char filenames[][256], int* count);  // 세이브 파일 목록 불러오기
int load_specific_game(const char* filename);             // 특정 세이브 파일 불러오기
const char* get_save_progress(const char* save_name);
int get_saved_level(const char* name);
int get_saved_score(const char* name);
const char* get_save_progress(const char* name);
void clear_all_save_files();;
const char* get_saved_username(const char* save_name);

#endif // SAVE_H
