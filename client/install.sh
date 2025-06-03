#!/bin/bash

set -e  # 오류 발생 시 즉시 중단

echo "🔧 Maze Game Installer"

# 1. ncurses 설치 (Ubuntu/Debian)
echo "📦 Checking for ncurses..."
if ! dpkg -s libncurses5-dev &>/dev/null; then
    echo "📥 Installing ncurses..."
    sudo apt update
    sudo apt install -y libncurses5-dev
else
    echo "✅ ncurses already installed."
fi

# 2. 프로젝트 구조 확인
if [[ ! -f Makefile ]]; then
    echo "❌ Makefile not found! Run from the project root directory."
    exit 1
fi

# 3. bin 디렉토리 생성
mkdir -p bin assets/save

# 4. 빌드
echo "🔨 Building the game..."
make

# 5. 실행 스크립트 권한 부여
chmod +x game.sh

# 6. 실행 제안
echo -e "\n✅ Installation complete!"
echo "👉 To play the game, run:"
echo "./game.sh"
