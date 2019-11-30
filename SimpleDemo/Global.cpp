#include "stdafx.h"
#include "Global.h"

int Global::g_width = 0;
int Global::g_height = 0;
unsigned char* Global::g_yuv[3] = { 0,0,0 };
int Global::g_lineSize[3] = { 0,0,0 };
unsigned char* Global::g_rgb = NULL;
int Global::g_nRGBLen = 0;

int Global::g_captureFps = 0;
int Global::g_renderFps = 0;
int Global::g_captureTime = 0;
int Global::g_renderTime = 0;
int Global::g_totalTime = 0;

Global::ERenderType Global::g_renderType = Global::RT_THREAD_SDL_RGB;

bool Global::GetFileSize(const std::wstring& strFilePath, ULONGLONG& nFileSize)
{
	HANDLE hFileHandle = CreateFile(strFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (INVALID_HANDLE_VALUE == hFileHandle)
	{
		return false;
	}

	LARGE_INTEGER filesize;

	if (GetFileSizeEx(hFileHandle, &filesize))
	{
		nFileSize = filesize.QuadPart;
		::CloseHandle(hFileHandle);
		return true;
	}
	::CloseHandle(hFileHandle);
	return false;
}

bool Global::ReadFileData(const std::wstring &fileName, std::string &data)
{
	ULONGLONG fileSize = 0;
	if (GetFileSize(fileName, fileSize) == false)
	{
		return false;
	}

	//the file size must be less than 16 MB
	//DW_ASSERT_X(fileSize <= 16 * 1024 * 1024, __FUNCTION__, "Can't read more 16MB file data in one time.");

	FILE *fp = NULL;
	_wfopen_s(&fp, fileName.c_str(), L"rb");
	if (fp)
	{
		data.resize((size_t)fileSize);
		size_t count = fread((void*)data.data(), 1, data.size(), fp);
		fclose(fp);

		if (count == data.size())
		{
			return true;
		}
	}

	data.clear();
	return false;
}

bool Global::WriteFileData(const std::wstring &fileName, const std::string &data)
{
	//DW_ASSERT_X(data.size() <= 16 * 1024 * 1024, __FUNCTION__, "Can't write more 16MB file data in one time.");

	FILE *fp = NULL;
	_wfopen_s(&fp, fileName.c_str(), L"wb+");
	if (fp)
	{
		size_t count = fwrite(data.data(), 1, data.size(), fp);
		fclose(fp);

		if (count == data.size())
		{
			return true;
		}
	}

	return false;
}