#include <windows.h>
#include <vector>
#include "Map.h"

Map::~Map()
{
}


void Map::Init(int width, int height)
{
	m_width = width;
	m_height = height;
	m_tileMap.assign(height, std::vector<TileType>(width));

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			m_tileMap[y][x] = TileType::Empty;
		}
	}
}

void Map::InitStart()
{
	m_start.m_xpos = -1;
	m_start.m_ypos = -1;
}

void Map::InitGoal()
{
	m_goal.m_xpos = -1;
	m_goal.m_ypos = -1;
}

void Map::MapClear()
{
	for (int y = 0; y < m_height; y++)
	{
		for (int x = 0; x < m_width; x++)
		{
			m_tileMap[y][x] = TileType::Empty;
		}
	}
}
