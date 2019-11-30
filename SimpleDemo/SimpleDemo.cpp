// SimpleDemo.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "SimpleDemo.h"
#include "CDx9Render.h"
#include "CDesktopCaptrue.h"
#include "SDL.h"

#define MAX_LOADSTRING 100

CDx9Render g_dx9Render;
CDesktopCaptrue g_desktopCapture;

// 全局变量:
HINSTANCE hInst;                                // 当前实例
HWND hWnd;
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名
HANDLE g_terminateThreadsEvent = NULL;
SDL_Window* g_screen = NULL;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
void startThread(void);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	timeBeginPeriod(1);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: 在此处放置代码。

	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_SIMPLEDEMO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	g_terminateThreadsEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	if (Global::RT_THREAD_SDL_RGB == Global::g_renderType)
	{
	}
	else 
	{
		// 执行应用程序初始化:
		if (!InitInstance(hInstance, nCmdShow))
		{
			return FALSE;
		}
		::SetTimer(hWnd, 6666, 100, NULL);
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIMPLEDEMO));
	MSG msg = { 0 };

	if (Global::g_renderType == Global::RT_NONE)
	{
		g_desktopCapture.init();
		// 主消息循环:
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

			int nW = 0;
			int nH = 0;
			DWORD dwTotal = ::timeGetTime();
			{
				g_desktopCapture.getDesktopCapture(nW, nH);
				if (nW != 0)
				{
					g_dx9Render.init(hWnd);
				}
			}

			{
				if (0 != nW)
				{
					static DWORD dwLastTime = ::timeGetTime();
					static int nLastCount = 0;
					++nLastCount;
					DWORD dwTime = ::timeGetTime() - dwLastTime;
					if (dwTime >= 1000)
					{
						Global::g_renderFps = nLastCount * 1000 / dwTime;
						dwLastTime = ::timeGetTime();
						nLastCount = 0;
					}

					dwTime = ::timeGetTime();
					//const unsigned char *yuv[3] = { Global::g_yuv[0], Global::g_yuv[1], Global::g_yuv[2] };
					//g_dx9Render.render(yuv, Global::g_lineSize, nW, nH);
					BITMAPINFOHEADER bitHeader = { 0 };
					bitHeader.biSize = sizeof(BITMAPINFOHEADER);
					bitHeader.biWidth = nW;
					bitHeader.biHeight = nH;
					bitHeader.biPlanes = 1;
					bitHeader.biBitCount = 32;
					bitHeader.biCompression = BI_RGB;
					bitHeader.biSizeImage = Global::g_nRGBLen;
					g_dx9Render.renderRGB(Global::g_rgb, bitHeader);
					Global::g_renderTime = ::timeGetTime() - dwTime;
					Global::g_totalTime = ::timeGetTime() - dwTotal;
				}
			}
		}
	}
	else if (Global::RT_THREAD == Global::g_renderType ||
		Global::RT_THREAD_SDL_YUV == Global::g_renderType ||
		Global::RT_THREAD_SDL_RGB == Global::g_renderType)
	{
		startThread();
		// 主消息循环:
		while (WM_QUIT != msg.message)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}

		}
	}

	SetEvent(g_terminateThreadsEvent);
	CloseHandle(g_terminateThreadsEvent);
	timeEndPeriod(1);
	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPLEDEMO));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_SIMPLEDEMO);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	RECT WindowRect = { 0, 0, 1920, 1080 };
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);
	hWnd = CreateWindowW(L"ddasample", L"DXGI desktop duplication sample",
		WS_OVERLAPPEDWINDOW,
		0, 0,
		WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top,
		nullptr, nullptr, hInstance, nullptr);
	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择:
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
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: 在此处添加使用 hdc 的任何绘图代码...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
	{
		char cBuf[256] = { 0 };
		sprintf_s(cBuf, 256, "CaptureFps=%d, PaintFps=%d, CaptureTime=%d, RenderTime=%d, TotalTime=%d\n", Global::g_captureFps, Global::g_renderFps, Global::g_captureTime, Global::g_renderTime, Global::g_totalTime);
		//_itoa_s(Global::g_CaptureFps, cBuf, 256, 10);
		OutputDebugStringA(cBuf);
		::SetWindowTextA(hWnd, cBuf);
	}
	break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
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

DWORD WINAPI threadProc(_In_ void* Param)
{
	SDL_Renderer* sdlRenderer = NULL;
	SDL_Texture* sdlTexture = NULL;
	g_desktopCapture.init();
	bool bInit = false;
	while ((WaitForSingleObjectEx(g_terminateThreadsEvent, 0, FALSE) == WAIT_TIMEOUT))
	{
		int nW = 0;
		int nH = 0;
		DWORD dwTotal = ::timeGetTime();
		{
			g_desktopCapture.getDesktopCapture(nW, nH);
			if (nW != 0 && !bInit)
			{
				bInit = true;
				if (Global::RT_THREAD == Global::g_renderType)
				{
					g_dx9Render.init(hWnd);
				}
				else if(Global::RT_THREAD_SDL_RGB == Global::g_renderType)
				{
					if (SDL_Init(SDL_INIT_VIDEO)) {
						//printf("Could not initialize SDL - %s\n", SDL_GetError());
						return -1;
					}
					//SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d11");
					//SDL 2.0 Support for multiple windows
					g_screen = SDL_CreateWindow("Simplest Video Play SDL2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
						1280, 720, SDL_WINDOW_SHOWN);
					if (!g_screen) {
						//printf("SDL: could not create window - exiting:%s\n", SDL_GetError());
						return -1;
					}

					sdlRenderer = SDL_CreateRenderer(g_screen, -1, 0);
					Uint32 pixformat = SDL_PIXELFORMAT_ARGB8888;
					sdlTexture = SDL_CreateTexture(sdlRenderer, pixformat, SDL_TEXTUREACCESS_STREAMING, nW, nH);
				}
			}
		}

		{
			if (0 != nW)
			{
				static DWORD dwLastTime = ::timeGetTime();
				static int nLastCount = 0;
				++nLastCount;
				DWORD dwTime = ::timeGetTime() - dwLastTime;
				if (dwTime >= 1000)
				{
					Global::g_renderFps = nLastCount * 1000 / dwTime;
					dwLastTime = ::timeGetTime();
					nLastCount = 0;
				}

				dwTime = ::timeGetTime();

				if (Global::RT_THREAD == Global::g_renderType)
				{
					//const unsigned char *yuv[3] = { Global::g_yuv[0], Global::g_yuv[1], Global::g_yuv[2] };
					//g_dx9Render.render(yuv, Global::g_lineSize, nW, nH);
					BITMAPINFOHEADER bitHeader = { 0 };
					bitHeader.biSize = sizeof(BITMAPINFOHEADER);
					bitHeader.biWidth = nW;
					bitHeader.biHeight = nH;
					bitHeader.biPlanes = 1;
					bitHeader.biBitCount = 32;
					bitHeader.biCompression = BI_RGB;
					bitHeader.biSizeImage = Global::g_nRGBLen;
					g_dx9Render.renderRGB(Global::g_rgb, bitHeader);
				}
				else if (Global::RT_THREAD_SDL_RGB == Global::g_renderType)
				{
					SDL_Rect sdlRect;
					sdlRect.x = 0;
					sdlRect.y = 0;
					sdlRect.w = nW;
					sdlRect.h = nH;
					SDL_UpdateTexture(sdlTexture, NULL, Global::g_rgb, nW * 4);
					SDL_RenderClear(sdlRenderer);
					SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, &sdlRect);
					SDL_RenderPresent(sdlRenderer);
				}

				Global::g_renderTime = ::timeGetTime() - dwTime;
				Global::g_totalTime = ::timeGetTime() - dwTotal;
			}
		}
	}
	return 0;
}

void startThread(void)
{
	DWORD ThreadId;
	HANDLE hThread = CreateThread(nullptr, 0, threadProc, nullptr, 0, &ThreadId);
}