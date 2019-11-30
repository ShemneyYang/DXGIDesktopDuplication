#pragma once

#include <d3d9.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

class CDx9Render
{
public:
	CDx9Render();
	~CDx9Render();

	bool init(HWND hWnd);

	bool render(const unsigned char *yuv[3], int linesize[3], int width, int height);

	bool renderRGB(void *buf, BITMAPINFOHEADER &header);

private:
	bool _initTexture(int w, int h);
	bool _updateYuvToTexture(ComPtr<IDirect3DTexture9> texture, ComPtr<IDirect3DTexture9> stagingTexture,
		const unsigned char* yuv, int lineSize, int width, int height);
	void _resetDestSurfaceRect(const RECT &srcRect, RECT &destRect);
	
	long _bytes_per_line(long bmWidth, long bmBitCount);
	bool _copyVideoBufferToVideoSurface(void *buf, BITMAPINFOHEADER &header);
	void _setupOffSurface(int width, int height, D3DFORMAT format);
	bool _copyVideoSurfaceToRenderTarget();
	void _getSurfaceRect(IDirect3DSurface9* surface, RECT &rc);
	void _setupRenderTargetSurface(int width, int height);

private:
	int m_nDstPaintWidth;
	int m_nDstPaintHeight;
	int m_nTextureWidth;
	int m_nTextureHeight;
	HMODULE m_hD3d9Module;
	ComPtr<IDirect3D9> m_d3d9;
	ComPtr <IDirect3DDevice9> m_d3dDevice;
	ComPtr<IDirect3DVertexBuffer9> m_graphicsItemVertexBuffer;
	
	
	ComPtr<IDirect3DTexture9> m_yTexture;
	ComPtr<IDirect3DTexture9> m_uTexture;
	ComPtr<IDirect3DTexture9> m_vTexture;

	ComPtr<IDirect3DTexture9> m_yStagingTexture;
	ComPtr<IDirect3DTexture9> m_uStagingTexture;
	ComPtr<IDirect3DTexture9> m_vStagingTexture;

	ComPtr<IDirect3DPixelShader9> m_shader;

	D3DDISPLAYMODE m_displayMode;
	D3DPRESENT_PARAMETERS	m_d3dPresendParam;

	ComPtr<IDirect3DSurface9> m_videoSurface;
	ComPtr<IDirect3DSurface9> m_canvsSurface;
};

