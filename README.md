# Maze Game

## 제출물 실행 및 사용방법

### 1. Windows 10, Windows 11

C 기반의 터미널 게임으로 ncurses 및 pthread 라이브러리를 이용한 미로 탈출 게임입니다. 아래는 실행 방법입니다.

#### 실행 환경 준비

* MinGW, WSL 등 POSIX 환경을 제공하는 터미널
* gcc 컴파일러
* `ncurses`, `pthread` 등 필요 라이브러리 설치 필요

#### 실행 방법

1. `client/` 폴더로 이동 후, `game.sh` 실행

```bash
./game.sh
```

2. `Makefile`을 통해 프로젝트를 컴파일 및 실행

```bash
make clean
make
./bin/maze_game game.sh
```

### 2. Ubuntu 22.04.1 LTS

#### 실행 환경 준비

* gcc
* libncurses-dev
* pthread

```bash
sudo apt update
sudo apt install gcc libncurses-dev
```

#### 실행

```bash
cd client/
chmod +x game.sh
./game.sh
```

## 프로그램 개요

플레이어가 다양한 성격을 지닌 적들로부터 도망치며, DFS 기반으로 생성된 미로를 탈출하는 게임입니다. 플레이어는 레벨이 오를수록 난이도가 증가하고, 다양한 아이템과 특성을 통해 플레이 스타일을 변화시킬 수 있습니다.

## 기능 설명

### 1. 미로 생성

* DFS (깊이 우선 탐색) 기반 미로 생성 알고리즘 사용
* 벽과 통로를 격자 형태로 구성
* 무작위 방향 선택 및 방문 확인을 통해 완전한 미로 형성

### 2. 적 이동 알고리즘

* 적은 다양한 유형으로 구성됨: 공격형, 랜덤형, 스토커형, 소심형
* A\* 알고리즘을 사용하여 플레이어를 추적하는 공격형 적 구현
* 플레이어가 가만히 있어도 적은 독립적으로 쓰레드를 통해 움직임
* `pthread`를 활용해 각 적을 별도 스레드로 실행

### 3. 아이템 및 특성 시스템

* 스타일에 따라 초기 능력치 및 보너스 아이템 차등 지급
* 스테이지별로 다양한 아이템 출현

### 4. 저장/불러오기 기능

* 게임 중간 저장 및 불러오기 기능 구현
* `assets/save/` 폴더 내에 저장 파일 유지
* 유저 선택에 따라 다양한 save 파일 불러오기 가능

### 5. 사용자 인터페이스

* ncurses 기반 터미널 UI
* 실시간 타이머, 적 위치, 플레이어 상태, 남은 생명, 포탈 및 열쇠 정보 출력
* WASD 및 방향키 기반 조작

### 6. 난이도 및 보스 시스템

* 레벨 증가 시 적 수 및 속도 증가
* 특정 레벨에서 보스 등장
* 보스는 고유의 움직임 패턴 및 강화된 추적 능력을 가짐
* 보스 스테이지에서는 특수 아이템 지급 및 클리어 조건 강화

## 개발환경

* OS: Windows 10, Ubuntu 22.04.1 LTS
* Language: C
* Library: ncurses, pthread
* Build Tool: Makefile

## 실행 시 유의사항

* 플레이어 이름은 영어만 사용 가능
* 저장 파일명은 중복되지 않아야 하며, 특수문자는 피해야 함
* 게임 데이터는 `assets/save` 폴더에 저장됨

## 핵심 코드 구현 위치

* `src/game.c(generate_maze(int, int))`: DFS 기반 미로 생성 알고리즘
* `src/enemy.c`: A\* 기반 적 추적 알고리즘
* `src/game.c`: 전체 게임 실행 흐름
* `src/player.c`: 플레이어 입력 및 움직임 처리
* `src/save.c`: 저장 및 불러오기 기능

---

