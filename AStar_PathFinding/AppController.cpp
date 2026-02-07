#include <windows.h>
#include <vector>
#include "Map.h"
#include "MapGenerator.h"
#include "Renderer.h"
#include "PathFinder.h"
#include "AppController.h"

AppController::~AppController()
{

}

void AppController::Init(const settings& src, HWND hWnd)
{
	m_mode = src.m_mode;

	m_map.Init(src.m_mapWidth, src.m_mapHeight);
	m_renderer.Init(hWnd);
}

void AppController::OnPaint(HWND hWnd)
{
	m_renderer.Render(hWnd, m_map);
}

void AppController::OnMouseWheel(HWND hwnd, int wheelDelta, int mouseX, int mouseY)
{

}

void AppController::OnLButtonDown()
{
}

