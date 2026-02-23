#pragma once

typedef int Probability;

class MapGenerator
{
public:
	MapGenerator() = default;
	~MapGenerator();

	void init(int width, int height);
	void GenProbWall(Map& map);

private:
	std::vector<std::vector<Probability>> m_probBoard; // 똑같은 타일 맵에 각 타일 마다 확률 다르게 설정해서 벽 생성

};

