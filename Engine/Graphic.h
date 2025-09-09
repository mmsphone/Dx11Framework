#pragma once

#include "Base.h"

NS_(Engine)

/* 1. check device status */
/* 2. make device object for data initialize in range supported*/

/* 1. ID3D11Device, ID3D11DeviceContext*/
/* 2. IDXGISwapChain(BackBuffer ID3D11Texture2D) */
/* 3. BackBufferView */
/* 4. DepthBufferTexture, DepthBufferView*/

class Graphic final : public Base
{
private:
	Graphic();
	virtual ~Graphic() = default;

public:
	//Create Graphic Device, Context
	HRESULT Initialize(HWND windowHandle, WINMODE isWindowMode, _uint iWindowSizeX, _uint iWindowSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	//Clear Buffer,View
	HRESULT ClearBackBufferView(const _float4* pClearColor);
	HRESULT ClearDepthStencilView();
	// change buffer front <-> back
	HRESULT Present();
	//Object
	static Graphic* Create(HWND windowHandle, WINMODE isWindowMode, _uint iWindowSizeX, _uint iWindowSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	virtual void Free() override;
private:
	HRESULT Ready_SwapChain(HWND WindowHandle, WINMODE isWindowMode, _uint iWinSizeX, _uint iWinSizeY);
	HRESULT Ready_BackBuffer();
	HRESULT Ready_DepthStencilView(_uint iWinSizeX, _uint iWinSizeY);

private:
	//memory
	ID3D11Device* m_pDevice = nullptr;
	//function
	ID3D11DeviceContext* m_pContext = nullptr;
	//Swap Buffer
	IDXGISwapChain* m_pSwapChain = nullptr;
	//Back Buffer(Render Target)
	ID3D11RenderTargetView* m_pBackBuffer = nullptr;
	//Depth Stencil Buffer
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
};

_NS