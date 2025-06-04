#ifndef SAVE_H
#define SAVE_H

#include <stdbool.h>

bool save_game(const char *filename);
int load_game();  // 이건 최신 세이브 자동 불러오기 (원하면 삭제 가능)

void list_save_files(char filenames[][256], int* count);  // 세이브 파일 목록 불러오기
int load_specific_game(const char* filename);             // 특정 세이브 파일 불러오기

#endif // SAVE_H
