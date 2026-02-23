#include <windows.h>
#include <vector>
#include <queue>
#include "framework.h"
#include "Map.h"
#include "PriorityQueue.h"
#include "PathFinder.h"
#include "Renderer.h"

Renderer::~Renderer()
{
	// 메모리 DC에 연결된 기존 BITMAP 연결 제거
	SelectObject(m_memDC, m_bitMapOld);

	// 메모리 DC에서 연결 끊은 비트맵 제거
	DeleteObject(m_bitMap);

	// 메모리 DC 제거
	DeleteObject(m_memDC);

	// 펜, 브러시 제거
	DeleteObject(m_gridPen);
	DeleteObject(m_wallBrush);
	DeleteObject(m_startBrush);
	DeleteObject(m_goalBrush);
	DeleteObject(m_pathPen);
	DeleteObject(m_pathBrush);
	DeleteObject(m_nodeBrush);
}

void Renderer::Init(HWND hWnd)
{
	HDC hdc = GetDC(hWnd); // 윈도우 클라이언트 영역에 그릴 수 있는 hdc를 받기(대여)
	GetClientRect(hWnd, &m_rc);

	m_gridSize = 16;
	m_offsetX = 0;
	m_offsetY = 0;
	m_bitMap = CreateCompatibleBitmap(hdc, m_rc.right, m_rc.bottom);
	m_memDC = CreateCompatibleDC(hdc);
	ReleaseDC(hWnd, hdc); // hdc os에게 반납

	// 메모리 DC에 생성한 비트맵을 연결하고 기존에 연결되어 있던 비트맵을 리턴해서 받음
	m_bitMapOld = (HBITMAP)SelectObject(m_memDC, m_bitMap);

	// 펜 및 브러시 생성
	m_gridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
	m_pathPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	m_wallBrush = CreateSolidBrush((RGB(100, 100, 100)));
	m_startBrush = CreateSolidBrush((RGB(255, 0, 0)));
	m_goalBrush = CreateSolidBrush((RGB(0, 0, 255)));
	m_pathBrush = CreateSolidBrush((RGB(255, 255, 0)));
	m_nodeBrush = CreateSolidBrush((RGB(0, 255, 0)));
	
}

void Renderer::Render(HWND hWnd, const Map& map, const PathFinder& pathfinder)
{
	HDC hdc = GetDC(hWnd);

	PatBlt(m_memDC, 0, 0, m_rc.right, m_rc.bottom, WHITENESS);

	RenderBrush(m_memDC, map, pathfinder);
	RenderPen(m_memDC, map, pathfinder);

	if (m_gridSize > 40)
	{
		RenderText(m_memDC, map, pathfinder);
	}

	BitBlt(hdc, 0, 0, m_rc.right, m_rc.bottom, m_memDC, 0, 0, SRCCOPY);

	ReleaseDC(hWnd, hdc);
}

void Renderer::RenderPen(HDC memdc, const Map& map, const PathFinder& pathfinder)
{
	HPEN hOldPen = (HPEN)SelectObject(memdc, m_gridPen);

	int w = map.GetWidth();
	int h = map.GetHeight();
	int tileWPixel = w * m_gridSize;
	int tileHPixel = h * m_gridSize;

	// 타일 맵의 가로 타일 갯수 만큼 세로선 긋기
	for (int tx = 0; tx <= w; tx++)
	{
		int cx = m_offsetX + tx * m_gridSize;
		int y0 = m_offsetY;
		int y1 = m_offsetY + tileHPixel;

		MoveToEx(memdc, cx, y0, NULL);
		LineTo(memdc, cx, y1);
	}

	// 타일 맵의 세로 타일 갯수 만큼 가로선 긋기
	for (int ty = 0; ty <= h; ty++)
	{
		int cy = m_offsetY + ty * m_gridSize;
		int x0 = m_offsetX;
		int x1 = m_offsetX + tileWPixel;

		MoveToEx(memdc, x0, cy, NULL);
		LineTo(memdc, x1, cy);
	}
	SelectObject(memdc, hOldPen);

	// 경로 줄 그리기
	if (pathfinder.GetPathFlag())
	{
		int sxpos = map.GetStartPos().m_xpos;
		int sypos = map.GetStartPos().m_ypos;
		int gxpos = map.GetGoalPos().m_xpos;
		int gypos = map.GetGoalPos().m_ypos;
		int oldxpos = gxpos;
		int oldypos = gypos;

		PathFinder::Tile** result = pathfinder.GetResult();

		HPEN hOldPenPath = (HPEN)SelectObject(memdc, m_pathPen);

		while (true)
		{
			int curxpos = result[oldypos][oldxpos].m_pXpos;
			int curypos = result[oldypos][oldxpos].m_pYpos;


			MoveToEx(memdc, m_offsetX + oldxpos * m_gridSize + m_gridSize / 2, m_offsetY + oldypos * m_gridSize + m_gridSize / 2, NULL);
			LineTo(memdc, m_offsetX + curxpos * m_gridSize + m_gridSize / 2, m_offsetY + curypos * m_gridSize + m_gridSize / 2);

			if (curxpos == sxpos && curypos == sypos)
				break;

			oldxpos = curxpos;
			oldypos = curypos;
		}

		SelectObject(memdc, hOldPenPath);
	}
}

void Renderer::RenderBrush(HDC memdc, const Map& map, const PathFinder& pathfinder)
{
	int w = map.GetWidth();
	int h = map.GetHeight();

	PathFinder::Tile** result = pathfinder.GetResult();

	for (int ty = 0; ty < h; ty++)
	{
		for (int tx = 0; tx < w; tx++)
		{
			// Start 색칠
			if (map.GetTilePosType(tx, ty) == Map::TileType::Start)
			{
				HBRUSH oldStartBrush = (HBRUSH)SelectObject(memdc, m_startBrush);

				Rectangle(memdc, m_offsetX + tx * m_gridSize, m_offsetY + ty * m_gridSize, m_offsetX + (tx + 1) * m_gridSize, m_offsetY + (ty + 1) * m_gridSize);

				SelectObject(memdc, oldStartBrush);
				continue;
				
			}
			// Goal 색칠
			else if (map.GetTilePosType(tx, ty) == Map::TileType::Goal)
			{
				HBRUSH oldGoalBrush = (HBRUSH)SelectObject(memdc, m_goalBrush);

				Rectangle(memdc, m_offsetX + tx * m_gridSize, m_offsetY + ty * m_gridSize, m_offsetX + (tx + 1) * m_gridSize, m_offsetY + (ty + 1) * m_gridSize);

				SelectObject(memdc, oldGoalBrush);
				continue;
			}

			// Wall 색칠
			else if (map.GetTilePosType(tx, ty) == Map::TileType::Wall)
			{
				HBRUSH oldWallBrush = (HBRUSH)SelectObject(memdc, m_wallBrush);

				Rectangle(memdc, m_offsetX + tx * m_gridSize, m_offsetY + ty * m_gridSize, m_offsetX + (tx + 1) * m_gridSize, m_offsetY + (ty + 1) * m_gridSize);

				SelectObject(memdc, oldWallBrush);
				continue;
			}

			if (result[ty][tx].m_close)
			{
				HBRUSH oldNodeBrush = (HBRUSH)SelectObject(memdc, m_nodeBrush);

				Rectangle(memdc, m_offsetX + tx * m_gridSize, m_offsetY + ty * m_gridSize, m_offsetX + (tx + 1) * m_gridSize, m_offsetY + (ty + 1) * m_gridSize);

				SelectObject(memdc, m_nodeBrush);
			}
		}
	}


	// 경로 색칠
	if (pathfinder.GetPathFlag())
	{
		int sxpos = map.GetStartPos().m_xpos;
		int sypos = map.GetStartPos().m_ypos;
		int gxpos = map.GetGoalPos().m_xpos;
		int gypos = map.GetGoalPos().m_ypos;
		int oldxpos = gxpos;
		int oldypos = gypos;

		HBRUSH oldPathBrush = (HBRUSH)SelectObject(memdc, m_pathBrush);
		while (true)
		{
			int curxpos = result[oldypos][oldxpos].m_pXpos;
			int curypos = result[oldypos][oldxpos].m_pYpos;

			if (curxpos == sxpos && curypos == sypos)
				break;

			Rectangle(memdc, m_offsetX + curxpos * m_gridSize, m_offsetY + curypos * m_gridSize, m_offsetX + (curxpos + 1) * m_gridSize, m_offsetY + (curypos + 1) * m_gridSize);

			oldxpos = curxpos;
			oldypos = curypos;
		}
		SelectObject(memdc, oldPathBrush);
	}
}

void Renderer::RenderText(HDC memdc, const Map& map, const PathFinder& pathfinder)
{
	// 텍스트 배경 투명하게 만들기
	SetBkMode(memdc, TRANSPARENT);


	int h = map.GetHeight();
	int w = map.GetWidth();
	PathFinder::Tile** result = pathfinder.GetResult();

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			// 방문 안한 타일은 pass
			if (result[y][x].m_gVal < 0)
				continue;

			// 방문 했으면 해당 타일의 클라 좌표로 사각형을 만듬.
			RECT tileRc{ m_offsetX + x * m_gridSize,m_offsetY + y * m_gridSize, m_offsetX + (x + 1) * m_gridSize, m_offsetY + (y + 1) * m_gridSize };

			// 표시할 텍스트를 buf에 저장
			WCHAR buf[128];
			swprintf_s(buf, L"g:%.0f\nh:%0.f\nf:%.0f", result[y][x].m_gVal, result[y][x].m_hVal, result[y][x].m_fVal);

			// 실제 메모리dc에 쓰기
			DrawTileTextCliped(memdc, tileRc, buf);
			
		}
	}

}

void Renderer::DrawTileTextCliped(HDC memdc, const RECT& tileRC, const WCHAR* buf)
{
	// 현재 DC 상태(폰트, 펜, 브러시 ,좌표계등) 전체 저장
	int saved = SaveDC(memdc);

	// 클립을 타일로 제한함으로 앞으로의 모든 그리기 작업이 해당 사각형 안에만 보이고 밖으로 삐져나온 부분은 잘림
	IntersectClipRect(memdc, tileRC.left, tileRC.top, tileRC.right, tileRC.bottom);

	// 여백 주기(상하좌우 2픽셀씩 출력 영역 줄이기)
	RECT rc = tileRC;
	InflateRect(&rc, -2, -2);

	// DT_CENTER : 가로 가운데 정렬, DT_VCENTER : 세로 가운데 정렬, DT_WORDBREAK : 줄바꿈/개행 포함해서 영역 안에 여러 줄로 만들어 처리, 
	// DT_NOPREFIX : & 같은 문자 단축키 밑줄로 해석하지 않고 바로 출력
	DrawTextW(memdc, buf, -1, &rc, DT_CENTER | DT_VCENTER | DT_WORDBREAK | DT_NOPREFIX);

	// 클리핑 해제하여 그리기 작업 정상으로 원복
	RestoreDC(memdc, saved);
}

