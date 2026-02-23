#pragma once
class Map
{
public:
	enum class TileType : CHAR
	{
		Empty = 0,
		Wall,
		Start,
		Goal
	};

	struct st_TilePos
	{
		INT m_xpos = -1;
		INT m_ypos = -1;
	}typedef TilePos;

public:
	Map() = default;
	~Map();

	void Init(int width, int height);
	void InitStart();
	void InitGoal();
	void MapClear();

	bool RangeCheck(int tilex, int tiley)
	{
		if (tilex < 0 || tilex >= m_width || tiley < 0 || tiley >= m_height)
			return false;

		return true;
	}

	// const noexcept : 멤버 변수 값 안바꾸고 예외 던지지 않는것
	inline INT GetWidth() const noexcept
	{
		return m_width;
	}

	inline INT GetHeight() const noexcept
	{
		return m_height;
	}

	inline TilePos GetStartPos() const noexcept
	{
		return m_start;
	}

	inline TilePos GetGoalPos() const noexcept
	{
		return m_goal;
	}

	inline TileType GetTilePosType(int x, int y) const noexcept
	{
		return m_tileMap[y][x];
	}

	void SetStartPos(int startX, int startY)
	{
		m_start.m_xpos = startX;
		m_start.m_ypos = startY;
		m_tileMap[startY][startX] = TileType::Start;
	}

	void SetGoalPos(int goalX, int goalY)
	{
		m_goal.m_xpos = goalX;
		m_goal.m_ypos = goalY;
		m_tileMap[goalY][goalX] = TileType::Goal;
	}

	void SetWall(TilePos wall)
	{
		m_tileMap[wall.m_ypos][wall.m_xpos] = TileType::Wall;
	}

	void SetEmpty(TilePos empty)
	{
		m_tileMap[empty.m_ypos][empty.m_xpos] = TileType::Empty;
	}


private:
	std::vector<std::vector<TileType>> m_tileMap; // 시작점과 종점
	INT                                m_width;
	INT                                m_height;
	TilePos                            m_start;   // 쉬프트 + 우클릭 
	TilePos                            m_goal;    // 알트   + 우클릭
};