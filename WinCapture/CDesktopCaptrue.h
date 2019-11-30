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

private:
	void _initStagingTexture(int w, int h);

private:
	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<ID3D11VertexShader> m_vertexShader;
	ComPtr<ID3D11PixelShader> m_pixelShader;
	ComPtr<ID3D11InputLayout> m_inputLayout;
	ComPtr<ID3D11SamplerState> m_samplerLinear;
	ComPtr<ID3D11Texture2D> m_desktopTexture;
	ComPtr<ID3D11Texture2D> m_stagingTexture;

	DXGI_OUTPUT_DESC m_outputDesc;
	ComPtr<IDXGIOutputDuplication> m_deskDupl;

	unsigned char* m_yuv[3];
	int m_lineSize[3];
	unsigned char* m_rgb;
	int m_nRGBLen;
};

