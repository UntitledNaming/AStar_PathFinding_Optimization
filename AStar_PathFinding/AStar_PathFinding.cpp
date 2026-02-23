#include <windows.h>
#include <vector>
#include <queue>
#include <windowsx.h>
#include "resource1.h"
#include "SettingsDialog.h"
#include "Map.h"
#include "MapGenerator.h"
#include "PriorityQueue.h"
#include "PathFinder.h"
#include "Renderer.h"
#include "AppController.h"
#include "framework.h"
#include "AStar_PathFinding.h"

#define MAX_LOADSTRING 100

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, AppController* p);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);


    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ASTARPATHFINDING, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 앱 컨트롤러 생성
    AppController* p = new AppController;


    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow, p))
    {
        return FALSE;
    }
    
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASTARPATHFINDING));

    MSG msg;

    // 기본 메시지 루프입니다:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    // 앱 컨트롤러 객체 삭제
    delete p;
    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASTARPATHFINDING));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_ASTARPATHFINDING);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, AppController* pController)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   // 윈도우 생성 시 앱 컨트롤러 객체 포인터 저장
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, pController);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    AppController* pController = nullptr;

    switch (message)
    {
    case WM_CREATE:
    {
        // CreateWindow 시 전달한 pController를 cs의 멤버 변수에서 가져와서 SetWindowLongPtr을 통해 저장
        CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
        AppController* pController = (AppController*)cs->lpCreateParams;

        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pController);

        // 리소스에 있는 다이얼로그 템플릿 가지고 실제 다이얼로그 만들어서 띄우고 다이얼로그 프로시저 호출될 수 있게 하는 함수
        // 이 함수 안에서 다이얼로그 생성 및 SettingsDlgProc을 호출함. 메세지 루프를 돌리면서 SettingsDlgProc이 메세지 처리하다가
        // EndDialog가 호출되면 메세지 루프가 끝나고 DialogBoxParam이 리턴함.
        DialogBoxParam(hInst, MAKEINTRESOURCE(IDD_SETTINGS), hWnd, SettingsDlgProc, (LPARAM)hWnd);

        if (pController->GetMode() == AppController::RunMode::Auto)
        {
            PostMessage(hWnd, WM_AUTO_PATHFINDING, 0, 0);
        }
    }
    break;
    
    case WM_MBUTTONDOWN:
    {
        pController = (AppController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        pController->OnMButtonDown(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    break;

    case WM_MBUTTONUP:
    {
        pController = (AppController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        pController->OnMButtonUp();
    }
    break;

    case WM_KEYDOWN:
    {
        pController = (AppController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        pController->OnKeyDown(hWnd,wParam);
    }
    break;

    // lParam : 클라이언트 좌표(윈도우 클라이언트 영역 좌측 상단 0,0 기준)
    case WM_LBUTTONDOWN:
    {
        pController = (AppController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        pController->OnLButtonDown(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    break;

    case WM_LBUTTONUP:
    {
        pController = (AppController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        pController->OnLButtonUp();
    }
    break;

    case WM_RBUTTONDOWN:
    {
        pController = (AppController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        pController->OnRButtonDown(hWnd, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    break;
    
    case WM_MOUSEMOVE:
    {
        pController = (AppController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        pController->OnMouseMove(hWnd, wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    }
    break;

    // lParam : 스크린 좌표(모니터 좌측 상단 0,0 기준)
    case WM_MOUSEWHEEL:
    {
        pController = (AppController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        // 마우스 휠을 작동했을 때 해당 마우스 위치를 기준으로 타일이 확대 및 축소되어서 wall, 경로, 시작정, 종점 등이 보이도록 할 것임.
        // 마우스 휠을 작동했을 때 좌표가 스크린 좌표이므로 이를 클라이언트 좌표로 변환해야 함.
        // 기존 렌더링 방식 : 클라 좌표 0,0 부터 width, height 만큼 gridSize 크기의 타일을 그리고 클라 영역 좌표 0,0 부터 출력함.
        // 변경 렌더링 방식 : 마우스 위치에 있는 타일 좌표를 기준으로 함.
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);

        POINT pt{ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
        ScreenToClient(hWnd, &pt);

        pController->OnMouseWheel(hWnd, delta, pt.x, pt.y);
    }
    break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);

            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_PAINT:
        {
            pController = (AppController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            pController->OnPaint(hWnd);
            EndPaint(hWnd, &ps);

            if (pController->GetMode() == AppController::RunMode::Auto)
            {
                PostMessage(hWnd, WM_AUTO_PATHFINDING, 0, 0);
            }

        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    case WM_AUTO_PATHFINDING:
    {
        pController = (AppController*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        pController->OnAutoPathFinding(hWnd);
    }
    break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
