#include "Graphic.h"

Graphic::Graphic()
{
}

HRESULT Graphic::Initialize(HWND windowHandle, WINMODE isWindowMode, _uint iWindowSizeX, _uint iWindowSizeY)
{
    _uint isDebug = 0;
#ifdef _DEBUG
    isDebug = D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL FeatureLevel;

    //Device Context생성
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, isDebug, nullptr, 0, D3D11_SDK_VERSION, &m_pDevice, &FeatureLevel, &m_pContext)))
        return E_FAIL;
    //SwapChain 생성
    if (FAILED(ReadySwapChain(windowHandle, isWindowMode, iWindowSizeX, iWindowSizeY)))
        return E_FAIL;
    //BackBuffer 생성
    if (FAILED(ReadyBackBuffer()))
        return E_FAIL;
    //DepthStencilBuffer 생성
    if (FAILED(ReadyDepthStencilView(iWindowSizeX, iWindowSizeY)))
        return E_FAIL;

    // RenderTargets, DepthStencil 설정 (최대 8 랜더타겟)
    ID3D11RenderTargetView* pRenderTargets[] = {
        m_pBackBuffer,
    };
    _uint RenderTargetsCount = 1;
    m_pContext->OMSetRenderTargets(RenderTargetsCount, pRenderTargets, m_pDepthStencilView);

    D3D11_VIEWPORT ViewPortDescriptor;
    //뷰포트 좌표
    ViewPortDescriptor.TopLeftX = 0;
    ViewPortDescriptor.TopLeftY = 0;
    //뷰포트 크기
    ViewPortDescriptor.Width = static_cast<_float>(iWindowSizeX);
    ViewPortDescriptor.Height = static_cast<_float>(iWindowSizeY);
    //Z버퍼 범위
    ViewPortDescriptor.MinDepth = 0.f;
    ViewPortDescriptor.MaxDepth = 1.f;
    _uint ViewPortCount = 1;

    m_pContext->RSSetViewports(ViewPortCount, &ViewPortDescriptor);

    return S_OK;
}

HRESULT Graphic::ClearBackBufferView(const _float4* pClearColor)
{
    CHECKNULLPTR(m_pContext)
        return E_FAIL;

    //백버퍼를 특정 색으로 초기화
    m_pContext->ClearRenderTargetView(m_pBackBuffer, reinterpret_cast<const _float*>(pClearColor));
    return S_OK;
}

HRESULT Graphic::ClearDepthStencilView()
{
    CHECKNULLPTR(m_pContext)
        return E_FAIL;

    m_pContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
    return S_OK;
}

HRESULT Graphic::Present()
{
    CHECKNULLPTR(m_pSwapChain)
        return E_FAIL;

    return E_NOTIMPL;
}

ID3D11Device* Graphic::GetDevice() const
{
    return m_pDevice;
}

ID3D11DeviceContext* Graphic::GetContext() const
{
    return m_pContext;
}

Graphic* Graphic::Create(HWND windowHandle, WINMODE isWindowMode, _uint iWindowSizeX, _uint iWindowSizeY)
{
    Graphic* pInstance = new Graphic();
    
    if (FAILED(pInstance->Initialize(windowHandle, isWindowMode, iWindowSizeX, iWindowSizeY)))
    {
        MSG_BOX("Failed to Create : Graphic");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Graphic::Free()
{
    __super::Free();
    SafeRelease(m_pSwapChain);
    SafeRelease(m_pDepthStencilView);
    SafeRelease(m_pBackBuffer);
    SafeRelease(m_pContext);

#ifdef _DEBUG
    ID3D11Debug* d3dDebug;
    HRESULT hr = m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
    if (SUCCEEDED(hr))
    {
        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
        OutputDebugStringW(L"                                                                    D3D11 Live Object ref Count Checker \r ");
        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");

        hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);

        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
        OutputDebugStringW(L"                                                                    D3D11 Live Object ref Count Checker END \r ");
        OutputDebugStringW(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- \r ");
    }
    if (d3dDebug != nullptr)            d3dDebug->Release();
#endif

    SafeRelease(m_pDevice);
}

HRESULT Graphic::ReadySwapChain(HWND WindowHandle, WINMODE isWindowMode, _uint iWindowSizeX, _uint iWindowSizeY)
{
    //임시 변수
    IDXGIDevice* pDevice = nullptr;
    m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDevice);

    IDXGIAdapter* pAdapter = nullptr;
    pDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pAdapter);

    IDXGIFactory* pFactory = nullptr;
    pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pFactory);

    //스왑체인 세팅
    DXGI_SWAP_CHAIN_DESC		SwapChainDescriptor;
    ZeroMemory(&SwapChainDescriptor, sizeof(DXGI_SWAP_CHAIN_DESC));

    //백버퍼 크기
    SwapChainDescriptor.BufferDesc.Width = iWindowSizeX;
    SwapChainDescriptor.BufferDesc.Height = iWindowSizeY;
    //픽셀 설정
    SwapChainDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA
    SwapChainDescriptor.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    SwapChainDescriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    //백버퍼 설정
    SwapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescriptor.BufferCount = 1;
    //스왑 설정
    SwapChainDescriptor.BufferDesc.RefreshRate.Numerator = 240; // fps : this / x 
    SwapChainDescriptor.BufferDesc.RefreshRate.Denominator = 1; // fps : x / this
    //샘플링 옵션
    SwapChainDescriptor.SampleDesc.Quality = 0;
    SwapChainDescriptor.SampleDesc.Count = 1;
    //기본 세팅
    SwapChainDescriptor.OutputWindow = WindowHandle;
    SwapChainDescriptor.Windowed = static_cast<BOOL>(isWindowMode);
    SwapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    //스왑체인 생성
    if (FAILED(pFactory->CreateSwapChain(m_pDevice, &SwapChainDescriptor, &m_pSwapChain)))
        return E_FAIL;

    //임시변수 삭제
    SafeRelease(pFactory);
    SafeRelease(pAdapter);
    SafeRelease(pDevice);

    return S_OK;
}

HRESULT Graphic::ReadyBackBuffer()
{
    CHECKNULLPTR(m_pDevice)
        return E_FAIL;

    //임시변수
    ID3D11Texture2D* pBackBufferTexture = nullptr;

    //스왑체인 텍스처 가져오기
    if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture)))
        return E_FAIL;
    //랜더타겟 텍스처 생성
    if (FAILED(m_pDevice->CreateRenderTargetView(pBackBufferTexture, nullptr, &m_pBackBuffer)))
        return E_FAIL;

    //임시변수 삭제
    SafeRelease(pBackBufferTexture);

    return S_OK;
}

HRESULT Graphic::ReadyDepthStencilView(_uint iWindowSizeX, _uint iWindowSizeY)
{
    CHECKNULLPTR(m_pDevice)
        return E_FAIL;

    //임시변수
    ID3D11Texture2D* pDepthStencilTexture = nullptr;

    //텍스처 세팅
    D3D11_TEXTURE2D_DESC	TextureDescriptor;
    ZeroMemory(&TextureDescriptor, sizeof(D3D11_TEXTURE2D_DESC));
    //DepthStencilBuffer 크기(BackBuffer 크기와 같게)
    TextureDescriptor.Width = iWindowSizeX;
    TextureDescriptor.Height = iWindowSizeY;
    //DepthStencilBuffer 옵션
    TextureDescriptor.MipLevels = 1;
    TextureDescriptor.ArraySize = 1;
    TextureDescriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    TextureDescriptor.SampleDesc.Quality = 0;
    TextureDescriptor.SampleDesc.Count = 1;

    // 정적 or 동적 바인딩 옵션
    TextureDescriptor.Usage = D3D11_USAGE_DEFAULT;
    // 텍스처 사용 목적에 맞게 설정
    TextureDescriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    TextureDescriptor.CPUAccessFlags = 0;
    TextureDescriptor.MiscFlags = 0;

    // 세팅한 값으로 텍스처 생성
    if (FAILED(m_pDevice->CreateTexture2D(&TextureDescriptor, nullptr, &pDepthStencilTexture)))
        return E_FAIL;

    // DepthStencilView 생성
    if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pDepthStencilView)))
        return E_FAIL;

    SafeRelease(pDepthStencilTexture);

    return S_OK;
}
