#include <windows.h>
#include <time.h>
#include <vector>
#include "Map.h"
#include "MapGenerator.h"

MapGenerator::~MapGenerator()
{
}

void MapGenerator::init(int width, int height)
{
	srand(time(NULL));

	m_probBoard.assign(height, std::vector<Probability>(width));

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			Probability prob = rand() % 101; // 타일에 설정할 확률(0 ~ 100 %)
			m_probBoard[y][x] = prob;
		}
	}

}

void MapGenerator::GenProbWall(Map& map)
{
	int w = map.GetWidth();
	int h = map.GetHeight();

	for (int y = 0; y < h; y++)
	{
		for (int x = 0; x < w; x++)
		{
			if (m_probBoard[y][x] == 0)
				continue;

			else if (m_probBoard[y][x] == 100)
			{
				map.SetWall(Map::TilePos{ x,y });
			}

			// 랜덤값 얻어서 해당 타일의 확률 범위 안에 들어가면 벽 생성
			int val = rand() % 99 + 1; // 1 ~ 99 중 랜덤값 생성

			if (val >= 1 && val <= m_probBoard[y][x])
			{
				map.SetWall(Map::TilePos{ x,y });
			}

		}
	}

}
