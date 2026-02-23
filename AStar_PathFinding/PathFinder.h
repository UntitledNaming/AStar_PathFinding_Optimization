#pragma once
class PathFinder
{
public:
	struct st_Tile
	{
		int     m_pXpos = -1;      // 해당 타일의 부모 타일 x 좌표
		int     m_pYpos = -1;      // 해당 타일의 부모 타일 y 좌표
		bool    m_close = false;   // close에 들어간 경우 true
		float   m_gVal = -1;          // G : 유클리드(대각 1.5)
		float   m_hVal = -1;          // H : 유클리드
		float   m_fVal = -1;          // F = G + H

	}typedef Tile;

	struct st_Node
	{
		int    m_xpos;
		int    m_ypos;
		float  m_gVal;
		float  m_fVal;


		bool operator>(const st_Node& other) const
		{
			return m_fVal > other.m_fVal;
		}

	}typedef Node;

	enum class Dir
	{
		N = 0,
		S,
		E,
		W,
		NW,
		NE,
		SE,
		SW,
	};
public:
	PathFinder() = default;
	PathFinder(const PathFinder&) = delete;
	PathFinder& operator=(const PathFinder&) = delete;
	~PathFinder();

	void Init(int width, int height);

	bool PathFinding(int sXpos, int sYpos, int gXpos, int gYpos, const Map& map, bool automode = false);

	// 호출자가 멤버인 m_closelist를 수정하지 못하게 const 리턴
	inline Tile** GetResult() const
	{
		return m_result;
	}

	inline bool GetPathFlag() const noexcept
	{
		return m_pathFlag;
	}

	void PathClear(); 

private:
	void Search(Dir dir, int xpos, int ypos, int gxpos, int gypos, float fVal, float gVal, const Map& map);
	void Search(Dir dir, int xpos, int ypos,  const Map& map , std::queue<std::pair<int, int>>& que, std::vector<std::vector<bool>>& search);
	bool ValidateGoal(int sxpos, int sypos, int gxpos, int gypos, const Map& map);                    // 목적지 좌표로 길찾기가 가능한지 BFS로 선행 체크하는 함수

private:
	bool                                                                  m_pathFlag;                 // 길찾기 결과가 나왔는지 확인 플래그
	int                                                                   m_width;	                  
	int                                                                   m_height;	                  

	PriorityQueue<Node,MinCmp<Node>>                                      m_openlist;
	Tile**                                                                m_result;
};

