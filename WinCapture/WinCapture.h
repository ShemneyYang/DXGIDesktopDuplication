#pragma once

#define EXPORT __declspec(dllexport)

#ifdef __cplusplus
extern "C" {
#endif

	EXPORT int winCapture_init(void);
	EXPORT int winCapture_uninit(void);
	EXPORT int winCapture_getDesktopCapture(int& w, int& h);
	EXPORT unsigned char* winCapture_getRGBData(void);
	EXPORT int winCapture_getRGBDataLen(void);
	EXPORT int winCapture_getShareHandle(void);

#ifdef __cplusplus
}
#endif