#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "game.h"

#define SAVE_DIR "assets/save"

#define true 1
#define false 0

typedef struct {
    char username[32];
    int level;
    int score;
    int lifes;
} SaveData;

bool save_game(const char* filename) {
    struct stat st;
    if (stat("assets", &st) == -1) {
        if (mkdir("assets", 0700) == -1) {
            perror("mkdir assets failed");
            return false;
        }
    }

    if (stat("assets/save", &st) == -1) {
        if (mkdir("assets/save", 0700) == -1) {
            perror("mkdir save failed");
            return false;
        }
    }

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("open failed");
        return false;
    }

    SaveData data;
    strncpy(data.username, g_username, sizeof(data.username));
    data.username[sizeof(data.username) - 1] = '\0';
    data.level = level;
    data.score = score;
    data.lifes = lifes;

    if (write(fd, &data, sizeof(SaveData)) != sizeof(SaveData)) {
        perror("write failed");
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

int load_specific_game(const char* filename) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%s", SAVE_DIR, filename);

    int fd = open(filepath, O_RDONLY);
    if (fd == -1) return 0;

    SaveData data;
    if (read(fd, &data, sizeof(SaveData)) != sizeof(SaveData)) {
        close(fd);
        return 0;
    }

    strncpy(g_username, data.username, sizeof(g_username));
    level = data.level;
    score = data.score;
    lifes = data.lifes;

    close(fd);
    return 1;
}

const char* get_saved_username(const char* save_name) {
    static char namebuf[32];
    char path[512];
    snprintf(path, sizeof(path), "assets/save/%s.txt", save_name);

    int fd = open(path, O_RDONLY);
    if (fd == -1) return "";

    SaveData data;
    if (read(fd, &data, sizeof(SaveData)) != sizeof(SaveData)) {
        close(fd);
        return "";
    }
    close(fd);
    strncpy(namebuf, data.username, sizeof(namebuf) - 1);
    namebuf[sizeof(namebuf) - 1] = '\0';
    return namebuf;
}


int get_saved_level(const char* save_name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", SAVE_DIR, save_name);

    int fd = open(path, O_RDONLY);
    if (fd == -1) return 0;

    SaveData data;
    if (read(fd, &data, sizeof(SaveData)) != sizeof(SaveData)) {
        close(fd);
        return 0;
    }

    close(fd);
    return data.level;
}

int get_saved_score(const char* save_name) {
    char path[512];
    snprintf(path, sizeof(path), "%s/%s", SAVE_DIR, save_name);

    int fd = open(path, O_RDONLY);
    if (fd == -1) return 0;

    SaveData data;
    if (read(fd, &data, sizeof(SaveData)) != sizeof(SaveData)) {
        close(fd);
        return 0;
    }

    close(fd);
    return data.score;
}

const char* get_save_progress(const char* save_name) {
    static char buffer[64];
    char path[512];
    snprintf(path, sizeof(path), "assets/save/%s.txt", save_name);

    int fd = open(path, O_RDONLY);
    if (fd == -1) return "0 | 0";

    SaveData data;
    if (read(fd, &data, sizeof(SaveData)) != sizeof(SaveData)) {
        close(fd);
        return "0 | 0";
    }
    close(fd);

    snprintf(buffer, sizeof(buffer), "%d | %d", data.level, data.score);
    return buffer;
}

void clear_all_save_files(void) {
    DIR* dir = opendir(SAVE_DIR);
    if (!dir) return;

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strstr(entry->d_name, ".txt") || strstr(entry->d_name, ".dat")) {
            char path[512];
            snprintf(path, sizeof(path), "%s/%s", SAVE_DIR, entry->d_name);
            unlink(path);
        }
    }
    closedir(dir);
}

void list_save_files(char filenames[][256], int* count) {
    *count = 0;
    DIR* dp = opendir(SAVE_DIR);
    if (!dp) return;

    struct dirent* entry;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char fullpath[512];
        snprintf(fullpath, sizeof(fullpath), "%s/%s", SAVE_DIR, entry->d_name);

        struct stat st;
        if (stat(fullpath, &st) == 0 && S_ISREG(st.st_mode)) {
            if (strstr(entry->d_name, ".dat") || strstr(entry->d_name, ".txt")) {
                strncpy(filenames[*count], entry->d_name, 255);
                filenames[*count][255] = '\0';
                (*count)++;
            }
        }
    }
    closedir(dp);
}

int load_game() {
    DIR* dp = opendir(SAVE_DIR);
    if (!dp) {
        perror("opendir failed");
        return 0;
    }

    struct dirent* entry;
    char latest_path[512] = {0};
    time_t latest_time = 0;

    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') continue;

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", SAVE_DIR, entry->d_name);

        struct stat st;
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
            // 가장 최근 파일 선택
            if (st.st_mtime > latest_time) {
                latest_time = st.st_mtime;
                strncpy(latest_path, path, sizeof(latest_path) - 1);
            }
        }
    }
    closedir(dp);

    if (strlen(latest_path) > 0) {
        int fd = open(latest_path, O_RDONLY);
        if (fd == -1) {
            perror("open failed");
            return 0;
        }

        SaveData data;
        ssize_t bytes_read = read(fd, &data, sizeof(SaveData));
        if (bytes_read != sizeof(SaveData)) {
            perror("read failed");
            close(fd);
            return 0;
        }

        close(fd);

        // 값 반영
        strncpy(g_username, data.username, sizeof(g_username));
        g_username[sizeof(g_username) - 1] = '\0';
        level = data.level;
        score = data.score;
        lifes = data.lifes;

        return 1;
    }

    return 0; // 저장 파일이 아예 없을 때
}
