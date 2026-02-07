#pragma once


class AppController
{
public:
	enum class RunMode
	{
		Manual = 0,
		Auto   = 1,
	};

	struct settings
	{
		RunMode m_mode      = RunMode::Manual;
		INT     m_mapWidth  = 100;             // 그리드가 가로롤 100개
		INT     m_mapHeight = 50;              // 그리드가 세로로 50개
	};

public:
	AppController() = default;
	~AppController();

	void Init(const settings& src, HWND hWnd);


	// 객체 사용 핸들러 
	void OnPaint(HWND hWnd);
	void OnMouseWheel(HWND hwnd, int wheelDelta, int mouseX, int mouseY);
	void OnLButtonDown();

private:
	RunMode      m_mode; // 수동 맵 생성, 자동 맵 생성
	Map          m_map;
	MapGenerator m_generator;
	Renderer     m_renderer;
	PathFinder   m_pathfinder;
};

