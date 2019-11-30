#include "stdafx.h"
#include "CDesktopCaptrue.h"
#include "PixelShader.h"
#include "VertexShader.h"
#include "libyuv/convert.h"
#include "TextureToFile.h"

CDesktopCaptrue::CDesktopCaptrue()
{
}


CDesktopCaptrue::~CDesktopCaptrue()
{
}

void CDesktopCaptrue::init(void)
{
	HRESULT hr = S_OK;

	// Driver types supported
	D3D_DRIVER_TYPE DriverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT NumDriverTypes = ARRAYSIZE(DriverTypes);

	// Feature levels supported
	D3D_FEATURE_LEVEL FeatureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_1
	};
	UINT NumFeatureLevels = ARRAYSIZE(FeatureLevels);

	D3D_FEATURE_LEVEL FeatureLevel;

	// Create device
	for (UINT DriverTypeIndex = 0; DriverTypeIndex < NumDriverTypes; ++DriverTypeIndex)
	{
		hr = D3D11CreateDevice(nullptr, DriverTypes[DriverTypeIndex], nullptr, 0, FeatureLevels, NumFeatureLevels,
			D3D11_SDK_VERSION, &m_device, &FeatureLevel, &m_context);
		if (SUCCEEDED(hr))
		{
			// Device creation success, no need to loop anymore
			break;
		}
	}
	if (FAILED(hr))
	{
		return;
	}

	// VERTEX shader
	UINT Size = ARRAYSIZE(g_VS);
	hr = m_device->CreateVertexShader(g_VS, Size, nullptr, &m_vertexShader);
	if (FAILED(hr))
	{
		return;
	}

	// Input layout
	D3D11_INPUT_ELEMENT_DESC Layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	UINT NumElements = ARRAYSIZE(Layout);
	hr = m_device->CreateInputLayout(Layout, NumElements, g_VS, Size, &m_inputLayout);
	if (FAILED(hr))
	{
		return;
	}
	m_context->IASetInputLayout(m_inputLayout.Get());

	// Pixel shader
	Size = ARRAYSIZE(g_PS);
	hr = m_device->CreatePixelShader(g_PS, Size, nullptr, &m_pixelShader);
	if (FAILED(hr))
	{
		return;
	}

	// Set up sampler
	D3D11_SAMPLER_DESC SampDesc;
	RtlZeroMemory(&SampDesc, sizeof(SampDesc));
	SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SampDesc.MinLOD = 0;
	SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_device->CreateSamplerState(&SampDesc, &m_samplerLinear);
	if (FAILED(hr))
	{
		return;
	}

	// Get DXGI device
	IDXGIDevice* DxgiDevice = nullptr;
	hr = m_device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
	if (FAILED(hr))
	{
		return;
	}

	// Get DXGI adapter
	IDXGIAdapter* DxgiAdapter = nullptr;
	hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
	DxgiDevice->Release();
	DxgiDevice = nullptr;
	if (FAILED(hr))
	{
		return;
	}

	// Get output
	IDXGIOutput* DxgiOutput = nullptr;
	hr = DxgiAdapter->EnumOutputs(0, &DxgiOutput);
	DxgiAdapter->Release();
	DxgiAdapter = nullptr;
	if (FAILED(hr))
	{
		return;
	}

	DxgiOutput->GetDesc(&m_outputDesc);

	// QI for Output 1
	IDXGIOutput1* DxgiOutput1 = nullptr;
	hr = DxgiOutput->QueryInterface(__uuidof(DxgiOutput1), reinterpret_cast<void**>(&DxgiOutput1));
	DxgiOutput->Release();
	DxgiOutput = nullptr;
	if (FAILED(hr))
	{
		return;
	}

	// Create desktop duplication
	hr = DxgiOutput1->DuplicateOutput(m_device.Get(), &m_deskDupl);
	DxgiOutput1->Release();
	DxgiOutput1 = nullptr;
	if (FAILED(hr))
	{
		if (hr == DXGI_ERROR_NOT_CURRENTLY_AVAILABLE)
		{
			//MessageBoxW(nullptr, L"There is already the maximum number of applications using the Desktop Duplication API running, please close one of those applications and then try again.", L"Error", MB_OK);
			return;// DUPL_RETURN_ERROR_UNEXPECTED;
		}
		return;
	}

	_initStagingTexture(Global::g_width, Global::g_height);
}

void CDesktopCaptrue::getDesktopCapture(int& w, int& h)
{
	w = 0;
	h = 0;
	DXGI_OUTDUPL_FRAME_INFO frameInfo = { 0 };
	ComPtr<IDXGIResource> desktopResource;
	HRESULT hr = m_deskDupl->AcquireNextFrame(1, &frameInfo, &desktopResource);
	if (hr == DXGI_ERROR_WAIT_TIMEOUT)
	{
		return;
	}

	if (DXGI_ERROR_INVALID_CALL == hr)
	{
		return;
	}

	do
	{
		hr = desktopResource->QueryInterface(__uuidof(ID3D11Texture2D), &m_desktopTexture);
		D3D11_TEXTURE2D_DESC desktopDesc = { 0 };
		m_desktopTexture->GetDesc(&desktopDesc);
		desktopResource = nullptr;
		if (FAILED(hr))
		{
			break;
		}
		w = desktopDesc.Width;
		h = desktopDesc.Height;

		if (NULL == m_stagingTexture)
		{
			_initStagingTexture(w, h);
		}
		else
		{
			D3D11_TEXTURE2D_DESC stagingDesc = { 0 };
			m_stagingTexture->GetDesc(&stagingDesc);
			if (stagingDesc.Width != desktopDesc.Width ||
				stagingDesc.Height != desktopDesc.Height)
			{
				_initStagingTexture(desktopDesc.Width, desktopDesc.Height);
			}
		}

		/*
// 		ID3D11Texture2D* CpuReadTexture = nullptr;
// 		D3D11CopyTexture(&CpuReadTexture, m_desktopTexture.Get(), m_device.Get(), m_context.Get());
//		if (CpuReadTexture)
		m_context->CopyResource(m_stagingTexture.Get(), m_desktopTexture.Get());
		{
//			SaveTextureToBmp(L"F:\\temp.bmp", m_stagingTexture.Get());
// 			CpuReadTexture->Release();
// 			CpuReadTexture = NULL;
		}
		*/

		
		static DWORD dwLastTime = ::timeGetTime();
		static int nLastCount = 0;
		++nLastCount;
		DWORD dwTime = ::timeGetTime() - dwLastTime;
		if (dwTime >= 1000)
		{
			Global::g_captureFps = nLastCount * 1000 / dwTime;
			dwLastTime = ::timeGetTime();
			nLastCount = 0;
		}

		m_context->CopyResource(m_stagingTexture.Get(), m_desktopTexture.Get());

		dwTime = ::timeGetTime();
		ComPtr< ID3D11DeviceContext> mD3dContext;
		m_device->GetImmediateContext(&mD3dContext);

		D3D11_MAPPED_SUBRESOURCE textureMemory;
		mD3dContext->Map(m_stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &textureMemory);
		// 			uint8* pData = new uint8[textureMemory.DepthPitch];
		// 			memcpy(pData, textureMemory.pData, textureMemory.DepthPitch);

		int nV = Global::g_lineSize[0];
		if (nV == 0)
		{
			Global::g_lineSize[0] = desktopDesc.Width;
			Global::g_lineSize[1] = (desktopDesc.Width + 1) / 2;
			Global::g_lineSize[2] = (desktopDesc.Width + 1) / 2;

			int nL = desktopDesc.Width * desktopDesc.Height;
			Global::g_yuv[0] = new unsigned char[nL];
			Global::g_yuv[1] = new unsigned char[nL / 4];
			Global::g_yuv[2] = new unsigned char[nL / 4];
		}

		if (Global::g_rgb == NULL)
		{
			Global::g_nRGBLen = textureMemory.DepthPitch;
			Global::g_rgb = new unsigned char[Global::g_nRGBLen];
		}

		memcpy(Global::g_rgb, textureMemory.pData, textureMemory.DepthPitch);
// 		int nRet = 0;
// 		uint8* dst_y = Global::g_yuv[0];
// 		uint8* dst_u = Global::g_yuv[1];
// 		uint8* dst_v = Global::g_yuv[2];
// 		nRet = libyuv::ABGRToI420((const uint8*)textureMemory.pData, textureMemory.RowPitch,
// 			dst_y, Global::g_lineSize[0],
// 			dst_u, Global::g_lineSize[1],
// 			dst_v, Global::g_lineSize[2],
// 			desktopDesc.Width, desktopDesc.Height);
		mD3dContext->Unmap(m_stagingTexture.Get(), 0);
		Global::g_captureTime = ::timeGetTime() - dwTime;
		if (false)
		{
// 			int nL = desktopDesc.Width * desktopDesc.Height;
// 			uint8* yuv = new uint8[nL * 3 / 2];
// 			uint8* dst = yuv;
// 			memcpy(dst, dst_y, nL);
// 			dst += nL;
// 			memcpy(dst, dst_u, nL / 4);
// 			dst += nL / 4;
// 			memcpy(yuv, dst_v, nL / 4);
// 			Global::WriteFileData(L"F:\\test.yuv", std::string((const char*)yuv, nL * 3 / 2));
		}

	} while (false);

	hr = m_deskDupl->ReleaseFrame();
}

void CDesktopCaptrue::_initStagingTexture(int w, int h)
{
	m_stagingTexture = nullptr;
	
	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.Width = w;
	desc.Height = h;
	desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.SampleDesc.Count = 1;
	desc.BindFlags = 0;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

	/*
	D3D11_TEXTURE2D_DESC DestBufferDesc;
	DestBufferDesc.Width = w;
	DestBufferDesc.Height = h;
	DestBufferDesc.MipLevels = 1;
	DestBufferDesc.ArraySize = 1;
	DestBufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	DestBufferDesc.SampleDesc.Count = 1;
	DestBufferDesc.SampleDesc.Quality = 0;
	DestBufferDesc.Usage = D3D11_USAGE_STAGING;
	DestBufferDesc.BindFlags = 0;
	DestBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	DestBufferDesc.MiscFlags = 0;
	*/

	HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, &m_stagingTexture);
	if (FAILED(hr))
	{
		return;
	}
}