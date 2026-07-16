# 코드 의도 문서 (Code Intent) — P3 A* PathFinding 최적화

> 이 문서는 저장소의 모든 코드 파일(.h/.cpp)을 "무엇을 하는가"가 아니라 **"왜 이렇게 작성했는가"** 중심으로 설명한다.
> 동작 흐름은 `docs/Code_Flow.md`, 설계 비교는 `docs/Design_Rationale.md`, 측정 결과는 `docs/Test_Report.md` 참고.
> 현재 소스는 최적화가 모두 반영된 최종본이다. 단계별 "이전" 코드는 미보존(측정 txt로만 존재)이며, switch판은 BFS용 `Search` 오버로드에 대조용으로 남아 있다.

---

## 0. 프로젝트 구조 자체의 의도

이 프로젝트의 주인공은 A* 구현이 아니라 **측정 환경**이다. 코드 배치도 그 목적을 따른다:
측정 대상(`PathFinder`) / 측정 표본 생성(`Map`, `MapGenerator`) / 측정 도구(`ProfilerTLS`) / 구동·조건 입력(`AppController`, `SettingsDialog`) / 눈 검증(`Renderer`)이 분리되어 있어, 측정 대상만 교체하면 같은 환경에서 다른 알고리즘도 잴 수 있다.

---

## 1. PathFinder.h / PathFinder.cpp — 측정 대상 (A* + BFS)

### 구조체·타입

| 항목 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `Tile` (부모 좌표, close, g/h/f) | 결과 격자의 한 칸 | 경로 복원을 위해 각 타일이 자기 부모 좌표를 기억한다. **[캐시 최적화]** 필드 크기를 줄여 캐시라인당 타일 수를 늘렸다. 헤더 주석의 '유클리드'는 변경 전 잔재 |
| `Node` (x, y, g, f + `operator>`) | open list에 들어가는 최소 단위 | 무거운 Tile 대신 **가벼운 Node만 힙에 넣는** 분리 — push/pop 시 복사 비용과 힙 메모리를 줄인다 |
| `Dir` (enum, 8방향) | 방향 인덱스 | 방향을 정수 인덱스로 쓰는 것 자체가 오프셋 테이블 참조의 전제 |

### 함수

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `Init(w, h)` | `new Tile[w*h]` 한 블록 할당 + 행 포인터 배열 구성 | **[캐시 최적화 핵심 지점]** `vector<vector<Tile>>`은 행마다 힙 할당이 흩어져 캐시 미스를 유발한다. 한 덩어리 연속 메모리로 바꾸되 행 포인터를 얹어 `m_result[y][x]` 문법은 유지 — 접근 코드를 안 바꾸고 배치만 바꿨다 |
| `PathFinding()` | (자동 모드) BFS 선판정 → 시작 노드 f 계산 → A* 루프. `CProfiler`로 측정 | 측정 구간이 이 함수 안에서 시작되도록 배치해, **경로 존재 검증(BFS)이 측정 밖**에 있게 한 것이 공정성의 핵심 |
| `ValidateGoal()` | BFS로 시작→도착 경로 존재 확인 | 경로 없는 입력이 섞이면 A*가 전 노드를 헛탐색해 평균이 오염된다. 측정 대상을 "경로가 있는 경우의 A*"로 통제 |
| `Search(Dir, x, y, gx, gy, f, g, map)` (A*용) | 이웃 확장: `dir_x/dir_y/cost` static 테이블로 좌표·비용 계산, 옥타일 h 계산, g 개선 시에만 open list 삽입 | **[switch 제거 지점]** 방향 분기를 테이블 참조로 대체해 분기 예측 실패를 없앴다. h는 `(dx+dy) - 0.5·min(dx,dy)` 옥타일식 인라인 — 함수 호출 비용도 없앴고 sqrt도 없다. "계산한 g가 기존 g보다 크면 안 넣는" 조건은 open list 폭증 문제의 해결 지점 |
| `Search(Dir, x, y, map, que, visited)` (BFS용 오버로드) | BFS 이웃 확장. **switch-case 잔존** | 의도적 대조군: 측정 대상이 아닌 BFS는 최적화하지 않고 남겨, 같은 파일에서 before(switch)/after(테이블)를 비교할 수 있게 했다 |
| `PathClear()` | 격자·open list 재사용 초기화 | 5만 회 반복에서 매번 할당/해제하면 측정에 할당 비용이 섞인다. 재사용 초기화로 반복 간 상태만 리셋. 최초 호출은 플래그로 스킵(호출 횟수가 PathFinding보다 1 적은 이유) |
| `GetResult()` / `GetPathFlag()` | 결과 격자/성공 플래그 노출 | Renderer가 경로를 복원·검증할 수 있게 결과를 읽기 전용으로 개방 |
| `~PathFinder()` | 연속 블록 해제 | Init에서 한 블록으로 잡았으므로 해제도 한 번 |

---

## 2. PriorityQueue.h — 자체 우선순위 큐

std::priority_queue를 쓰지 않고 직접 만든 이유: (1) 미리 capacity를 잡아 push 중 재할당·복사를 통제하고, (2) `clear()`가 size만 0으로 만들어 **메모리를 반복 간 재사용**하게 하기 위해서다. 반복 측정 측정 프로그램에서는 컨테이너의 숨은 할당이 측정 노이즈가 된다.

| 함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `push(T)` | 용량 부족 시 `assign(2배+1)` 후 말단 삽입 → 상향 비교 | 표준 배열 힙. 성장 정책을 코드에 명시해 재할당 시점을 예측 가능하게 |
| `pop()` | 루트 제거 → 말단을 루트로 → 하향 비교(두 자식 중 작은 쪽) | min-heap 유지. 비교자는 템플릿 인자로 분리 |
| `top()` / `empty()` | 루트 조회/빈 검사 | A* 루프의 종료·추출 조건 |
| `assign(n)` | 새 배열 할당 + 복사 | 기존 capacity 이상이면 false로 거부 — 축소 재할당을 막는 방어 |
| `clear()` | size = 0 (메모리 유지) | **재사용 핵심.** 해제하지 않아 다음 반복이 같은 메모리를 씀 |
| `MinCmp` / `MaxCmp` | 비교자 | 힙 방향을 타입 인자로 결정 — 큐 코드 재사용 |

---

## 3. Map.h/.cpp · MapGenerator.h/.cpp — 측정 표본

| 파일/함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `Map::Init` / `MapClear` | 타일맵 생성/전체 Empty 리셋 | 반복마다 맵 객체를 새로 만들지 않고 리셋 — 표본 생성 비용을 측정 밖에서도 최소화 |
| `Map::SetWall/SetEmpty/SetStartPos/SetGoalPos` | 타일 타입 설정 | 수동 편집(마우스)과 자동 생성이 같은 API를 쓰게 통일 |
| `Map::RangeCheck` | 좌표 범위 검사 | 이웃 확장에서 경계 밖 접근을 한 곳에서 차단 |
| `Map::InitStart/InitGoal` | 시작/도착 초기화(-1) | "선정 안 됨" 상태를 명시적 값으로 |
| `MapGenerator::init` / `GenProbWall` | 타일별 랜덤 확률로 벽 생성 | 하나의 맵 표본으로는 알고리즘 우열을 말할 수 없다는 문제의식의 구현체. 확률 기반이라 매 반복 다른 표본이 나오고, 5만 회 평균이 특정 지형에 좌우되지 않는다 |

---

## 4. ProfilerTLS.h — 측정 도구

| 항목 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `CProfiler` (RAII: 생성=Begin, 소멸=End) | 구간 측정 | 측정 시작/종료 쌍을 수동 호출하면 누락·불일치가 난다. 스코프 수명에 묶어 **측정 구간이 코드 블록과 정확히 일치**하게 강제 |
| `CProfilerManager::Init/ProfileInit` | TLS 인덱스 발급, 스레드별 샘플 등록 | 멀티스레드 측정에서 락 없이 기록하기 위해 스레드별 저장소(TLS) 사용 — P2 비교 테스트와 공유하는 인프라 |
| `ProfileBegin/ProfileEnd` | QueryPerformanceCounter로 tick 기록·누적, 에러 플래그 검증, Max[2]/Min[2] 갱신 | QPC는 고해상도 단조 시계. 에러 플래그(0xFFFF) 검사로 Begin 없는 End 같은 오사용을 즉시 throw |
| `ProfileDataOutText` | 상하위 2개 제외한 트림 평균을 μs로 환산해 파일 저장 | 이상치(첫 실행 캐시 미스, OS 개입)가 평균을 왜곡하지 않도록 **트림 평균**을 지표로 채택. 결과를 파일로 남겨 단계별 비교가 가능하게 |
| `st_PROFILE_DATA/st_TOTAL_DATA` | TotalTime/CallTime/Max[2]/Min[2] | 트림에 필요한 최소 상태만 유지(전체 샘플 배열 비보존) — 5만 회 샘플을 다 들고 있지 않으려는 메모리 절약 |

---

## 5. 구동·UI — AppController, AStar_PathFinding.cpp, SettingsDialog, Renderer

| 파일/함수 | 하는 일 | 이렇게 작성한 의도 |
|---|---|---|
| `AStar_PathFinding.cpp` (WinMain/WndProc) | 메시지 루프. `WM_AUTO_PATHFINDING`(WM_APP+1) 수신 시 `OnAutoPathFinding` 호출, 처리 후 스스로 `PostMessage`로 재게시 | 자동 측정 5만 회를 for 루프로 돌리면 UI가 멈춘다. **자기 자신에게 메시지를 다시 던지는 방식**으로 반복하면 메시지 펌프가 살아 있어 진행 중에도 창·그리기·중단이 동작한다 |
| `AppController::Init` | 맵/파인더/생성기/프로파일러 초기화, 목표 횟수(m_testCnt) 설정 | 측정 조건을 한 곳에서 조립 |
| `AppController::OnAutoPathFinding` | 맵 생성 → 시작/도착 랜덤 선정 → BFS 검증 → A* → 카운트, 목표 도달 시 `ProfileDataOutText` 저장 | 측정 절차 5단계가 이 함수 하나에 순서대로 있음 — 처리 절차을 코드에서 그대로 읽을 수 있게 |
| `AppController::OnKeyDown` (SPACE) | 수동 1회 길찾기 | 자동 측정과 별개로, 특정 맵에서 눈으로 검증하는 경로 |
| `AppController::OnLButtonDown/OnRButtonDown/OnMouseMove/OnMouseWheel` | 벽 그리기/지우기, 시작·도착 지정, 줌 | 디버깅용 수동 편집. 측정과 무관하므로 최적화 대상에서 제외 |
| `SettingsDialog.cpp` | `GetDlgItemInt`로 맵 W/H/반복 횟수 입력 | 측정 조건(그리드 크기, 5만 회)을 재빌드 없이 바꾸기 위한 입력 창 |
| `Renderer.cpp` | 격자·벽·open/close 상태 그리기. 경로는 도착점에서 `Tile`의 부모 좌표(m_pXpos/m_pYpos)를 따라 시작점까지 LineTo로 역추적 | 그리드 확대 시 g/h/f 값이 보이게 해 **알고리즘 검증을 눈으로** 할 수 있게 한 것이 초기 목적. 경로 복원 로직이 Renderer에 있는 이유: PathFinder는 부모 기록까지만 책임지고, 복원·표시는 소비자 몫으로 분리 |
| `framework.h/targetver.h/Resource.h/resource1.h` | VS 생성 보일러플레이트 | 별도 의도 없음 |

---

## 6. 공용 헤더 (LFStack.h, LockFreeMemoryPoolLive.h, MemoryPoolTLS.h)

P1/P2에서 가져온 공용 인프라 사본. 이 프로젝트의 .cpp에서는 include되지 않음을 확인했다(서로 간 include만 존재). 초기에 노드 할당 최적화(메모리 풀 적용) 실험을 고려해 가져왔던 것으로, **현재 측정 경로에서는 미사용**이다. 발표·문서에서 P3 기술 스택으로 언급하지 않는다.

---

## 7. 확인 필요

| 항목 | 내용 |
|---|---|
| 단계별 "이전" 코드 | 유클리드/vector 배치/switch(A*)판 미보존. before 근거는 측정 txt와 BFS `Search` 잔존 switch |
| 단계별 Profiling txt 수치 | 4단계 Avg/Min/Max 실값 파싱 후 문서 인용 확정 |
| BFS switch 잔존 | 측정 비대상이라 미최적화로 남긴 것 — 면접에서 "의도적 대조군"으로 설명하되, 원래 의도였는지 사후 활용인지는 구분해 말할 것 |
