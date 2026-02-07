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
	m_tileMap.assign(height, std::vector<CHAR>(width));

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			m_tileMap[y][x] = (CHAR)TileType::Empty;
		}
	}
}
