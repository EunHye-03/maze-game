#ifndef ASTAR_H
#define ASTAR_H

// 최대 미로 크기 및 경로 길이 제한
#define MAX_PATH_LEN 256
#define MAX_OPEN_LIST 1024
#define MAX_CLOSED_LIST 1024

// 경로 노드 구조체
typedef struct {
    int x, y;            // 좌표
    int g;               // 시작점으로부터의 비용
    int h;               // 휴리스틱 (목표까지 예상 비용)
    int f;               // 총 비용 = g + h
    int parentX, parentY;// 부모 노드 (경로 재구성용)
} AStarNode;

/**
 * A* 경로를 계산하여 path[]에 [y, x] 형태로 저장
 * 
 * @param startY 시작 y좌표
 * @param startX 시작 x좌표
 * @param goalY 목표 y좌표
 * @param goalX 목표 x좌표
 * @param path 결과 경로 (path[i][0] = y, path[i][1] = x)
 * @param pathLen path 배열에 들어갈 실제 경로 길이 반환
 * @return 경로 찾았으면 1, 못 찾으면 0
 */
int astar_find_path(int startY, int startX, int goalY, int goalX, int path[][2], int *pathLen);

#endif
