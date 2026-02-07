#pragma once
class Renderer
{
public:
	Renderer() = default;
	~Renderer();

	void Init(HWND hWnd);
	void Render(HWND hWnd, const Map& map);

private:
	void RenderPen(HDC memdc, const Map& map);
	void RenderBrush(HDC memdc, const Map& map);

private:
	INT     m_gridSize;      // 타일 1칸이 화면에서 몇 픽셀인지의 크기
	HBITMAP m_bitMap;        // 비트맵(실제 픽셀 저장소) 핸들, 메모리 dc에 한 GDI(그리기 등) 작업이 비트맵에 저장되고 BitBlt를 할 때 선택된 비트맵 리소스에서 픽셀 값들 다 읽어서 화면으로 출력
	HBITMAP m_bitMapOld;     // 메모리 DC에서 BITMAP을 빼내기 위해 필요한 변수
	HDC     m_memDC;         // 메모리 DC
	HPEN    m_gridPen;       // 그리드 그릴 펜 오브젝트
	HPEN    m_pathPen;       // 경로   그릴 펜 오브젝트
	HBRUSH  m_tileBrush;     // 타일 브러쉬
	RECT    m_rc;            // 윈도우 클라이언트 영역(출력 영역) 구조체
};

