
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <pwd.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "game.h"

#define SAVE_DIR "assets/save"
#define SAVE_FILE "autosave.txt"

#define true 1
#define false 0

typedef struct {
    char username[32];
    short level;
    short score;
    short lifes;
} SaveData;

bool save_game() {
    // 저장 디렉토리가 없다면 생성
    struct stat st = {0};
    if (stat(SAVE_DIR, &st) == -1) {
        mkdir(SAVE_DIR, 0700);
    }

    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", SAVE_DIR, SAVE_FILE);

    FILE *fp = fopen(filepath, "w");
    if (!fp) return false;

    fprintf(fp, "%s %d %d %d\n", g_username, level, score, lifes);
    fclose(fp);
    return true;
}

int load_game() {
    DIR *dp = opendir("assets/save");
    if (!dp) return 0;

    struct dirent *entry;
    char latest_path[512] = {0};
    time_t latest_time = 0;

    while ((entry = readdir(dp)) != NULL) {
        char path[512];

#ifdef DT_REG
        if (entry->d_type != DT_REG) continue;
#else
        struct stat st_check;
        snprintf(path, sizeof(path), "assets/save/%s", entry->d_name);
        if (stat(path, &st_check) == -1 || !S_ISREG(st_check.st_mode)) continue;
#endif

        // 전체 경로 만들기
        snprintf(path, sizeof(path), "assets/save/%s", entry->d_name);

        struct stat st;
        if (stat(path, &st) == 0) {
            if (st.st_mtime > latest_time) {
                latest_time = st.st_mtime;
                strncpy(latest_path, path, sizeof(latest_path) - 1);
            }
        }
    }

    closedir(dp);

    // 가장 최신 파일이 있으면 로드
    if (strlen(latest_path) > 0) {
        FILE *fp = fopen(latest_path, "r");
        if (fp) {
            char name[32];
            int lvl, scr, life;
            if (fscanf(fp, "%31s %d %d %d", name, &lvl, &scr, &life) == 4) {
                strncpy(g_username, name, sizeof(g_username) - 1);
                level = lvl;
                score = scr;
                lifes = life;
                fclose(fp);
                return 1;
            }
            fclose(fp);
        }
    }

    return 0;
}