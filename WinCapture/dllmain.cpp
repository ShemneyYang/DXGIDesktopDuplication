// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"
#include "CDesktopCaptrue.h"
#include "WinCapture.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

static CDesktopCaptrue* g_desktopCaptrue = NULL;

int winCapture_init(void)
{
	if (NULL == g_desktopCaptrue)
	{
		g_desktopCaptrue = new CDesktopCaptrue;
		g_desktopCaptrue->init();
	}
	else
	{
		return -1;
	}

	return 0;
}

int winCapture_uninit(void)
{
	if (NULL == g_desktopCaptrue)
	{
		return -1;
	}

	delete g_desktopCaptrue;
	g_desktopCaptrue = NULL;

	return 0;
}

int winCapture_getDesktopCapture(int& w, int& h)
{
	if (NULL == g_desktopCaptrue)
	{
		return -1;
	}
	g_desktopCaptrue->getDesktopCapture(w, h);
	return 0;
}

unsigned char* winCapture_getRGBData(void)
{
	if (NULL == g_desktopCaptrue)
	{
		return NULL;
	}

	return g_desktopCaptrue->getRGBData();
}

int winCapture_getRGBDataLen(void)
{
	if (NULL == g_desktopCaptrue)
	{
		return -1;
	}

	return g_desktopCaptrue->getRGBDataLen();
}

int winCapture_getShareHandle(void)
{
	return (int)g_desktopCaptrue->getShareHandle();
}
