#pragma once

#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

using namespace Microsoft::WRL;

class CDesktopCaptrue
{
public:
	CDesktopCaptrue();
	~CDesktopCaptrue();

	void init(void);

	void getDesktopCapture(int& w, int& h);

	unsigned char* getRGBData(void);
	int getRGBDataLen(void);

	HANDLE getShareHandle() const;

private:
	void _initStagingTexture(int w, int h, DXGI_FORMAT Format);
	void _dealWithStageCopy(const D3D11_TEXTURE2D_DESC &desktopDesc, ComPtr<ID3D11Texture2D> srcTexture);
	void _initShareHandleTexture(int w, int h, DXGI_FORMAT Format);
	void _dealWithShareHandle(const D3D11_TEXTURE2D_DESC &desktopDesc, ComPtr<ID3D11Texture2D> srcTexture);
	void _testShareHandle(const D3D11_TEXTURE2D_DESC &desktopDesc);

private:
	enum OutputType
	{
		OT_ShareHandle = 1,
		OT_MemCopy = 2,
	};
	OutputType	m_outputType;
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;
	ComPtr<ID3D11SamplerState> m_samplerLinear;
	ComPtr<ID3D11Texture2D> m_desktopTexture;
	ComPtr<ID3D11Texture2D> m_stagingTexture;
	ComPtr<ID3D11Texture2D> m_shareHandletexture;
	HANDLE		m_sharehandle;

	DXGI_OUTPUT_DESC m_outputDesc;
	ComPtr<IDXGIOutputDuplication> m_deskDupl;

	unsigned char* m_yuv[3];
	int m_lineSize[3];
	unsigned char* m_rgb;
	int m_nRGBLen;
};

