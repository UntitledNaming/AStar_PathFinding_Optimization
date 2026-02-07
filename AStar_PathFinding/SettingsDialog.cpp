#include <windows.h>
#include <vector>
#include "resource1.h"
#include "Map.h"
#include "MapGenerator.h"
#include "Renderer.h"
#include "PathFinder.h"
#include "AppController.h"
#include "SettingsDialog.h"

void FillCombo(HWND hDlg)
{

}

// WndProc에서는 함수 포인터만 등록하지만 OS가 매개인자를 전달해줌.
INT_PTR SettingsDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		// lParam으로 전달된 윈도우 객체 핸들을 다이얼로그에 저장하기
		SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);

		// 콤보 박스 핸들 얻기(다이얼로그가 부모 역할하고 다이얼로그안에 콤보 박스 찾기 위해 매개인자로 콤보 박스 ID전달)
		HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_MODE);

		// 항목 추가
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"수동");
		SendMessage(hCombo, CB_ADDSTRING, 0, (LPARAM)L"자동");

		// 콤보 박스 0번 항목 강제 지정
		SendMessage(hCombo, CB_SETCURSEL, 0, 0);

		// 드롭다운 높이 확 늘리기 (핵심)
		RECT rc{};                         // 사각형 좌표 담을 구조체 생성
		GetWindowRect(hCombo, &rc);        // 콤보 박스의 화면 좌표를 rc에 저장
		int w = rc.right - rc.left;        // 현재 콤보 박스의 가로를 계산
		SetWindowPos(hCombo, NULL, 0, 0, w, 200, SWP_NOMOVE | SWP_NOZORDER); // 윈도우의 위치와 크기 순서를 바꾸는 API
		                                                                     // 3, 4번째 인자는 윈도우 이동 시킬 좌표인데 우선 0,0으로 선택
		                                                                     // 윈도우의 크기를 가로 w, 세로 200으로 변경(세로만 키움)
		                                                                     // SWP_NOMOVE : 윈도우 위치 이동하지 마라 / SWP_NOZORDER : 윈도우 창 순서 바꾸지 마라
		return true;
	}

	// 다이얼로그 안에서 사용자가 특정 버튼 누름 등 액션 발생시 처리되는 이벤트
	// wParam에 여러 정보들 섞여 있음. 하위 16bit(LOWORD)에는 메뉴ID, 버튼 ID, 컨트롤 ID를 담고 있음.
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
			// OK 버튼 눌렸을 때 처리
		case IDOK:
		{
			// 콤보 박스에서 값 얻기
			// 콤보 박스 핸들 얻기
			HWND hCombo = GetDlgItem(hDlg, IDC_COMBO_MODE);

			// 콤보 박스의 항목 리스트가 현재 사용자가 선택한 것인지 얻기(0번째 인덱스 부터 시작)
			int sel = (int)SendMessage(hCombo,CB_GETCURSEL, 0, 0);

			if (sel == CB_ERR)
			{
				MessageBox(hDlg, L"모드를 선택하세요.", L"경고", MB_ICONWARNING);
				return true;
			}

			// Edit Controller Box에서 값 얻기
			BOOL ok = false;
			int w = (int)GetDlgItemInt(hDlg, IDC_EDIT_W, &ok, false);   // 매개인자로 다이얼로그 핸들, EDIT 컨트롤 박스 ID, 변환 성공 여부 얻을 아웃 파라미터, 음수 허용/미허용
			if (!ok)
			{
				MessageBox(hDlg, L"가로 값을 입력하세요", L"Error", MB_OK);
				return true;
			}

			int h = (int)GetDlgItemInt(hDlg, IDC_EDIT_H, &ok, false);
			if (!ok)
			{
				MessageBox(hDlg, L"세로 값을 입력하세요", L"Error", MB_OK);
				return true;
			}


			HWND hwnd = (HWND)GetWindowLongPtr(hDlg, DWLP_USER);

			AppController* pController = (AppController*)GetWindowLongPtr(hwnd, GWLP_USERDATA);

			// 인자 전달할 setting 구조체 초기화
			AppController::settings set;

			if (sel == 0)
			{
				set.m_mode = AppController::RunMode::Manual;
			}

			else if (sel == 1)
			{
				set.m_mode = AppController::RunMode::Auto;
			}

			set.m_mapHeight = h;
			set.m_mapWidth = w;

			pController->Init(set, hwnd);

			// 다이얼로그 창 닫기
			EndDialog(hDlg, IDCANCEL);
			return true;
		}

		case IDCANCEL:
			EndDialog(hDlg, IDCANCEL);
			return true;

		}

		break;
	}


	return false;
}

