#define PROFILE
#include <windows.h>
#include <queue>
#include <vector>
#include <limits>
#include <string>
#include "PriorityQueue.h"
#include "ProfilerTLS.h"
#include "Map.h"
#include "PathFinder.h"

PathFinder::~PathFinder()
{
	if (m_result)
	{
		Tile* p = m_result[0];
		delete[] p;
		delete[] m_result;
		m_result = nullptr;
	}
}

void PathFinder::Init(int width, int height)
{
	// Tile* 포인터 변수 저장할 배열을 생성
	m_result = new Tile*[height];

	// w * h 크기의 Tile 구조체를 생성
	Tile* data = new Tile[width * height];

	for (int i = 0; i < height; i++)
	{
		m_result[i] = data + width * i;
	}

	m_openlist.assign(10000);
	m_pathFlag = false;
	m_width = width;
	m_height = height;
}

bool PathFinder::PathFinding(int sXpos, int sYpos, int gXpos, int gYpos, const Map& map, bool automode)
{
	if (automode)
	{
		if (!ValidateGoal(sXpos, sYpos, gXpos, gYpos, map))
			return false;
	}

	CProfiler profiler(L"PathFinding");

	Node start{ 0 };
	int dx = abs(sXpos - gXpos);
	int dy = abs(sYpos - gYpos);

	start.m_xpos = sXpos;
	start.m_ypos = sYpos;
	start.m_fVal = 1 * (dx + dy) + (1.5 - 2 * 1) * min(dx, dy);
	start.m_gVal = 0;

	m_result[sYpos][sXpos].m_pXpos = sXpos;
	m_result[sYpos][sXpos].m_pYpos = sYpos;

	m_openlist.push(start);

	while (true)
	{
		// 목적지 확인안된 상태에서 더 이상 노드를 방문해서 정점 발견할수 없는 경우 false 리턴
		if (m_openlist.empty())
			return false;

		Node cur = m_openlist.top();
		m_openlist.pop();


		if (cur.m_xpos == gXpos && cur.m_ypos == gYpos)
		{
			// closelist에 목적지 넣기
			m_result[cur.m_ypos][cur.m_xpos].m_close = true;
			break;
		}

		if ((cur.m_fVal != m_result[cur.m_ypos][cur.m_xpos].m_fVal) && (m_result[cur.m_ypos][cur.m_xpos].m_fVal != -1))
			continue;


		// 정점 방문
		m_result[cur.m_ypos][cur.m_xpos].m_close = true;

		// 정점 주위 8방향 탐색
		for (int i = 0; i < 8; i++)
		{
			Search((Dir)i, cur.m_xpos, cur.m_ypos, gXpos, gYpos, cur.m_fVal, cur.m_gVal, map);
		}

	}

	m_pathFlag = true;

	return true;
}

void PathFinder::PathClear()
{
	// 길찾기 제대로 안되고 초기화 하는 경우 리턴
	if (!m_pathFlag)
		return;

	CProfiler profiler(L"PathClear");
	m_pathFlag = false;

	
	m_openlist.clear();

	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			m_result[y][x].m_close = false;
			m_result[y][x].m_fVal = -1;
			m_result[y][x].m_gVal = -1;
			m_result[y][x].m_hVal = -1;
			m_result[y][x].m_pXpos = -1;
			m_result[y][x].m_pYpos = -1;
		}
	}
}

void PathFinder::Search(Dir dir, int xpos, int ypos, int gxpos, int gypos, float fVal, float gVal, const Map& map)
{
	int x{ 0 };
	int y{ 0 };
	float newG = 0;
	float h = 0;
	int   dx = 0;
	int   dy = 0;

	// 초기화 작업 제거 위해 static으로 선언 
	static int   dir_x[8] = { 0,0,1,-1,-1,1,1,-1 };
	static int   dir_y[8] = { -1,1,0,0,-1,-1,1,1 };
	static float cost[8] = { 1,1,1,1,1.5,1.5,1.5,1.5 };

	x = xpos + dir_x[(int)dir];
	y = ypos + dir_y[(int)dir];

	// 범위 체크
	if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		return;

	// 해당 좌표에 벽 있는지 체크
	if (map.GetTilePosType(x, y) == Map::TileType::Wall)
		return;

	// 이미 pq에서 뽑혀서 확정된 노드면 굳이 openlist에 넣을 필요없음. 최소의 F가 이미 나온 상태임.
	if (m_result[y][x].m_close)
		return;

	// g 계산
	newG = gVal + cost[(int)dir];

	// 처음 본 좌표도 아닌데 계산한 gVal이 기존 gVal보다 이상이면 굳이 OpenList에 넣지 않고 리턴
	if (newG >= m_result[y][x].m_gVal && m_result[y][x].m_gVal != -1)
		return;

	// h 계산
	dx = abs(x - gxpos);
	dy = abs(y - gypos);
	h = 1 * (dx + dy) + (1.5 - 2 * 1) * min(dx, dy);


	m_result[y][x].m_gVal = newG;
	m_result[y][x].m_hVal = h;
	m_result[y][x].m_fVal = newG + h;
	m_result[y][x].m_pXpos = xpos;
	m_result[y][x].m_pYpos = ypos;

	Node next{ 0 };
	next.m_gVal = newG;
	next.m_fVal = newG + h;
	next.m_xpos = x;
	next.m_ypos = y;
	m_openlist.push(next);

}

void PathFinder::Search(Dir dir, int xpos, int ypos, const Map& map, std::queue<std::pair<int,int>>& que, std::vector<std::vector<bool>>& search)
{
	int x{ 0 };
	int y{ 0 };

	switch (dir)
	{
	case Dir::N:
	{
		x = xpos;
		y = ypos - 1;
	}
	break;

	case Dir::S:
	{
		x = xpos;
		y = ypos + 1;
	}
	break;

	case Dir::E:
	{
		x = xpos + 1;
		y = ypos;
	}
	break;

	case Dir::W:
	{
		x = xpos - 1;
		y = ypos;
	}
	break;

	case Dir::NE:
	{
		x = xpos + 1;
		y = ypos - 1;
	}
	break;

	case Dir::NW:
	{
		x = xpos - 1;
		y = ypos - 1;
	}
	break;

	case Dir::SE:
	{
		x = xpos + 1;
		y = ypos + 1;
	}
	break;

	case Dir::SW:
	{
		x = xpos - 1;
		y = ypos + 1;
	}
	break;

	default:
		return;
	}

	// 범위 체크
	if (x < 0 || x >= m_width || y < 0 || y >= m_height)
		return;

	// 해당 좌표에 벽 있는지 체크
	if (map.GetTilePosType(x, y) == Map::TileType::Wall)
		return;

	// 탐색 했는지 체크
	if (search[y][x])
		return;

	search[y][x] = true;
	que.push(std::pair<int, int>(x, y));
}

bool PathFinder::ValidateGoal(int sxpos, int sypos, int gxpos, int gypos, const Map& map)
{
	bool ok = false;
	std::queue<std::pair<int, int>> q;
	std::vector<std::vector<bool>> visit(m_height, std::vector<bool>(m_width, false));

	// 시작점 넣기
	q.push(std::pair<int, int>(sxpos, sypos));

	while (!q.empty())
	{
		std::pair<int, int> pos = q.front();
		q.pop();

		if (pos.first == gxpos && pos.second == gypos)
		{
			ok = true;
			break;
		}

		// 8방향 좌표 탐색
		for (int i = 0; i < 8; i++)
		{
			Search((Dir)i, pos.first, pos.second, map, q, visit);
		}
	}

	if (!ok)
		return false;

	return true;
}
