#include "Graphic.h"

Graphic::Graphic()
{
}

HRESULT Graphic::Initialize(HWND windowHandle, WINMODE isWindowMode, _uint iWindowSizeX, _uint iWindowSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
    _uint isDebug = 0;
#ifdef _DEBUG
    isDebug = D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL FeatureLevel;

    //Create Device
    if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, isDebug, nullptr, 0, D3D11_SDK_VERSION, &m_pDevice, &FeatureLevel, &m_pContext)))
        return E_FAIL;
    //Create SwapChain
    if (FAILED(Ready_SwapChain(windowHandle, isWindowMode, iWindowSizeX, iWindowSizeY)))
        return E_FAIL;
    //Create BackBuffer
    if (FAILED(Ready_BackBuffer()))
        return E_FAIL;
    //Create DepthStencilBuffer
    if (FAILED(Ready_DepthStencilView(iWindowSizeX, iWindowSizeY)))
        return E_FAIL;

    //Set RenderTargets and DepthStencil(Maximum 8 RenderTargets)
    ID3D11RenderTargetView* pRenderTargets[] = {
        m_pBackBuffer,
    };
    _uint RenderTargetsCount = 1;
    m_pContext->OMSetRenderTargets(RenderTargetsCount, pRenderTargets, m_pDepthStencilView);

    D3D11_VIEWPORT ViewPortDescriptor;
    //On Viewport Location
    ViewPortDescriptor.TopLeftX = 0;
    ViewPortDescriptor.TopLeftY = 0;
    //On ViewPort Size
    ViewPortDescriptor.Width = static_cast<_float>(iWindowSizeX);
    ViewPortDescriptor.Height = static_cast<_float>(iWindowSizeY);
    //Range of Depth
    ViewPortDescriptor.MinDepth = 0.f;
    ViewPortDescriptor.MaxDepth = 1.f;
    _uint ViewPortCount = 1;

    m_pContext->RSSetViewports(ViewPortCount, &ViewPortDescriptor);

    //End of Create, Save to member
    *ppDevice = m_pDevice;
    Safe_AddRef(m_pDevice);

    *ppContext = m_pContext;
    Safe_AddRef(m_pContext);

    return S_OK;
}

HRESULT Graphic::ClearBackBufferView(const _float4* pClearColor)
{
    CHECKNULLPTR(m_pContext)
        return E_FAIL;

    //Clear BackBuffer With Color
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

Graphic* Graphic::Create(HWND windowHandle, WINMODE isWindowMode, _uint iWindowSizeX, _uint iWindowSizeY, ID3D11Device** ppDevice, ID3D11DeviceContext** ppContext)
{
    Graphic* pInstance = new Graphic();
    
    if (FAILED(pInstance->Initialize(windowHandle, isWindowMode, iWindowSizeX, iWindowSizeY, ppDevice, ppContext)))
    {
        MSG_BOX("Failed to Create : Graphic");
        Safe_Release(pInstance);
    }

    return pInstance;
}

void Graphic::Free()
{
    __super::Free();
    Safe_Release(m_pSwapChain);
    Safe_Release(m_pDepthStencilView);
    Safe_Release(m_pBackBuffer);
    Safe_Release(m_pContext);

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

    Safe_Release(m_pDevice);
}

HRESULT Graphic::Ready_SwapChain(HWND WindowHandle, WINMODE isWindowMode, _uint iWindowSizeX, _uint iWindowSizeY)
{
    //Temp Variables
    IDXGIDevice* pDevice = nullptr;
    m_pDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&pDevice);

    IDXGIAdapter* pAdapter = nullptr;
    pDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&pAdapter);

    IDXGIFactory* pFactory = nullptr;
    pAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&pFactory);

    //Set SwapChainDescriptor
    DXGI_SWAP_CHAIN_DESC		SwapChainDescriptor;
    ZeroMemory(&SwapChainDescriptor, sizeof(DXGI_SWAP_CHAIN_DESC));

    //BackBuffer Size
    SwapChainDescriptor.BufferDesc.Width = iWindowSizeX;
    SwapChainDescriptor.BufferDesc.Height = iWindowSizeY;
    //Pixel Option
    SwapChainDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // RGBA
    SwapChainDescriptor.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    SwapChainDescriptor.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    //BackBuffer Option
    SwapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDescriptor.BufferCount = 1;
    //Swap Option
    SwapChainDescriptor.BufferDesc.RefreshRate.Numerator = 240; // fps : this / x 
    SwapChainDescriptor.BufferDesc.RefreshRate.Denominator = 1; // fps : x / this
    //Sampling Option
    SwapChainDescriptor.SampleDesc.Quality = 0;
    SwapChainDescriptor.SampleDesc.Count = 1;
    //basic set
    SwapChainDescriptor.OutputWindow = WindowHandle;
    SwapChainDescriptor.Windowed = static_cast<BOOL>(isWindowMode);
    SwapChainDescriptor.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    //Create SwapChain
    if (FAILED(pFactory->CreateSwapChain(m_pDevice, &SwapChainDescriptor, &m_pSwapChain)))
        return E_FAIL;

    //Remove Temp Variables
    Safe_Release(pFactory);
    Safe_Release(pAdapter);
    Safe_Release(pDevice);

    return S_OK;
}

HRESULT Graphic::Ready_BackBuffer()
{
    CHECKNULLPTR(m_pDevice)
        return E_FAIL;

    //Temp Variable
    ID3D11Texture2D* pBackBufferTexture = nullptr;

    //Get SwapChain Texture
    if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBufferTexture)))
        return E_FAIL;
    //Create RenderTarget Texture
    if (FAILED(m_pDevice->CreateRenderTargetView(pBackBufferTexture, nullptr, &m_pBackBuffer)))
        return E_FAIL;

    //Remove Temp Variable
    Safe_Release(pBackBufferTexture);

    return S_OK;
}

HRESULT Graphic::Ready_DepthStencilView(_uint iWindowSizeX, _uint iWindowSizeY)
{
    CHECKNULLPTR(m_pDevice)
        return E_FAIL;

    //Temp Variable
    ID3D11Texture2D* pDepthStencilTexture = nullptr;

    //Set TextureDescriptor
    D3D11_TEXTURE2D_DESC	TextureDescriptor;
    ZeroMemory(&TextureDescriptor, sizeof(D3D11_TEXTURE2D_DESC));
    //DepthStencilBuffer Size(== BackBuffer Size)
    TextureDescriptor.Width = iWindowSizeX;
    TextureDescriptor.Height = iWindowSizeY;
    //DepthStencilBuffer Option
    TextureDescriptor.MipLevels = 1;
    TextureDescriptor.ArraySize = 1;
    TextureDescriptor.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    TextureDescriptor.SampleDesc.Quality = 0;
    TextureDescriptor.SampleDesc.Count = 1;

    // binding with static or dynamic
    TextureDescriptor.Usage = D3D11_USAGE_DEFAULT;
    // Texture Usage
    TextureDescriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    TextureDescriptor.CPUAccessFlags = 0;
    TextureDescriptor.MiscFlags = 0;

    //Create Texture With Descriptor
    if (FAILED(m_pDevice->CreateTexture2D(&TextureDescriptor, nullptr, &pDepthStencilTexture)))
        return E_FAIL;

    //Create DepthStencil
    if (FAILED(m_pDevice->CreateDepthStencilView(pDepthStencilTexture, nullptr, &m_pDepthStencilView)))
        return E_FAIL;

    Safe_Release(pDepthStencilTexture);

    return S_OK;
}
