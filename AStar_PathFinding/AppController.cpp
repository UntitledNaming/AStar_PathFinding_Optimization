#include <windows.h>
#include <vector>
#include <string>
#include <queue>
#include "ProfilerTLS.h"
#include "Map.h"
#include "MapGenerator.h"
#include "PriorityQueue.h"
#include "PathFinder.h"
#include "Renderer.h"
#include "AppController.h"

AppController::~AppController()
{

}

void AppController::Init(const settings& src, HWND hWnd)
{
	m_mode = src.m_mode;
	m_drag = MouseMode::None;
	m_map.Init(src.m_mapWidth, src.m_mapHeight);
	m_renderer.Init(hWnd);
	m_pathfinder.Init(src.m_mapWidth, src.m_mapHeight);
	m_generator.init(src.m_mapWidth, src.m_mapHeight);
	m_oldMousePos.m_xpos = 0;
	m_oldMousePos.m_ypos = 0;
	m_testCnt = src.m_count;

	CProfilerManager::GetInstance()->Init();

	CProfilerManager::GetInstance()->ProfileInit();
}

void AppController::OnKeyDown(HWND hwnd, WPARAM wParam)
{
	// 길찾기
	if (wParam == VK_SPACE)
	{
		int sX = m_map.GetStartPos().m_xpos;
		int sY = m_map.GetStartPos().m_ypos;
		int gX = m_map.GetGoalPos().m_xpos;
		int gY = m_map.GetGoalPos().m_ypos;

		m_pathfinder.PathFinding(sX, sY, gX, gY, m_map);
		InvalidateRect(hwnd, NULL, false);
	}

	// 전체 Map 초기화
	else if (wParam == VK_TAB)
	{
		m_map.MapClear();
		InvalidateRect(hwnd, NULL, false);
	}
}

void AppController::OnPaint(HWND hWnd)
{
	m_renderer.Render(hWnd, m_map, m_pathfinder);
}

void AppController::OnMouseWheel(HWND hwnd, int wheelDelta, int clientX, int clientY)
{
	int oldGridSize = m_renderer.GetGridSize();

	int step = 0;

	if (wheelDelta > 0)
		step = 2;
	else
		step = -2;

	int newGrid = oldGridSize + step;

	if (newGrid < 4)
		newGrid = 4;

	else if (newGrid > 120)
		newGrid = 120;

	if (newGrid == oldGridSize)
		return;

	// 기존 타일 좌표 계산
	int offsetX = m_renderer.GetOffsetX();
	int offsetY = m_renderer.GetOffsetY();
	int tileX = (clientX - offsetX) / oldGridSize;
	int tileY = (clientY - offsetY) / oldGridSize;

	// 새로운 GridSize와 계산한 타일 좌표를 기준으로 새로운 offset 계산
	m_renderer.SetZoomData(newGrid, clientX - tileX * newGrid, clientY - tileY * newGrid);
	InvalidateRect(hwnd, nullptr, FALSE);
}

void AppController::OnLButtonDown(HWND hwnd, int clientX, int clientY)
{
	// 타일 좌표 계산
	int gridSize = m_renderer.GetGridSize();
	int offX = m_renderer.GetOffsetX();
	int offY = m_renderer.GetOffsetY();
	int tileX = (clientX - offX) / gridSize;
	int tileY = (clientY - offY) / gridSize;

	if (!m_map.RangeCheck(tileX, tileY))
		return;

	Map::TileType type = m_map.GetTilePosType(tileX, tileY);
	Map::TilePos pos{ tileX, tileY };

	switch (type)
	{
	case Map::TileType::Empty:
	{
		m_map.SetWall(pos);
		m_drag = MouseMode::Draw;
	}
	break;

	case Map::TileType::Wall:
	{
		m_map.SetEmpty(pos);
		m_drag = MouseMode::Erase;
	}
	break;

	case Map::TileType::Start:
	{
		m_map.SetEmpty(pos);
		m_map.InitStart();
	}
	break;

	case Map::TileType::Goal:
	{
		m_map.SetEmpty(pos);
		m_map.InitGoal();
	}
	break;

	default:
		break;
	}

	InvalidateRect(hwnd, nullptr, FALSE);
}

void AppController::OnRButtonDown(HWND hwnd, int clientX, int clientY)
{
	// 타일 좌표 계산
	int gridSize = m_renderer.GetGridSize();
	int offX = m_renderer.GetOffsetX();
	int offY = m_renderer.GetOffsetY();
	int tileX = (clientX - offX) / gridSize;
	int tileY = (clientY - offY) / gridSize;


	if (!m_map.RangeCheck(tileX, tileY))
		return;


	Map::TileType type = m_map.GetTilePosType(tileX, tileY);
	Map::TilePos pos{ tileX, tileY };
	Map::TilePos start = m_map.GetStartPos();
	Map::TilePos goal = m_map.GetGoalPos();

	// 버튼 키 확인
	bool shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
	bool alt = (GetKeyState(VK_MENU) & 0x8000) != 0;


	switch (type)
	{
	case Map::TileType::Empty:
	{
		if (shift && start.m_xpos == -1 && start.m_ypos == -1)
		{
			m_map.SetStartPos(tileX, tileY);
		}

		else if (alt && goal.m_xpos == -1 && goal.m_ypos == -1)
		{
			m_map.SetGoalPos(tileX, tileY);
		}
	}
	break;

	default:
		break;
	}

	InvalidateRect(hwnd, nullptr, FALSE);

}

void AppController::OnLButtonUp()
{
	m_drag = MouseMode::None;
}

void AppController::OnMouseMove(HWND hwnd, WPARAM wParam, int clientX, int clientY)
{
	// 타일 좌표 계산
	int gridSize = m_renderer.GetGridSize();
	int offX = m_renderer.GetOffsetX();
	int offY = m_renderer.GetOffsetY();
	int tileX = (clientX - offX) / gridSize;
	int tileY = (clientY - offY) / gridSize;
	int w = m_map.GetWidth();
	int h = m_map.GetHeight();


	switch (m_drag)
	{
	case MouseMode::Draw:
	{
		if (!m_map.RangeCheck(tileX, tileY))
			return;

		Map::TileType type = m_map.GetTilePosType(tileX, tileY);
		Map::TilePos pos{ tileX, tileY };

		switch (type)
		{
		case Map::TileType::Empty:
		{
			m_map.SetWall(pos);
		}
		break;
		}
	}
	break;

	case MouseMode::Erase:
	{
		if (!m_map.RangeCheck(tileX, tileY))
			return;

		Map::TileType type = m_map.GetTilePosType(tileX, tileY);
		Map::TilePos pos{ tileX, tileY };

		switch (type)
		{
		case Map::TileType::Wall:
		{
			m_map.SetEmpty(pos);
		}
		break;
		}
	}
	break;


	case MouseMode::Move:
	{
		int dx = clientX - m_oldMousePos.m_xpos;
		int dy = clientY - m_oldMousePos.m_ypos;

		m_renderer.SetZoomData(gridSize, offX + dx, offY + dy);

		m_oldMousePos.m_xpos = clientX;
		m_oldMousePos.m_ypos = clientY;

	}
	break;

	}


	InvalidateRect(hwnd, nullptr, FALSE);
}

void AppController::OnMButtonDown(HWND hwnd, int clientX, int clientY)
{
	m_drag = MouseMode::Move;
	m_oldMousePos.m_xpos = clientX;
	m_oldMousePos.m_ypos = clientY;
}

void AppController::OnMButtonUp()
{
	m_drag = MouseMode::None;
}

void AppController::OnAutoPathFinding(HWND hwnd)
{
	// Map 초기화
	while (true)
	{
		m_map.MapClear();
		m_pathfinder.PathClear();

		// 벽 랜덤 생성
		m_generator.GenProbWall(m_map);

		// 시작점, 도착점 랜덤 생성
		int sxpos = 0;
		int sypos = 0;
		int gxpos = 0;
		int gypos = 0;

		int w = m_map.GetWidth();
		int h = m_map.GetHeight();

		while (true)
		{
			sxpos = rand() % w;
			sypos = rand() % h;

			// 시작점 좌표가 비어있지 않다면 다른 좌표 선택
			if (!(m_map.GetTilePosType(sxpos, sypos) == Map::TileType::Empty))
				continue;

			m_map.SetStartPos(sxpos, sypos);

			while (true)
			{
				gxpos = rand() % w;
				gypos = rand() % h;

				// 도착점 좌표가 비어있지 않다면 다른 좌표 선택
				if (!(m_map.GetTilePosType(gxpos, gypos) == Map::TileType::Empty))
					continue;

				m_map.SetGoalPos(gxpos, gypos);

				break;
			}
			break;
		}

		// 길찾기 실패하면 다시 맵 초기화 해서 벽과 시작점, 도착점 세팅하기
		if (!m_pathfinder.PathFinding(sxpos, sypos, gxpos, gypos, m_map, true))
			continue;
		


		break;
	}


	m_testCnt--;
	if (m_testCnt == 0)
	{
		// 테스트 횟수 끝나면 파일 저장 후 프로세스 종료
		CProfilerManager::GetInstance()->ProfileDataOutText(const_cast<WCHAR*>(L"PathFinding"));
		PostQuitMessage(0);
		return;
	}

	InvalidateRect(hwnd, nullptr, FALSE);
	return;
}

