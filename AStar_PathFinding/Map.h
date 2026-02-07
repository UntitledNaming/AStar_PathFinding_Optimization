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

	// const noexcept : 멤버 변수 값 안바꾸고 예외 던지지 않는것
	INT GetWidth() const noexcept
	{
		return m_width;
	}

	INT GetHeight() const noexcept
	{
		return m_height;
	}

private:
	std::vector<std::vector<CHAR>> m_tileMap; // 시작점과 종점
	INT                            m_width;
	INT                            m_height;
	TilePos                        m_start;
	TilePos                        m_goal;
};

