#include <windows.h>
#include <vector>
#include "framework.h"
#include "Map.h"
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
	DeleteObject(m_tileBrush);
	DeleteObject(m_pathPen);
}

void Renderer::Init(HWND hWnd)
{
	HDC hdc = GetDC(hWnd); // 윈도우 클라이언트 영역에 그릴 수 있는 hdc를 받기(대여)
	GetClientRect(hWnd, &m_rc);

	m_gridSize = 16;
	m_bitMap = CreateCompatibleBitmap(hdc, m_rc.right, m_rc.bottom);
	m_memDC = CreateCompatibleDC(hdc);
	ReleaseDC(hWnd, hdc); // hdc os에게 반납

	// 메모리 DC에 생성한 비트맵을 연결하고 기존에 연결되어 있던 비트맵을 리턴해서 받음
	m_bitMapOld = (HBITMAP)SelectObject(m_memDC, m_bitMap);

	// 펜 및 브러시 생성
	m_gridPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
	m_pathPen = CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
	m_tileBrush = CreateSolidBrush((RGB(100, 100, 100)));
	
}

void Renderer::Render(HWND hWnd, const Map& map)
{
	HDC hdc = GetDC(hWnd);

	PatBlt(m_memDC, 0, 0, m_rc.right, m_rc.bottom, WHITENESS);

	RenderPen(m_memDC, map);
	RenderBrush(m_memDC, map);

	BitBlt(hdc, 0, 0, m_rc.right, m_rc.bottom, m_memDC, 0, 0, SRCCOPY);

	ReleaseDC(hWnd, hdc);
}

void Renderer::RenderPen(HDC memdc, const Map& map)
{

	int iX = 0;
	int iY = 0;
	HPEN hOldPen = (HPEN)SelectObject(memdc, m_gridPen);

	for (int iCntW = 0; iCntW <= map.GetWidth(); iCntW++)
	{
		// 펜을 (iX, 0) 으로 이동
		MoveToEx(memdc, iX, 0, NULL);

		// 펜으로 (iX, 0) 에서 (iX, Height * gridSize)로 줄 긋기(세로 1줄 그리기)
		LineTo(memdc, iX, map.GetHeight() * m_gridSize);

		// iX를 타일 1칸 크기 만큼 증가 시키기
		iX += m_gridSize;
	}

	for (int iCntH = 0; iCntH <= map.GetHeight(); iCntH++)
	{
		MoveToEx(memdc, 0, iY, NULL);
		LineTo(memdc, map.GetWidth() * m_gridSize, iY);
		iY += m_gridSize;
	}

	SelectObject(memdc, hOldPen);

	// todo : 경로 줄 그리기
}

void Renderer::RenderBrush(HDC memdc, const Map& map)
{

}

