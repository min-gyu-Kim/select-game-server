#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <windowsx.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <timeapi.h>
#include <time.h>
#include <CommCtrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "Types.h"
#include "resource.h"

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "Ws2_32.lib")

#define BGR(b, g, r) RGB(b, g, r)

#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:WINDOWS")