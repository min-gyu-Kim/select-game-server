#define _WINSOCK_DEPRECATED_NO_WARNINGS 

#include "framework.h"
#include "Game/ScreenDib.h"
#include "Game/SpriteDib.h"
#include "Game/FrameManager.h"
#include "Game/Object/BaseObject.h"
#include "Game/Object/Effect.h"
#include "Game/Object/Player.h"

//#include "Util/PacketDefine.h"
#include "Util/PacketType.h"
#include "Util/List.h"
#include "Util/RingBuffer.h"
#include "Util/NetProc.h"
#include "Camera.h"
#include "Util/Protocol.h"
#include "Domain.h"

#define PORT 10201
#define UM_SOCK (WM_USER + 1)

static HWND g_hWnd;
static HIMC g_hOldIMC;
static bool g_isActive;

bool g_isSend = false;
bool g_isNet = false;
RingBuffer* g_recvBuf;
RingBuffer* g_sendBuf;
SOCKET g_sock;
WCHAR g_IPBuf[128];

List<Effect*> g_effectList;
List<Player*> g_playerList;
Player* g_myPlayer;
BYTE  m_byTileMap[dfMAP_HEIGHT][dfMAP_WIDTH];

bool InitWindow(HINSTANCE hInst);
bool InitNetwork();
BOOL CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
void NetworkProc(WPARAM wparam, LPARAM lparam);
void Disconnect();

void NetRecvProc();
void NetSendProc();

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void AdjustSizeWindow();

void Logic(void);
void Input();
void Update();
void Render();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    timeBeginPeriod(1);

    if (!InitWindow(hInstance))
    {
        return -1;
    }

    if (!InitNetwork())
    {
        return -1;
    }

    srand(time(0));

    bool result = ScreenDib::GetInstance()->Initialize(640, 480, 32);
    {
        SpriteDib::GetInstance()->Initiailze(68, RGB(255, 255, 255));
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Tile_01.bmp", 0, POINT{ 0, 0 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack1_L_01.bmp", 1, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack1_L_02.bmp", 2, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack1_L_03.bmp", 3, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack1_L_04.bmp", 4, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack1_R_01.bmp", 5, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack1_R_02.bmp", 6, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack1_R_03.bmp", 7, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack1_R_04.bmp", 8, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack2_L_01.bmp", 9, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack2_L_02.bmp", 10, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack2_L_03.bmp", 11, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack2_L_04.bmp", 12, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack2_R_01.bmp", 13, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack2_R_02.bmp", 14, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack2_R_03.bmp", 15, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack2_R_04.bmp", 16, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_L_01.bmp", 17, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_L_02.bmp", 18, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_L_03.bmp", 19, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_L_04.bmp", 20, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_L_05.bmp", 21, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_L_06.bmp", 22, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_R_01.bmp", 23, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_R_02.bmp", 24, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_R_03.bmp", 25, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_R_04.bmp", 26, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_R_05.bmp", 27, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Attack3_R_06.bmp", 28, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/HPGuage.bmp", 29, POINT{ 0, 0 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_01.bmp", 30, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_02.bmp", 31, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_03.bmp", 32, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_04.bmp", 33, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_05.bmp", 34, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_06.bmp", 35, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_07.bmp", 36, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_08.bmp", 37, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_09.bmp", 38, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_10.bmp", 39, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_11.bmp", 40, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_L_12.bmp", 41, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_01.bmp", 42, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_02.bmp", 43, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_03.bmp", 44, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_04.bmp", 45, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_05.bmp", 46, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_06.bmp", 47, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_07.bmp", 48, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_08.bmp", 49, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_09.bmp", 50, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_10.bmp", 51, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_11.bmp", 52, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Move_R_12.bmp", 53, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Shadow.bmp", 54, POINT{ 32, 4 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Stand_L_01.bmp", 55, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Stand_L_02.bmp", 56, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Stand_L_03.bmp", 57, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Stand_L_02.bmp", 58, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Stand_L_01.bmp", 59, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Stand_R_01.bmp", 60, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Stand_R_02.bmp", 61, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Stand_R_03.bmp", 62, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Stand_R_02.bmp", 63, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/Stand_R_01.bmp", 64, POINT{ 71, 90 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/xSpark_1.bmp", 65, POINT{ 70, 70 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/xSpark_2.bmp", 66, POINT{ 70, 70 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/xSpark_3.bmp", 67, POINT{ 70, 70 });
        SpriteDib::GetInstance()->LoadSprite(L"Sprite/xSpark_4.bmp", 68, POINT{ 70, 70 });
    }

    {
        Effect* effect;
        for (int i = 0; i < 5; i++)
        {
            effect = new Effect();
            g_effectList.push_back(effect);
        }
    }
    
    FrameManager::GetInstance()->Initialize();

    MSG msg;

    while (true)
    {
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE))
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
            if(g_isNet)
                Logic();
        }
    }

    {
        List<Effect*>::iterator iter = g_effectList.begin();
        List<Effect*>::iterator iter_end = g_effectList.end();
        for (; iter != iter_end;)
        {
            delete (*iter);
            iter = g_effectList.erase(iter);
        }
    }

    SpriteDib::GetInstance()->ReleaseAll();

    WSACleanup();

    timeEndPeriod(1);

    return (int) msg.wParam;
}

bool InitWindow(HINSTANCE hInst)
{
    WNDCLASSEXW wcex;
    ZeroMemory(&wcex, sizeof(wcex));
    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"2D Game";
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    RegisterClassExW(&wcex);

    g_hWnd = CreateWindowW(L"2D Game", L"Game", WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInst, nullptr);

    if (!g_hWnd)
    {
        DWORD error_code = GetLastError();
        WCHAR buf[256] = { 0, };
        wsprintf(buf, L"%d", error_code);
        MessageBox(g_hWnd, buf, L"error", MB_OK);
        return false;
    }

    ShowWindow(g_hWnd, SW_SHOW);
    UpdateWindow(g_hWnd);
    SetFocus(g_hWnd);
    AdjustSizeWindow();
    /*
    HWND hwnd = GetConsoleWindow();
    if (hwnd)
    {
        ShowWindow(hwnd, SW_HIDE);
    }*/
    return true;
}

bool InitNetwork()
{
    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(2,2), &wsadata)!= 0)
    {
        int errCode = WSAGetLastError();
        WCHAR msg[128];
        wsprintf(msg, L"WSAStartup() error! code : %d", errCode);

        MessageBoxW(nullptr, msg, L"error!", MB_OK);
        return false;
    }

    DialogBox(nullptr, MAKEINTRESOURCE(IDD_DIALOG1), nullptr, DlgProc);
        
    SOCKADDR_IN serverAddr;
    ZeroMemory(&serverAddr, sizeof(serverAddr));
    if (InetPton(AF_INET, g_IPBuf, &serverAddr.sin_addr) != 1)
    {
        if (!DomainToIP(g_IPBuf, &serverAddr.sin_addr, FALSE))
        {
            int errCode = WSAGetLastError();
            WCHAR msg[128];
            wsprintf(msg, L"InetPton() error! code : %d", errCode);

            MessageBoxW(nullptr, msg, L"error!", MB_OK);
            return false;
        }
    }
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_family = AF_INET;

    g_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(g_sock == INVALID_SOCKET)
    {
        int errCode = WSAGetLastError();
        WCHAR msg[128];
        wsprintf(msg, L"socket() error! code : %d", errCode);

        MessageBoxW(nullptr, msg, L"error!", MB_OK);
        return false;
    }

    if (WSAAsyncSelect(g_sock, g_hWnd, UM_SOCK, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE) != 0)
    {
        int errCode = WSAGetLastError();
        WCHAR msg[128];
        wsprintf(msg, L"WSAAsyncSelect() error! code : %d", errCode);

        MessageBoxW(nullptr, msg, L"error!", MB_OK);
        return false;
    }

    if(connect(g_sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        int errCode = WSAGetLastError();
        if (errCode != WSAEWOULDBLOCK)
        {
            WCHAR msg[128];
            wsprintf(msg, L"connect() error! code : %d", errCode);

            MessageBoxW(nullptr, msg, L"error!", MB_OK);
            return false;
        }
    }

    g_recvBuf = new RingBuffer(10000);
    g_sendBuf = new RingBuffer(10000);

    return true;
}

BOOL CALLBACK DlgProc(HWND dlgWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
    {
        RECT rect;
        GetWindowRect(dlgWnd, &rect);
        SetWindowPos(dlgWnd, nullptr,
            (GetSystemMetrics(SM_CXSCREEN) - rect.right - rect.left) / 2,
            (GetSystemMetrics(SM_CYSCREEN) - rect.bottom - rect.top) / 2,
            0, 0, SWP_NOZORDER | SWP_NOSIZE);
        SetDlgItemText(dlgWnd, IDC_IPEDIT, L"127.0.0.1");
        return TRUE;
    }
    case WM_COMMAND:
        switch (wParam)
        {
        case IDOK:
            GetDlgItemText(dlgWnd, IDC_IPEDIT, g_IPBuf, 128);
            EndDialog(dlgWnd, TRUE);
        default:
            break;
        }

        return TRUE;
    default:
        break;
    }
    return FALSE;
}

void NetworkProc(WPARAM wparam, LPARAM lparam)
{
    int errCode = WSAGETSELECTERROR(lparam);
    if (errCode)
    {
        WCHAR msg[128];
        wsprintf(msg, L"connect() error! code : %d", errCode);

        MessageBoxW(nullptr, msg, L"error!", MB_OK);
        Disconnect();
        return;
    }

    int eventType = WSAGETSELECTEVENT(lparam);
    switch (eventType)
    {
    case FD_CONNECT:
        g_isNet = true;
        break;
    case FD_READ:
        NetRecvProc();
        break;
    case FD_WRITE:
        g_isSend = true;
        NetSendProc();
        break;
    case FD_CLOSE:
        Disconnect();
        break;
    default:
        break;
    }
}

void NetRecvProc()
{
    char buffer[1000];
    int errCode;
    int recvSize = min(1000, g_recvBuf->GetFreeSize());
    int recvRet = recv(g_sock, buffer, recvSize, 0);
    if (recvRet == 0)
    {
        Disconnect();
        return;
    }
    else if(recvRet == SOCKET_ERROR)
    {
        errCode = WSAGetLastError();
        if (errCode != WSAEWOULDBLOCK)
        {
            WCHAR msg[128];
            wsprintf(msg, L"recv() error! code : %d", errCode);

            MessageBoxW(nullptr, msg, L"error!", MB_OK);
            Disconnect();
            return;
        }
    }

    g_recvBuf->Enqueue(buffer, recvRet);

    while (1)
    {
        int useSize = g_recvBuf->GetUseSize();
        if (useSize < 4)
            break;

        PacketHeader header;
        g_recvBuf->Peek((char*)&header, sizeof(header));

        if (header.byCode != 0x89)
        {
            Disconnect();
            return;
        }

        if (useSize < sizeof(header) + header.bySize)
        {
            break;
        }

        g_recvBuf->MoveFront(sizeof(header));
        char tmpBuf[12];
        g_recvBuf->Dequeue(tmpBuf, header.bySize);

        switch (header.byType)
        {
        case dfPACKET_SC_CREATE_MY_CHARACTER:
            NetProcCreateMyCharacter(tmpBuf);
            break;
        case dfPACKET_SC_CREATE_OTHER_CHARACTER:
            NetProcCreateOtherCharacter(tmpBuf);
            break;
        case dfPACKET_SC_DELETE_CHARACTER:
            NetProcDeleteCharacter(tmpBuf);
            break;
        case dfPACKET_SC_MOVE_START:
            NetProcMoveStart(tmpBuf);
            break;
        case dfPACKET_SC_MOVE_STOP:
            NetProcMoveStop(tmpBuf);
            break;
        case dfPACKET_SC_ATTACK1:
            NetProcAttack1(tmpBuf);
            break;
        case dfPACKET_SC_ATTACK2:
            NetProcAttack2(tmpBuf);
            break;
        case dfPACKET_SC_ATTACK3:
            NetProcAttack3(tmpBuf);
            break;
        case dfPACKET_SC_DAMAGE:
            NetProcDamage(tmpBuf);
            break;
        default:
            break;
        }
    }
}

void NetSendProc()
{
    while (1)
    {
        if (g_sendBuf->GetUseSize() == 0)
        {
            return;
        }

        char buf[1000];
        int peekRet = g_sendBuf->Peek(buf, 1000);
       
        int sendRet = send(g_sock, buf, peekRet, 0);
        if (sendRet == SOCKET_ERROR)
        {
            int errCode = WSAGetLastError();
            if (errCode == WSAEWOULDBLOCK)
            {
                g_isSend = false;
            }
            else
            {
                WCHAR msg[128];
                wsprintf(msg, L"send() error! code : %d", errCode);

                MessageBoxW(nullptr, msg, L"error!", MB_OK);
                Disconnect();
            }
            return;
        }

        g_sendBuf->MoveFront(sendRet);
    }
}

void Disconnect()
{
    g_isNet = false;
    closesocket(g_sock);
    delete g_recvBuf;
    delete g_sendBuf;

    PostQuitMessage(0);
    MessageBox(nullptr, L"연결 종료", L"알림", MB_OK);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case UM_SOCK:
        NetworkProc(wParam, lParam);
        break;
    case WM_CREATE:
        g_hOldIMC = ImmAssociateContext(g_hWnd, NULL);
        break;
    case WM_ACTIVATEAPP:
        g_isActive = (BOOL)wParam;
        break;

    case WM_DESTROY:
        ImmAssociateContext(g_hWnd, g_hOldIMC);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void AdjustSizeWindow()
{
    RECT window_rect;
    window_rect.left = 0;
    window_rect.top = 0;
    window_rect.right = 640;
    window_rect.bottom = 480;

    AdjustWindowRectEx(&window_rect, GetWindowStyle(g_hWnd), GetMenu(g_hWnd) != NULL, GetWindowExStyle(g_hWnd));

    int position_x = (GetSystemMetrics(SM_CXSCREEN) - 640) / 2;
    int position_y = (GetSystemMetrics(SM_CYSCREEN) - 480) / 2;

    MoveWindow(g_hWnd, position_x, position_y, window_rect.right - window_rect.left, window_rect.bottom - window_rect.top, TRUE);
}

void Logic(void)
{
    FrameManager::GetInstance()->UpdateCount();
    if (g_isActive == true)
    {
        Input();
    }
    Update();
    if (FrameManager::GetInstance()->Update())
    {
        Render();
    }

    {
        wchar_t buf[30];
        HDC hdc = GetDC(g_hWnd);

        SetTextColor(hdc, RGB(255, 51, 00));
        SetBkMode(hdc, TRANSPARENT);
        wsprintf(buf, L"Render FPS: %d", FrameManager::GetInstance()->GetRenderFrame());
        TextOut(hdc, 0, 0, buf, wcslen(buf));

        wsprintf(buf, L"Update FPS: %d", FrameManager::GetInstance()->GetUpdateFrame());
        TextOut(hdc, 0, 15, buf, wcslen(buf));

        if (g_myPlayer)
        {
            wsprintf(buf, L"X: %d, Y: %d", g_myPlayer->GetPosition().x, g_myPlayer->GetPosition().y);
            TextOut(hdc, 0, 30, buf, wcslen(buf));
        }

        ReleaseDC(g_hWnd, hdc);
    }
}

void Input()
{
    DWORD action = -1;
    BYTE horizon_key = 0;

    if (GetAsyncKeyState(VK_LEFT))
    {
        horizon_key = dfDIRECTION_LEFT;
    }

    if (GetAsyncKeyState(VK_RIGHT))
    {
        horizon_key = dfDIRECTION_RIGHT;
    }

    if (GetAsyncKeyState(VK_UP))
    {
        if (horizon_key == dfDIRECTION_LEFT)
        {
            action = dfACTION_MOVE_LU;
        }
        else if(horizon_key == dfDIRECTION_RIGHT)
        {
            action = dfACTION_MOVE_RU;
        }
        else
        {
            action = dfACTION_MOVE_UU;
        }
    }

    if (GetAsyncKeyState(VK_DOWN))
    {
        if (horizon_key == dfDIRECTION_LEFT)
        {
            action = dfACTION_MOVE_LD;
        }
        else if (horizon_key == dfDIRECTION_RIGHT)
        {
            action = dfACTION_MOVE_RD;
        }
        else
        {
            action = dfACTION_MOVE_DD;
        }
    }

    if (action == -1)
    {
        if (horizon_key == dfDIRECTION_LEFT)
        {
            action = dfACTION_MOVE_LL;
        }
        else if (horizon_key == dfDIRECTION_RIGHT)
        {
            action = dfACTION_MOVE_RR;
        }
    }

    if (GetAsyncKeyState('Z'))
    {
        action = dfACTION_ATTACK1;
    }
    if (GetAsyncKeyState('X'))
    {
        action = dfACTION_ATTACK2;
    }
    if (GetAsyncKeyState('C'))
    {
        action = dfACTION_ATTACK3;
    }

    if(g_myPlayer)
        g_myPlayer->ActionInput(action);
}

bool comp(Player* a, Player* b)
{
    return a->GetPosition().y > b->GetPosition().y;
}

void Update()
{
    List<Player*>::iterator iter = g_playerList.begin();
    List<Player*>::iterator iter_end = g_playerList.end();

    for (; iter != iter_end; ++iter)
    {
        (*iter)->Update();
    }
    
    iter = g_playerList.begin();

    for (; iter != iter_end; ++iter)
    {
        List<Player*>::iterator iter_second = iter;
        for (; iter_second != iter_end; ++iter_second)
        {
            if (comp(*iter, *iter_second))
            {
                Player* tmp = *iter_second;
                *iter_second = *iter;
                *iter = tmp;
            }
        }
    }

    List<Effect*>::iterator effect_iter = g_effectList.begin();
    List<Effect*>::iterator effect_iter_end = g_effectList.end();

    for (;effect_iter != effect_iter_end; ++effect_iter)
    {
        if ((*effect_iter)->IsRun())
        {
            (*effect_iter)->Update();
        }
    }
    Camera::GetInstance()->Update();

    NetSendProc();
}

void Render()
{
    FrameManager::GetInstance()->RenderCount();

    BYTE* buffer = ScreenDib::GetInstance()->GetBuffer();
    size_t width = ScreenDib::GetInstance()->GetWidth();
    size_t height = ScreenDib::GetInstance()->GetHeight();
    size_t pitch = ScreenDib::GetInstance()->GetPitch();
    /*
    SpriteDib::GetInstance()->DrawSprite(0, POINT{ 0, 0 }, buffer, width, height, pitch);
    */

    POINT cameraPos = Camera::GetInstance()->GetPosition();
    cameraPos.x &= 63;
    cameraPos.y &= 63;
    for (int iCntV = 0; iCntV < 10; iCntV++)
    {
        for (int iCntH = 0; iCntH < 11; iCntH++)
        {
            BYTE byTileIndex = m_byTileMap[iCntV][iCntH];
            SpriteDib::GetInstance()->DrawSprite_BK(byTileIndex, { -cameraPos.x + 64 * iCntH, -cameraPos.y + 64 * iCntV }, buffer, width, height, pitch);
        }
    }

    for (const auto& p : g_playerList)
    {
        p->Render(buffer, width, height, pitch);
    }

    for (const auto& e : g_effectList)
    {
        if(e->IsRun())
            e->Render(buffer, width, height, pitch);
    }

    ScreenDib::GetInstance()->Flip(g_hWnd);
}
