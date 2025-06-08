#include "astar.h"
#include "game.h" // maze, MAZE_WIDTH, MAZE_HEIGHT
#include <stdlib.h>
#include <string.h>

#define ABS(x) ((x) < 0 ? -(x) : (x))

// 휴리스틱 계산 (맨해튼 거리)
static int heuristic(int y1, int x1, int y2, int x2) {
    return ABS(y1 - y2) + ABS(x1 - x2);
}

static int find_lowest_f_index(AStarNode *list, int count) {
    if (count <= 0) return -1;
    int best = 0;
    for (int i = 1; i < count; i++) {
        if (list[i].f < list[best].f)
            best = i;
    }
    return best;
}

static int node_in_list(AStarNode *list, int count, int y, int x) {
    for (int i = 0; i < count; i++) {
        if (list[i].y == y && list[i].x == x)
            return i;
    }
    return -1;
}

int astar_find_path(int startY, int startX, int goalY, int goalX, int path[][2], int *pathLen) {
    AStarNode *open = malloc(sizeof(AStarNode) * MAX_OPEN_LIST);
    AStarNode *closed = malloc(sizeof(AStarNode) * MAX_CLOSED_LIST);
    if (!open || !closed) {
        free(open);
        free(closed);
        return 0;
    }

    int openCount = 0, closedCount = 0;
    AStarNode start = {startX, startY, 0, heuristic(startY, startX, goalY, goalX), 0, -1, -1};
    start.f = start.g + start.h;
    open[openCount++] = start;

    int dirs[4][2] = {{0,1},{1,0},{0,-1},{-1,0}};

    while (openCount > 0) {
        int currentIndex = find_lowest_f_index(open, openCount);
        if (currentIndex == -1) break;

        AStarNode current = open[currentIndex];

        if (current.y == goalY && current.x == goalX) {
            int cx = current.x, cy = current.y, clen = 0;

            while (current.parentX != -1 && current.parentY != -1) {
                if (clen >= 100) break;  // path 범위 초과 방지

                path[clen][0] = cy;
                path[clen][1] = cx;
                clen++;

                int found = node_in_list(closed, closedCount, current.parentY, current.parentX);
                if (found >= 0) {
                    current = closed[found];
                    cx = current.x;
                    cy = current.y;
                } else break;
            }

            if (clen < 100) {  // 마지막 시작 지점 넣기
                path[clen][0] = startY;
                path[clen][1] = startX;
                clen++;
            }

            // 경로 역순 정렬 (clen 안에서만)
            for (int i = 0; i < clen / 2; i++) {
                int tmpy = path[i][0], tmpx = path[i][1];
                path[i][0] = path[clen - 1 - i][0];
                path[i][1] = path[clen - 1 - i][1];
                path[clen - 1 - i][0] = tmpy;
                path[clen - 1 - i][1] = tmpx;
            }

            *pathLen = clen;
            free(open);
            free(closed);
            return 1;
        }

        for (int i = currentIndex; i < openCount - 1; i++)
            open[i] = open[i + 1];
        openCount--;

        closed[closedCount++] = current;

        for (int d = 0; d < 4; d++) {
            int ny = current.y + dirs[d][0];
            int nx = current.x + dirs[d][1];
            if (ny < 0 || ny >= MAZE_HEIGHT || nx < 0 || nx >= MAZE_WIDTH) continue;
            if (maze[ny][nx] != ' ') continue;
            if (node_in_list(closed, closedCount, ny, nx) != -1) continue;

            int g = current.g + 1;
            int h = heuristic(ny, nx, goalY, goalX);
            int f = g + h;

            int idx = node_in_list(open, openCount, ny, nx);
            if (idx == -1) {
                if (openCount < MAX_OPEN_LIST) {
                    AStarNode neighbor = {nx, ny, g, h, f, current.x, current.y};
                    open[openCount++] = neighbor;
                }
            } else if (g < open[idx].g) {
                open[idx].g = g;
                open[idx].f = f;
                open[idx].parentX = current.x;
                open[idx].parentY = current.y;
            }
        }
    }

    free(open);
    free(closed);
    return 0;
}
