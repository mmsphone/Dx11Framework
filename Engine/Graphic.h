#pragma once

#include "Base.h"

NS_(Engine)

/* 1. 기기 성능 조사 */
/* 2. 지원되는 범위 내 값 초기화를 위한 디바이스 오브젝트 생성 */

/* 1. ID3D11Device, ID3D11DeviceContext 생성*/
/* 2. IDXGISwapChain(BackBuffer ID3D11Texture2D) 생성*/
/* 3. BackBufferView 생성*/
/* 4. DepthBufferTexture, DepthBufferView 생성*/

class Graphic final : public Base
{
private:
	Graphic();
	virtual ~Graphic() = default;

public:
	// Device, Context 생성
	HRESULT Initialize(HWND windowHandle, WINMODE isWindowMode, _uint iWindowSizeX, _uint iWindowSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	//Buffer,View을 pClearColor로 리셋
	HRESULT ClearBackBufferView(const _float4* pClearColor);
	HRESULT ClearDepthStencilView();
	// front <-> back Buffer 교체
	HRESULT Present();

	static Graphic* Create(HWND windowHandle, WINMODE isWindowMode, _uint iWindowSizeX, _uint iWindowSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext);
	virtual void Free() override;

private:
	//내부함수
	HRESULT Ready_SwapChain(HWND WindowHandle, WINMODE isWindowMode, _uint iWinSizeX, _uint iWinSizeY);
	HRESULT Ready_BackBuffer();
	HRESULT Ready_DepthStencilView(_uint iWinSizeX, _uint iWinSizeY);

private:
	//데이터 메모리
	ID3D11Device* m_pDevice = nullptr;
	//기능 모음
	ID3D11DeviceContext* m_pContext = nullptr;
	//버퍼 스왑용
	IDXGISwapChain* m_pSwapChain = nullptr;
	//Back Buffer(Render Target)
	ID3D11RenderTargetView* m_pBackBuffer = nullptr;
	//Depth Stencil Buffer
	ID3D11DepthStencilView* m_pDepthStencilView = nullptr;
};

_NS