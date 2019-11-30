#include "CDx9Render.h"

//Flexible Vertex Format, FVF
struct CUSTOMVERTEX
{
	FLOAT       x, y, z;      // vertex untransformed position
	FLOAT       rhw;        // eye distance
	D3DCOLOR    diffuse;    // diffuse color
	FLOAT       tu, tv;     // texture relative coordinates
};

#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)

CDx9Render::CDx9Render():
	m_hD3d9Module(NULL)
{
}


CDx9Render::~CDx9Render()
{
}

bool CDx9Render::init(HWND hWnd, int nWidth, int nHeight)
{
	if (NULL == m_hD3d9Module)
	{
		m_hD3d9Module = LoadLibrary(L"d3d9.dll");
		if (m_hD3d9Module == NULL)
		{
			return false;
		}
	}

	typedef IDirect3D9* (WINAPI *LPDIRECT3DCREATE9)(UINT);
	LPDIRECT3DCREATE9 Direct3DCreate9Ptr = NULL;
	Direct3DCreate9Ptr = (LPDIRECT3DCREATE9)GetProcAddress(m_hD3d9Module, "Direct3DCreate9");
	if (Direct3DCreate9Ptr != NULL)
	{
		// Create the D3D object, which is needed to create the D3DDevice.
		m_d3d9 = Direct3DCreate9Ptr(D3D_SDK_VERSION);
	}
	//m_d3D9 = Direct3DCreate9( D3D_SDK_VERSION );
	if (m_d3d9 == NULL)
	{
		//LogFinal(LOG::KVideoDXRender) << "创建D3D对象失败，error=" << ::GetLastError();
		return false;
	}

	//display mode of the adapter
	HRESULT hr = m_d3d9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &m_displayMode);
	//_d3dApiErrLog(hr, "GetAdapterDisplayMode");
	if (hr != D3D_OK)
	{
		return false;
	}
	//LogFinal(LOG::KVideoDXRender) << "display format=" << m_displayMode.Format;

	D3DCAPS9 deviceCaps;
	hr = m_d3d9->GetDeviceCaps(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, &deviceCaps);
	//_d3dApiErrLog(hr, "GetDeviceCaps");
	if (hr != D3D_OK)
	{
		return false;
	}

	// 是否可以使用硬件顶点处理?
	if (!(deviceCaps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT))
	{
		return false;
	}

	ZeroMemory(&m_d3dPresendParam, sizeof(m_d3dPresendParam));
	m_d3dPresendParam.BackBufferWidth = nWidth;
	m_d3dPresendParam.BackBufferHeight = nHeight;
	m_d3dPresendParam.Windowed = TRUE;
	m_d3dPresendParam.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dPresendParam.BackBufferFormat = D3DFMT_UNKNOWN;
	m_d3dPresendParam.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;

	//检测是否有硬件处理顶点
	hr = m_d3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_NOWINDOWCHANGES, &m_d3dPresendParam, &m_d3dDevice);
	if (hr == D3D_OK && m_d3dPresendParam.BackBufferWidth != 0 && m_d3dPresendParam.BackBufferHeight != 0 && m_d3dPresendParam.BackBufferFormat != D3DFMT_UNKNOWN)
	{
// 		LogFinal(LOG::KVideoDXRender) << "CreateDevice ok" << " d3dvice=" << (long)m_d3DDevice << " backwidth=" << m_d3dPresendParam.BackBufferWidth << " backheight=" << m_d3dPresendParam.BackBufferHeight
// 			<< " backformat" << m_d3dPresendParam.BackBufferFormat << " devicewnd=" << m_d3dPresendParam.hDeviceWindow << " flags=" << m_d3dPresendParam.Flags
// 			<< " winwidth=" << m_windowGeometry.width() << " winheight=" << m_windowGeometry.height();

		m_d3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);
		// Turn off D3D lighting
		m_d3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		// Turn on the zbuffer
		m_d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	}
	else
	{
		return false;
	}

	hr = m_d3dDevice->TestCooperativeLevel();
	if (hr != D3D_OK)
	{
		//_d3dApiErrLog(hr, "TestCooperativeLevel");
		return false;
	}

	ComPtr<IDirect3DSurface9> videoSurface = nullptr;
	hr = m_d3dDevice->CreateOffscreenPlainSurface(50, 50, D3DFMT_X8R8G8B8, D3DPOOL_SYSTEMMEM, &videoSurface, NULL);
	if (hr != D3D_OK)
	{
		return false;
	}
	else
	{
		videoSurface = nullptr;
	}

	hr = E_FAIL;
	if (m_yTexture == nullptr)
	{
		hr = m_d3dDevice->CreateTexture(nWidth, nHeight, 1, 0, D3DFMT_L8, D3DPOOL_DEFAULT, &m_yTexture, NULL);
		//_d3dApiErrLog(hr, "CreateTexture Y D3DPOOL_DEFAULT");
	}

	if (m_uTexture == NULL)
	{
		hr = m_d3dDevice->CreateTexture(nWidth / 2, nHeight / 2, 1, 0, D3DFMT_L8, D3DPOOL_DEFAULT, &m_uTexture, NULL);
		//_d3dApiErrLog(hr, "CreateTexture U D3DPOOL_DEFAULT");
	}

	if (m_vTexture == NULL)
	{
		hr = m_d3dDevice->CreateTexture(nWidth / 2, nHeight / 2, 1, 0, D3DFMT_L8, D3DPOOL_DEFAULT, &m_vTexture, NULL);
		//_d3dApiErrLog(hr, "CreateTexture U D3DPOOL_DEFAULT");
	}

	if (m_yStagingTexture == NULL)
	{
		hr = m_d3dDevice->CreateTexture(nWidth, nHeight, 1, 0, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &m_yStagingTexture, NULL);
		//_d3dApiErrLog(hr, "CreateTexture Y D3DPOOL_SYSTEMMEM");
	}

	if (m_uStagingTexture == NULL)
	{
		hr = m_d3dDevice->CreateTexture(nWidth / 2, nHeight / 2, 1, 0, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &m_uStagingTexture, NULL);
		//_d3dApiErrLog(hr, "CreateTexture U D3DPOOL_SYSTEMMEM");
	}

	if (m_vStagingTexture == NULL)
	{
		hr = m_d3dDevice->CreateTexture(nWidth / 2, nHeight / 2, 1, 0, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &m_vStagingTexture, NULL);
		//_d3dApiErrLog(hr, "CreateTexture U D3DPOOL_SYSTEMMEM");
	}

	if (m_shader == NULL)
	{
		const DWORD shader_data[] = {
			0xffff0200, 0x05000051, 0xa00f0000, 0xbd808081, 0xbf008081, 0xbf008081,
			0x3f800000, 0x05000051, 0xa00f0001, 0x3f94fdf4, 0x00000000, 0x3fcc49ba,
			0x00000000, 0x05000051, 0xa00f0002, 0x3f94fdf4, 0xbec83127, 0xbf5020c5,
			0x00000000, 0x05000051, 0xa00f0003, 0x3f94fdf4, 0x400126e9, 0x00000000,
			0x00000000, 0x0200001f, 0x80000000, 0xb0030000, 0x0200001f, 0x80000000,
			0x900f0000, 0x0200001f, 0x90000000, 0xa00f0800, 0x0200001f, 0x90000000,
			0xa00f0801, 0x0200001f, 0x90000000, 0xa00f0802, 0x03000042, 0x800f0000,
			0xb0e40000, 0xa0e40800, 0x03000042, 0x800f0001, 0xb0e40000, 0xa0e40801,
			0x03000042, 0x800f0002, 0xb0e40000, 0xa0e40802, 0x02000001, 0x80020000,
			0x80000001, 0x02000001, 0x80040000, 0x80000002, 0x03000002, 0x80070000,
			0x80e40000, 0xa0e40000, 0x03000008, 0x80010001, 0x80e40000, 0xa0e40001,
			0x03000008, 0x80020001, 0x80e40000, 0xa0e40002, 0x0400005a, 0x80040001,
			0x80e40000, 0xa0e40003, 0xa0aa0003, 0x02000001, 0x80080001, 0xa0ff0000,
			0x03000005, 0x800f0000, 0x80e40001, 0x90e40000, 0x02000001, 0x800f0800,
			0x80e40000, 0x0000ffff
		};

		HRESULT hr = m_d3dDevice->CreatePixelShader(shader_data, &m_shader);
		//_d3dApiErrLog(hr, "CreatePixelShader");
	}
	return true;
}

bool CDx9Render::render(const unsigned char *yuv[3], int linesize[3], int width, int height)
{
	if (m_yTexture != NULL && m_uTexture != NULL && m_vTexture != NULL && m_yStagingTexture != NULL && m_uStagingTexture != NULL && m_vStagingTexture != NULL)
	{
		const unsigned char *lpSrcY = yuv[0];
		const unsigned char *lpSrcU = yuv[1];
		const unsigned char *lpSrcV = yuv[2];
		_updateYuvToTexture(m_yTexture, m_yStagingTexture, lpSrcY, linesize[0], width, height);
		_updateYuvToTexture(m_uTexture, m_uStagingTexture, lpSrcU, linesize[1], width >> 1, height >> 1);
		_updateYuvToTexture(m_vTexture, m_vStagingTexture, lpSrcV, linesize[2], width >> 1, height >> 1);
	}

	if (m_graphicsItemVertexBuffer == NULL)
	{
		HRESULT hr = m_d3dDevice->CreateVertexBuffer(4 * sizeof(CUSTOMVERTEX), 0, D3DFVF_CUSTOMVERTEX, D3DPOOL_DEFAULT, &m_graphicsItemVertexBuffer, NULL);
		//_d3dApiErrLog(hr, "CreateOffscreenPlainSurface");
	}

	HRESULT hr = m_d3dDevice->BeginScene();
	if (hr != D3D_OK)
	{
		//_d3dApiErrLog(hr, "BeginScene");
		return false;
	}

	m_d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	IDirect3DStateBlock9* pStateBlock = NULL;
	hr = m_d3dDevice->CreateStateBlock(D3DSBT_ALL, &pStateBlock);
	if (hr == S_OK)
	{
		pStateBlock->Capture();
	}
	D3DSURFACE_DESC desc;
	hr = m_yStagingTexture->GetLevelDesc(0, &desc);
	if (hr != D3D_OK)
	{
		return false;
	}

// 	RECT srcRect = { 0 };
// 	RECT destRect = { 0 };
// 
// 	QRectF uvRect;
// 	uvRect.setLeft(0.0f);
// 	uvRect.setTop(0.0f);
// 	uvRect.setRight(1.0f);
// 	uvRect.setBottom(1.0f);
// 
// 	srcRect.right = desc.Width;
// 	srcRect.bottom = desc.Height;
// 
// 	destRect.left = q->pos().x();
// 	destRect.top = q->pos().y();
// 	destRect.right = q->fixSize().width();
// 	destRect.bottom = q->fixSize().height();
// 
// 	if (m_videoOutputRatioFlags & VOPF_Move)
// 	{
// 		_resetDestSurfaceRect(srcRect, destRect);
// 	}
// 
// 	if (m_videoOutputRatioFlags & VOPF_Clip)
// 	{
// 		_resetSrcSurfaceRect(destRect, srcRect, uvRect);
// 	}

	//desc to vector desc
// 	float x = destRect.left;
// 	float y = destRect.top;
// 	QSizeF size = QSizeF(destRect.right - destRect.left, destRect.bottom - destRect.top);

	CUSTOMVERTEX vertices[] =
	{
		{0.5f, - 0.5f, 0.0f, 1.0f,D3DCOLOR_ARGB(255, 255, 255, 255),	0.0f, 0.0f},
		{(float)width - 0.5f, - 0.5f,	0.0f,	1.0f,D3DCOLOR_ARGB(255, 255, 255, 255),	1.0f,	0.0f},
		{(float)width - 0.5f,	(float)height - 0.5f,	0.0f,	1.0f,D3DCOLOR_ARGB(255, 255, 255, 255),	1.0f,	1.0f},
		{- 0.5f,				(float)height - 0.5f,	0.0f,	1.0f,D3DCOLOR_ARGB(255, 255, 255, 255),	0.0f,	1.0f}
	};

	CUSTOMVERTEX *pVertex;
	hr = m_graphicsItemVertexBuffer->Lock(0, 4 * sizeof(CUSTOMVERTEX), (void**)&pVertex, 0);
	if (hr == S_OK)
	{
		memcpy(pVertex, vertices, sizeof(vertices));
		m_graphicsItemVertexBuffer->Unlock();
	}

	m_d3dDevice->SetTexture(0, m_yTexture.Get());
	m_d3dDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_d3dDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_d3dDevice->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	m_d3dDevice->SetTexture(1, m_uTexture.Get());
	m_d3dDevice->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_d3dDevice->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_d3dDevice->SetSamplerState(1, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	m_d3dDevice->SetTexture(2, m_vTexture.Get());
	m_d3dDevice->SetSamplerState(2, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_d3dDevice->SetSamplerState(2, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_d3dDevice->SetSamplerState(2, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);

	hr = m_d3dDevice->SetPixelShader(m_shader.Get());
	//_d3dApiErrLog(hr, "SetPixelShader");

	hr = m_d3dDevice->SetStreamSource(0, m_graphicsItemVertexBuffer.Get(), 0, sizeof(CUSTOMVERTEX));
	//_d3dApiErrLog(hr, "SetTexture from YYDxYuvGraphicsItem");

	hr = m_d3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
	//_d3dApiErrLog(hr, "SetTexture from YYDxYuvGraphicsItem");

	hr = m_d3dDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	//_d3dApiErrLog(hr, "SetTexture from YYDxYuvGraphicsItem");

	hr = m_d3dDevice->SetPixelShader(NULL);
	//_d3dApiErrLog(hr, "SetPixelShader NULL");

	if (hr == S_OK)
	{
		hr = pStateBlock->Apply();
		hr = pStateBlock->Release();
	}

	m_d3dDevice->EndScene();

	hr = m_d3dDevice->Present(NULL, NULL, NULL, NULL);
	if (hr == D3DERR_DEVICELOST)
	{
		//设备丢失，重新创建设备
		//_forceResetDevice();
	}
	else if (hr == S_PRESENT_OCCLUDED)
	{
		//Occluded applications can continue rendering and all calls will succeed, 
		//but the occluded presentation window will not be updated. 
		//Preferably the application should stop rendering to the presentation window using the device 
		//and keep calling CheckDeviceState until S_OK or S_PRESENT_MODE_CHANGED returns.
		//_d3dApiErrLog(hr, "Present");
	}
	else if (hr != D3D_OK)
	{
		//_d3dApiErrLog(hr, "Present");
		return false;
	}

	return true;
}

bool CDx9Render::renderRGB(void *buf, BITMAPINFOHEADER &header)
{
	return true;
}

bool CDx9Render::_updateYuvToTexture(ComPtr<IDirect3DTexture9> texture, ComPtr<IDirect3DTexture9> stagingTexture,
	const unsigned char* yuv, int lineSize, int width, int height)
{
	if (texture == NULL || stagingTexture == NULL)
	{
		return false;
	}

	D3DLOCKED_RECT d3d_rect;
	HRESULT ret = stagingTexture->LockRect(0, &d3d_rect, NULL, 0);
	if (ret == D3D_OK)
	{
		unsigned char *lpSurface = (unsigned char*)d3d_rect.pBits;
		DWORD dwCopyLen = min((DWORD)lineSize, width);
		for (DWORD i = 0; i < height; i++)
		{
			memcpy(lpSurface, yuv, dwCopyLen);
			yuv += lineSize;
			lpSurface += d3d_rect.Pitch;
		}

		stagingTexture->UnlockRect(0);

		ret = m_d3dDevice->UpdateTexture(stagingTexture.Get(), texture.Get());
		//_d3dApiErrLog(ret, "UpdateTexture");
	}

	return (ret == D3D_OK);
}