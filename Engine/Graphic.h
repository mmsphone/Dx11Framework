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
	HRESULT Initialize(HWND hWnd, )
};

_NS