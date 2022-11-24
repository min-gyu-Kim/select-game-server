#include "framework.h"

#include "Util/RingBuffer.h"
#include "Util/Packet.h"
#include "Network/Protocol.h"
#include "Network/Network.h"
#include "Contents/Contents.h"

#define SEED        3000

#ifdef __WINDBG__
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
HWND g_hWnd;

// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    timeBeginPeriod(1);//timegettime
    srand(SEED);
    if (!InitNetwork())
    {
        wprintf(L"InitNetwork() error!\n");
        return -1;
    }

    MyRegisterClass(hInstance);

    // 애플리케이션 초기화를 수행합니다:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HDC hdc = GetDC(g_hWnd);
    g_memDC = CreateCompatibleDC(hdc);
    g_bitmap = CreateCompatibleBitmap(hdc, 1920, 1080);
    SelectObject(g_memDC, g_bitmap);
    ReleaseDC(g_hWnd, hdc);
    g_whitePen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

    SelectObject(g_memDC, GetStockObject(NULL_PEN));
    SelectObject(g_memDC, GetStockObject(BLACK_BRUSH));

    MSG msg;
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            ProcessNetwork();
            Update();

            //그리기
            HDC dc = GetDC(g_hWnd);
            BitBlt(dc, 0, 0, 1920, 1080, g_memDC, 0, 0, SRCCOPY);
            ReleaseDC(g_hWnd, dc);
        }
    }

    DeleteObject(g_whitePen);
    DeleteObject(g_bitmap);
    DeleteObject(g_memDC);

    ShutdownNetwork();
    timeEndPeriod(0);

    return (int) msg.wParam;
}

//
//  함수: MyRegisterClass()
//
//  용도: 창 클래스를 등록합니다.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = L"GameServer";
    wcex.hIconSm        = wcex.hIcon;

    return RegisterClassExW(&wcex);
}

//
//   함수: InitInstance(HINSTANCE, int)
//
//   용도: 인스턴스 핸들을 저장하고 주 창을 만듭니다.
//
//   주석:
//
//        이 함수를 통해 인스턴스 핸들을 전역 변수에 저장하고
//        주 프로그램 창을 만든 다음 표시합니다.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 인스턴스 핸들을 전역 변수에 저장합니다.

   g_hWnd = CreateWindowW(L"GameServer", L"Monitor", WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, 700, 700, nullptr, nullptr, hInstance, nullptr);

   if (!g_hWnd)
   {
      return FALSE;
   }

   ShowWindow(g_hWnd, nCmdShow);
   UpdateWindow(g_hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 여기에 hdc를 사용하는 그리기 코드를 추가합니다...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

#else
#include <conio.h>
#include "Util/Profile.h"
bool g_isRun = true;

int main()
{
    timeBeginPeriod(1);//timegettime
    srand(SEED);
    if (!InitNetwork())
    {
        wprintf(L"InitNetwork() error!\n");
        return -1;
    }

    while (g_isRun)
    {
        ProcessNetwork();
        Update();
        /*
        if (_kbhit())
        {
            break;
        }
        */
    }
        
    SAVE_PROFILE_DATA(L"Debug");
    ShutdownNetwork();
    timeEndPeriod(0);
    return 0;
}

#endif