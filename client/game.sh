#!/bin/bash

# 항상 스크립트 위치 기준으로 작동하도록 설정
cd "$(dirname "$0")"

echo "[*] Building game..."
mkdir -p bin
make clean
make

echo "[*] Running game..."
./bin/maze_game
