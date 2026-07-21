# Code Flow

이 문서는 A* 길찾기와 측정 코드의 흐름을 기능 기준으로 정리합니다. README보다 자세하되, 함수 전체 목록이 아니라 각 흐름의 핵심 함수만 설명합니다. 이 프로젝트는 A* 구현 자체보다 측정 조건을 통제하고 단계별로 최적화한 과정이 핵심이므로, 각 흐름에 최적화 포인트를 함께 정리합니다.

현재 소스(`AStar_PathFinding/`)는 최적화가 모두 반영된 최종본입니다. 각 최적화 단계의 "이전" 코드는 별도 폴더가 아니라 단계별 측정 결과(`길찾기 측정 자료/`)로만 남아 있어, 변경 전 코드는 최종본의 흔적(스테일 주석 등)과 폴더명으로 추정합니다. 추정 지점은 각 흐름의 "확인 필요"에 표시했습니다.

---

## 1. 맵 생성 흐름

- 목적: 매 반복마다 랜덤 격자와 확률 기반 장애물을 만들어 측정 표본을 다양화한다.
- 관련 파일: `AStar_PathFinding/Map.cpp`, `MapGenerator.cpp`, `AppController.cpp`(구동)
- 관련 클래스: `Map`(`m_tileMap` = 2차원 벡터), `MapGenerator`(`m_probBoard`)
- 관련 함수: `Map::Init/MapClear/SetWall/GetTilePosType`, `MapGenerator::init/GenProbWall`
- 처리 순서:
  1. `Map::Init(width, height)`가 격자를 모두 이동 가능(Empty) 타일로 초기화한다. 반복 시작마다 `MapClear`로 다시 Empty로 되돌린다.
  2. `MapGenerator::init`이 각 타일에 0~100의 랜덤 확률을 심는다(타일마다 벽 생성 확률이 다르다).
  3. `GenProbWall(map)`이 타일별 확률에 따라 벽(이동 불가)과 통로(이동 가능)를 배치한다.
- 최적화 포인트: 맵 자체는 최적화 대상이 아니다. 다만 매 반복 새 랜덤 맵을 만들어 측정 표본을 다양화하는 것이 목적이다. 랜덤 맵이므로 시작→도착 경로가 아예 없는 맵이 생길 수 있고, 이 경우는 3번 흐름(BFS)에서 걸러낸다.
- 관련 테스트: 5만 회 반복 측정.
- 참고: `srand`는 프로그램 시작 시 `MapGenerator::init`에서 한 번만 호출되고, 타일별 확률판(`m_probBoard`, 0~100)도 이때 한 번 생성된다. 매 반복의 `GenProbWall`은 타일별 `rand()` 값만 다시 굴려 벽을 배치하므로, 지형의 밀도 분포는 고정되고 표본은 반복마다 달라진다.

---

## 2. 시작점 / 도착점 선정 흐름

- 목적: 벽이 아닌 빈 타일에서 시작점·도착점을 무작위로 뽑는다.
- 관련 파일: `AppController.cpp`(`OnAutoPathFinding`), `Map.cpp`
- 관련 클래스: `AppController`, `Map`
- 관련 함수: `AppController::OnAutoPathFinding`, `Map::SetStartPos/SetGoalPos/GetTilePosType`
- 처리 순서:
  1. 시작점 좌표를 `rand()%width`, `rand()%height`로 뽑는다. 해당 타일이 빈 타일(Empty)이 아니면 다시 뽑는다(벽 위 시작 방지).
  2. `SetStartPos`로 시작점을 확정한다.
  3. 도착점도 같은 방식으로 빈 타일에서 뽑아 `SetGoalPos`로 확정한다.
  4. 좌표가 빈 타일인지 1차로 확인하고, "경로 존재" 유효성은 다음 3번 흐름(BFS)에서 판정한다.
- 최적화 포인트: 없음(측정 공정성을 위한 전처리).
- 관련 테스트: 5만 회 반복 측정.
- 참고: 시작점과 도착점이 같은 좌표가 되는 경우는 방지된다 — 시작점을 `SetStartPos`로 확정하면 그 타일은 Start 타입이 되고, 도착점은 Empty 타일에서만 다시 뽑으므로 같은 좌표가 선택되지 않는다.

---

## 3. BFS ValidateGoal 흐름

- 목적: A* 측정 전에 시작→도착 경로가 실제로 존재하는지 BFS로 선판정해 측정 오염을 막는다.
- 관련 파일: `PathFinder.cpp`(`ValidateGoal`, BFS용 `Search` 오버로드), `AppController.cpp`(automode 호출)
- 관련 클래스: `PathFinder`
- 관련 함수: `PathFinder::ValidateGoal`, `PathFinder::Search`(BFS 버전), `PathFinder::PathFinding`(automode 분기)
- 처리 순서:
  1. `PathFinding(..., automode=true)` 진입 시 가장 먼저 `ValidateGoal`을 호출한다.
  2. BFS는 큐(`queue<pair<int,int>>`)에 시작점을 넣고 방문 배열(`visited`)로 방문을 표시한다.
  3. 큐를 비우며 front가 도착점이면 성공, 아니면 8방향 이웃을 큐에 넣는다(범위·벽·방문 체크 통과 시).
  4. 도착점에 도달하면 true, 큐가 비면 false를 반환한다.
  5. `ValidateGoal`이 false면 `PathFinding`이 바로 실패를 반환하고, 구동부는 맵을 새로 만들어 재시도한다.
- 최적화 포인트: 측정 공정성의 핵심이다. 경로 없는 맵에서 A*가 격자를 거의 다 탐색하는 최악 케이스가 표본에 섞이면 평균이 왜곡되므로 BFS로 사전 배제한다. 또한 측정용 프로파일러(`CProfiler`)는 `ValidateGoal` 통과 **이후**에 생성되므로, BFS 시간은 A* 측정에 포함되지 않는다.
- 관련 테스트: 5만 회 반복 측정(경로 있는 케이스만 측정).
- 확인 필요: BFS용 `Search`는 여전히 switch-case를 사용한다(성능 비민감 경로라 미최적화). A*용 `Search`(7번)와 대비된다.

---

## 4. A* Search 흐름

- 목적: 우선순위 큐 기반 A*로 최단 경로를 찾고 부모 좌표로 경로를 복원한다.
- 관련 파일: `PathFinder.cpp`(`PathFinding`, A*용 `Search`), `PathFinder.h`(`Tile`/`Node`), `PriorityQueue.h`
- 관련 클래스: `PathFinder`, `PathFinder::Tile`(결과 격자), `PathFinder::Node`(오픈리스트 원소), `PriorityQueue`
- 관련 함수: `PathFinding`, `Search`(A* 버전), `PathClear`, `PriorityQueue::push/pop/top/empty`
- 처리 순서:
  1. 오픈리스트(f 값 기준 최소 힙)에 시작 노드를 넣는다.
  2. 오픈리스트가 비면 실패. 아니면 f가 가장 작은 노드를 꺼낸다.
  3. 꺼낸 노드가 도착점이면 종료한다.
  4. (지연 삭제) 꺼낸 노드의 f가 격자에 저장된 f보다 크면(`cur.m_fVal > result.m_fVal`) 이미 더 좋은 값으로 갱신된 오래된 항목이므로 건너뛴다. 저장된 값과 같은(그 타일의 최신 최적) 노드는 통과시킨다.
  5. 현재 노드를 close 처리한다.
  6. 8방향 이웃을 `Search`로 확장한다: 범위·벽·close 체크 후 g를 계산하고(기존 g보다 나쁘면 건너뜀), h를 계산해 격자 Tile에 g/h/f와 **부모 좌표**를 저장하고 오픈리스트에 넣는다.
  7. 도착 후에는 격자의 부모 좌표 체인을 거슬러 올라가며 경로를 복원한다.
- 최적화 포인트: 우선순위 큐를 표준 컨테이너 대신 배열 기반 최소 힙(`PriorityQueue`)으로 직접 구현해 `clear`로 메모리를 재사용한다. 지연 삭제로 노드 갱신 시 힙 감소키 대신 중복 push + 건너뛰기를 쓴다. g/h/f·부모를 격자에 인라인 저장해 별도 자료구조를 두지 않는다.
- 관련 테스트: 5만 회 반복 측정(핵심 측정 대상).
- 참고: 경로 복원의 실제 순회는 렌더러(`Renderer.cpp`의 `RenderPen`/`RenderBrush`)에서 도착점→시작점으로 부모 좌표를 따라 수행한다. `PathFinder`는 각 Tile에 부모 좌표를 기록하는 데까지만 책임진다.

---

## 5. 휴리스틱 변경 적용 흐름

- 목적: 8방향 이동에 맞는 휴리스틱으로 바꾸고 불필요한 제곱근 연산을 제거한다.
- 관련 파일: `PathFinder.cpp`(`PathFinding`의 시작 f, `Search`의 h 계산), `PathFinder.h`(Tile 주석)
- 관련 클래스: `PathFinder`
- 관련 함수: `PathFinding`, `Search`
- 처리 순서:
  1. 기존: 초기엔 유클리드 거리 휴리스틱을 사용함.
  2. 변경 후: 현재 코드의 h와 시작 f는 8방향 이동(직선 비용 1, 대각 비용 1.5)에 맞는 옥타일 형태의 거리로 계산한다. 즉 `(dx+dy)`에서 대각으로 줄어드는 만큼을 빼는 방식이다.
  3. 이 식은 덧셈·최솟값·상수 곱만 사용하므로 제곱근 호출이 없다.
- 최적화 포인트: 제곱근 제거로 노드마다 반복되는 h 계산 비용을 줄이고, 8방향 격자에 맞는 정확한 하한으로 탐색 노드 수를 줄인다.
- 관련 테스트: 측정 자료 "휴리스틱 알고리즘 변경" 단계.


---

## 6. 캐시 지역성 개선 적용 흐름

- 목적: Tile/Node 구조를 줄이고 결과 격자를 연속 메모리로 배치해 캐시 적중률을 높인다.
- 관련 파일: `PathFinder.h`(`Tile`/`Node`), `PathFinder.cpp`(`Init`의 결과 격자 할당)
- 관련 클래스: `PathFinder::Tile`, `PathFinder::Node`, `PathFinder`(결과 격자)
- 관련 함수: `PathFinder::Init`, `PathFinder::PathClear`
- 처리 순서:
  1. Tile 구조는 부모 좌표·close·g/h/f만 담은 작은 구조체다.
  2. Node(오픈리스트 원소)는 좌표·g·f + 비교 연산자만 담은 최소 필드로, 힙 이동 비용을 줄인다.
  3. `Init`에서 결과 격자를 한 덩어리(`new Tile[width*height]`)로 잡고, 행 포인터가 그 안을 가리키게 한다. 즉 2차원처럼 접근하지만 실제 데이터는 연속 메모리다.
- 최적화 포인트: `vector<vector<Tile>>`이나 행별 개별 할당은 행마다 힙이 흩어져 캐시 미스가 잦다. 위 방식은 전체가 연속이라 인접 타일을 순회·초기화(`PathClear`)할 때 캐시 라인 재사용이 좋다. A*는 인접 타일을 반복 접근하므로 이 배치가 실측 성능에 영향을 준다.
- 관련 테스트: 측정 자료 "캐시 히트율 높이기(구조체 크기 감소 및 2차원 배열 형태 변경)" 단계.


---

## 7. switch 제거 적용 흐름

- 목적: 8방향 처리를 switch-case에서 인덱스 테이블 참조로 바꿔 분기를 없앤다.
- 관련 파일: `PathFinder.cpp`(A*용 `Search`의 방향/비용 테이블 vs BFS용 `Search`의 switch)
- 관련 클래스: `PathFinder`
- 관련 함수: `PathFinder::Search`(A* 버전)
- 처리 순서:
  1. 기존(BFS 버전에 남아 있음): 방향별 `switch(dir)`로 8방향 오프셋을 분기 처리한다.
  2. 변경 후(A* 버전): 방향 오프셋 배열(`dir_x`, `dir_y`)과 비용 배열(`cost`)을 static으로 두고, `x = xpos + dir_x[dir]`, `cost[dir]`로 분기 없이 계산한다. static이라 함수 호출마다 배열을 다시 초기화하지 않는다.
- 최적화 포인트: 8방향 분기 예측 실패·점프를 제거해 노드마다의 처리 비용을 줄인다. 비용 테이블(대각 1.5)로 g 증가분도 분기 없이 조회한다. 반복 루프(모든 확장 노드 × 8방향)에서 분기 제거 효과가 누적된다.
- 관련 테스트: 측정 자료 "switch_case문 제거" 단계.


---

## 8. 측정 / 프로파일링 흐름

- 목적: 5만 회 반복으로 A*를 측정하고 평균/최소/최대를 파일로 남긴다.
- 관련 파일: `ProfilerTLS.h`(`CProfiler`, `CProfilerManager`), `AppController.cpp`(`OnAutoPathFinding`, `Init`), `PathFinder.cpp`(측정 구간), 결과 `Profiling [PathFinding]_*.txt`
- 관련 클래스: `CProfiler`(RAII), `CProfilerManager`(싱글턴, 스레드 로컬)
- 관련 함수: `CProfilerManager::Init/ProfileDataOutText`, `CProfiler::ProfileBegin/ProfileEnd`(생성자/소멸자), `AppController::OnAutoPathFinding`
- 처리 순서:
  1. 구동부는 반복 횟수(측정에서 5만)를 세팅한다. `OnAutoPathFinding`이 매 호출마다 새 맵 생성 → BFS 검증 → A* 실행(성공 시에만 카운트) → 남은 횟수 감소를 반복하고, 0이 되면 결과를 저장하고 종료한다.
  2. A* 측정 구간에 `CProfiler` 객체를 생성한다(RAII). 생성자가 시작 시각(성능 카운터)을 기록하고, 소멸자가 경과를 태그별 데이터에 누적한다.
  3. 태그별로 총 시간·호출 수와 함께 상위 몇 개·하위 몇 개의 극단값을 유지한다.
  4. 출력 시 극단값을 제외한 평균을 마이크로초로 계산해 파일로 기록한다.
- 관련 테스트: 측정 자료 각 단계의 Profiling 결과.
