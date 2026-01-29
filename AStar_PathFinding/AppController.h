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
		RunMode s_mode      = RunMode::Manual;
		int     s_mapWidth  = 150;
		int     s_mapHeight = 100;
		int     s_gridSize  = 5;
	};

public:
	AppController() = default;
	~AppController();



private:
	int          m_mode; // 荐悼 甘 积己, 磊悼 甘 积己
	Map          m_map;
	MapGenerator m_generator;
	Renderer     m_renderer;
	PathFinder   m_pathfinder;
};

