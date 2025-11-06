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

    m_WindowHandle = windowHandle;
    m_WindowSize.x = static_cast<_float>(iWindowSizeX);
    m_WindowSize.y = static_cast<_float>(iWindowSizeY);

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

    return m_pSwapChain->Present(0, 0);
}

ID3D11Device* Graphic::GetDevice() const
{
    return m_pDevice;
}

ID3D11DeviceContext* Graphic::GetContext() const
{
    return m_pContext;
}

HWND Graphic::GetWindowHandle() const
{
    return m_WindowHandle;
}

_float2 Graphic::GetWindowSize() const
{
    return m_WindowSize;
}
void Graphic::SnapDepthForPicking()
{
    if (!m_pContext || !m_pDepthStencilTex || !m_pDepthReadback)
        return;

    // DSV는 부분복사 불가 → 전체 서브리소스 복사
    m_pContext->CopySubresourceRegion(
        m_pDepthReadback, 0, 0, 0, 0,
        m_pDepthStencilTex, 0, nullptr
    );
}

_bool Graphic::ReadDepthAtPixel(_int px, _int py, _float* outDepth01)
{
    if (!m_pContext || !m_pDepthReadback || !outDepth01)
        return false;

    // ★ 여기서는 복사하지 않음! (렌더 끝에서 SnapDepthForPicking 호출을 가정)

    D3D11_MAPPED_SUBRESOURCE m{};
    if (FAILED(m_pContext->Map(m_pDepthReadback, 0, D3D11_MAP_READ, 0, &m)))
        return false;

    // ★ RS Viewport 기준으로 좌표 클램프 (창 크기와 다를 수 있음)
    D3D11_VIEWPORT vp{}; UINT vpCount = 1;
    m_pContext->RSGetViewports(&vpCount, &vp);
    int w = (int)vp.Width;
    int h = (int)vp.Height;

    int ix = px; if (ix < 0) ix = 0; else if (ix >= w) ix = w - 1;
    int iy = py; if (iy < 0) iy = 0; else if (iy >= h) iy = h - 1;

    const uint8_t* row = (const uint8_t*)m.pData + iy * m.RowPitch;
    uint32_t packed = *(const uint32_t*)(row + ix * 4);

    m_pContext->Unmap(m_pDepthReadback, 0);

    // 하위 24비트 = depth
    uint32_t depth24 = packed & 0x00FFFFFFu;           // [0, 2^24-1]
    float    depth01 = float(depth24) / 16777215.0f;   // 2^24-1

    if (depth01 <= 1e-7f || depth01 >= 0.9999999f)     // 클리어/미스 보호
        return false;

    *outDepth01 = depth01;
    return true;
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
        
    if (m_pContext)
    {
        m_pContext->ClearState();
        m_pContext->Flush();
    }

    SafeRelease(m_pBackBuffer);
    SafeRelease(m_pDepthStencilView);
    SafeRelease(m_pDepthStencilTex);
    SafeRelease(m_pDepthReadback);

    if (m_pSwapChain)
    {
        ID3D11Texture2D* backBuffer = nullptr;
        m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        SafeRelease(backBuffer);
    }
    SafeRelease(m_pSwapChain);

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

    SafeRelease(m_pDepthStencilView);
    SafeRelease(m_pDepthStencilTex);
    SafeRelease(m_pDepthReadback);

    // ★ Typeless로 만들고
    D3D11_TEXTURE2D_DESC td{};
    td.Width = iWindowSizeX;
    td.Height = iWindowSizeY;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R24G8_TYPELESS;
    td.SampleDesc.Count = 1;                 // ★ MSAA off (Count=1)
    td.SampleDesc.Quality = 0;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    td.CPUAccessFlags = 0;
    td.MiscFlags = 0;

    if (FAILED(m_pDevice->CreateTexture2D(&td, nullptr, &m_pDepthStencilTex)))
        return E_FAIL;

    // ★ DSV는 D24로 명시
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvd{};
    dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    dsvd.Texture2D.MipSlice = 0;

    if (FAILED(m_pDevice->CreateDepthStencilView(m_pDepthStencilTex, &dsvd, &m_pDepthStencilView)))
        return E_FAIL;

    // ★ Readback용 1:1 staging 텍스처(Count=1)
    D3D11_TEXTURE2D_DESC rd = td;
    rd.BindFlags = 0;
    rd.Usage = D3D11_USAGE_STAGING;
    rd.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

    if (FAILED(m_pDevice->CreateTexture2D(&rd, nullptr, &m_pDepthReadback)))
        return E_FAIL;

    return S_OK;
}
