#pragma once
#define WM_AUTO_PATHFINDING (WM_APP+1)

class AppController
{
public:
	enum class RunMode
	{
		Manual = 0,
		Auto   = 1,
	};

	enum class MouseMode
	{
		None = 0,
		Erase = 1,
		Draw = 2,
		Move = 3,
	};

	struct settings
	{
		RunMode m_mode      = RunMode::Manual;
		INT     m_mapWidth  = 100;             // 그리드가 가로롤 100개
		INT     m_mapHeight = 50;              // 그리드가 세로로 50개
		UINT64  m_count = 0;
	};

	struct st_Pos
	{
		int m_xpos;
		int m_ypos;
	}typedef Pos;

public:
	AppController() = default;
	~AppController();

	void Init(const settings& src, HWND hWnd);
	inline RunMode GetMode() const noexcept
	{
		return m_mode;
	}

	// 객체 사용 핸들러 
	void OnKeyDown(HWND hwnd, WPARAM wParam);
	void OnPaint(HWND hWnd);
	void OnMouseWheel(HWND hwnd, int wheelDelta, int clientX, int clientY);
	void OnLButtonDown(HWND hwnd, int clientX, int clientY);
	void OnRButtonDown(HWND hwnd, int clientX, int clientY);
	void OnLButtonUp();
	void OnMouseMove(HWND hwnd, WPARAM wParam, int clientX, int clientY);
	void OnMButtonDown(HWND hwnd, int clientX, int clientY);
	void OnMButtonUp();
	void OnAutoPathFinding(HWND hwnd);

private:
	RunMode      m_mode;        // 수동 맵 생성, 자동 맵 생성
	MouseMode    m_drag;        // 마우스 모드
	Pos          m_oldMousePos; // 이전 마우스 위치
	UINT64       m_testCnt;     // 자동 모드 길찾기 횟수
	Map          m_map;
	MapGenerator m_generator;
	Renderer     m_renderer;
	PathFinder   m_pathfinder;
};

