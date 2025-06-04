#ifndef ENEMY_H
#define ENEMY_H

#include <pthread.h>

// 적의 성격 유형 정의
typedef enum {
    ENEMY_AGGRESSIVE,   // 무조건 플레이어 추적
    ENEMY_STALKER,      // 2칸 이상 가까워지면 추적
    ENEMY_RANDOM,       // 무작위 이동
    ENEMY_COWARD,       // 일정 거리 이내면 도망
    TYPE_FROZEN,        // 적 멈추기
} EnemyType;

// 적 구조체 정의
typedef struct {
    int x, y;                 // 현재 좌표
    EnemyType type;          // 성격
    pthread_t thread;        // 개별 쓰레드
    int hp;                  // 보스만 사용
    int alive;               // 활성화 여부
} Enemy;

// 최대 적 수
#define MAX_ENEMIES 12

// 외부에서 접근 가능한 적 배열
extern Enemy enemies[MAX_ENEMIES];

// 함수 선언
void init_enemies();
void destroy_enemies();
void *enemy_thread(void *arg);
int ask_retry();

// 보스
#define TYPE_BOSS 99

#endif
