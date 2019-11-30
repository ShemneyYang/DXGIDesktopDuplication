#pragma once

class Global
{
public:
	enum ERenderType
	{
		RT_NONE = 0,
		RT_THREAD,
		RT_THREAD_SDL_YUV,
		RT_THREAD_SDL_RGB
	};
public:
	static int g_width;
	static int g_height;
	static unsigned char* g_yuv[3];
	static int g_lineSize[3];
	static unsigned char* g_rgb;
	static int g_nRGBLen;

	static int g_captureFps;
	static int g_renderFps;
	static int g_captureTime;
	static int g_renderTime;
	static int g_totalTime;
	static ERenderType g_renderType;

public:
	static bool GetFileSize(const std::wstring& strFilePath, ULONGLONG& nFileSize);
	static bool ReadFileData(const std::wstring &fileName, std::string &data);
	static bool	WriteFileData(const std::wstring &fileName, const std::string &data);
};

