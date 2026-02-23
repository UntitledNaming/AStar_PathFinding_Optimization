#pragma once

template <typename T>
struct MinCmp
{
	bool operator()(const T& a, const T& b) const
	{
		return a > b;
	}

};

template <typename T>
struct MaxCmp
{
	bool operator()(const T& a, const T& b) const
	{
		return a < b;
	}
};

template <typename T, typename Compare>

class PriorityQueue
{
public:
	PriorityQueue()
	{
		m_size = 0;
		m_capacity = 0;
		m_allocPtr = nullptr;
	}

	~PriorityQueue()
	{
		if (m_allocPtr)
		{
			delete[] m_allocPtr;
			m_allocPtr = nullptr;
		}
	}

	void push(const T& value)
	{
		// 꽉 찼으면 재할당
		if (m_capacity == m_size)
		{
			assign(m_capacity * 2 + 1);
		}

		// 데이터 삽입 
		int idx = m_size;     // value 넣을 위치 찾는 index (새로 넣을 값이 힙 제일 마지막에 있다 가정)

		// 해당 위치부터 부모와 계속 비교해서 실제 위치 찾기
		while (true)
		{
			// 부모 계산
			int parentidx = (idx - 1) / 2;

			// 내가 루트까지 왔으면 그 위치에 저장하고 탈출
			// 부모와 value 비교하기(최소힙인 경우 부모가 value보다 작은 경우 if 만족)
			if (idx <= 0 || !m_cmp(m_allocPtr[parentidx], value))
			{
				// 자식 위치에 value 넣기
				m_allocPtr[idx] = value;
				break;
			}

			// 부모와 위치 변경(최소힙인 경우 부모보다 value가 더 작은 경우)
			m_allocPtr[idx] = m_allocPtr[parentidx];

			// 부모 위치로 이동
			idx = parentidx;

		}

		m_size++;
	}

	void pop()
	{
		// 데이터 삭제
		
		// 힙 배열의 마지막 원소를 루트 노드에 있다고 가정하고 자식들과 비교하면서 위치 찾기
		int idx = 0;

		if (m_size == 1)
		{
			m_size = 0;
			return;
		}

		while (true)
		{
			// 자식 찾기
			int leftidx = idx * 2 + 1;   // 왼쪽 자식

			// 왼쪽 자식이 범위에 벗어났으면(없으면)
			if (leftidx >= m_size - 1)
			{
				m_allocPtr[idx] = m_allocPtr[m_size - 1];
				break;
			}

			int rightidx = idx * 2 + 2;  // 오른쪽 자식

			// 오른쪽 자식이 없는 경우
			if (rightidx >= m_size - 1)
			{
				// 왼쪽 자식과 나 사이 비교
				if (m_cmp(m_allocPtr[leftidx], m_allocPtr[m_size - 1]))
				{
					// 최소힙인 경우 내가 더 작으면
					m_allocPtr[idx] = m_allocPtr[m_size - 1];
					break;
				}

				// 내 자리에 왼쪽자식이 오고
				m_allocPtr[idx] = m_allocPtr[leftidx];

				// 왼쪽 자식에 내 값 넣기
				m_allocPtr[leftidx] = m_allocPtr[m_size - 1];
				break;
			}


			// 오른쪽 자식이 있는 경우 왼쪽 자식과 오른쪽 자식 비교
			int lastchild = 0;
			if (m_cmp(m_allocPtr[leftidx], m_allocPtr[rightidx]))
			{
				lastchild = rightidx;
			}
			else
				lastchild = leftidx;

			// last 자식과 나를 비교
			if (m_cmp(m_allocPtr[lastchild], m_allocPtr[m_size - 1]))
			{
				// 최소힙인 경우 last 자식보다 내가 더 작은 경우
				m_allocPtr[idx] = m_allocPtr[m_size - 1];
				break;
			}

			// last child를 idx 위치로 이동
			m_allocPtr[idx] = m_allocPtr[lastchild];

			// 자식 위치로 이동해야함
			idx = lastchild;
		}
		
		m_size--;
	}

	T top() const
	{
		return m_allocPtr[0];
	}

	bool empty() const
	{
		return m_size == 0;
	}

	// 해당 갯수만큼 배열 할당하여 기존 배열 옮기는 함수
	bool assign(int newCapacity)
	{
		if (m_capacity >= newCapacity)
			return false;

		T* newptr = new T[newCapacity];
		m_capacity = newCapacity;

		if (m_allocPtr)
			delete[] m_allocPtr;

		m_allocPtr = newptr;
	}

	// 기존 메모리 재활용해서 pq 초기화 하는 함수
	void clear()
	{
		m_size = 0;
	}


private:
	T*       m_allocPtr;  // 할당 주소
	Compare  m_cmp;       // 비교자
	INT      m_size;      // 배열에 유효한 원소 갯수
	INT      m_capacity;  // 배열 할당 원소 갯수
};