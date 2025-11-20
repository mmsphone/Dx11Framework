#include "UIImage.h"
#include "EngineUtility.h"
#include "Transform.h"

UIImage::UIImage()
    : UI{}
{
}

UIImage::UIImage(const UIImage& Prototype)
    : UI{ Prototype }
    , m_pTexture { nullptr }
{
}

HRESULT UIImage::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT UIImage::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    if (m_desc.imagePath.empty() == false)
    {
        if (FAILED(LoadTextureFromPath()))
            return E_FAIL;
    }

    return S_OK;
}

void UIImage::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void UIImage::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void UIImage::LateUpdate(_float fTimeDelta)
{
    __super::LateUpdate(fTimeDelta);
}

HRESULT UIImage::Render()
{
    if (m_desc.visible == false)
        return S_OK;

    if (FAILED(__super::Render()))
        return E_FAIL;

    Transform* pTransform = static_cast<Transform*>(FindComponent(L"Transform"));
    VIBufferRect* pVIBuffer = static_cast<VIBufferRect*>(FindComponent(L"VIBuffer"));
    Shader* pShader = static_cast<Shader*>(FindComponent(L"Shader"));

    if (FAILED(pShader->BindMatrix("g_WorldMatrix", pTransform->GetWorldMatrixPtr()))) 
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", &m_ViewMatrix))) 
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", &m_ProjMatrix))) 
        return E_FAIL;

    _float4 defaultColor = { 0.f, 0.f, 0.f, 0.f };
    if (FAILED(pShader->BindRawValue("g_vDefaultColor", &defaultColor, sizeof(_float4)))) 
        return E_FAIL;

    _bool bUse = (m_pTexture != nullptr);
    if (FAILED(pShader->BindRawValue("g_bUseTex", &bUse, sizeof(_bool))))
        return E_FAIL;

    if (m_pTexture)
        m_pTexture->BindShaderResources(pShader, "g_Texture");
   
    _uint passIndex = 0;

    pShader->BindRawValue("g_iMaskingType", &m_MaskingType, sizeof(_int));
    if (m_MaskingType == 1)
    {
        _float4 scissorRect =
        {
            static_cast<_float>(m_scissorRect.left),
            static_cast<_float>(m_scissorRect.top),
            static_cast<_float>(m_scissorRect.right),
            static_cast<_float>(m_scissorRect.bottom)
        };

        pShader->BindRawValue("g_vScissorRect", &scissorRect, sizeof(_float4));

        passIndex = 2;   // ★ 마스크 패스
    }
    else if (m_MaskingType == 2)
    {
        pShader->BindRawValue("g_hpRatio", &m_Ratio, sizeof(_float));
        passIndex = 2;
    }
    else
    {
        passIndex = 0;   // 기본 UI 패스
    }

    pShader->Begin(passIndex);
    pVIBuffer->BindBuffers();
    pVIBuffer->Render();

    return S_OK;
}

void UIImage::SetScissor(const D3D11_RECT& rect)
{
    m_scissorRect = rect;
    m_MaskingType = 1;
}

void UIImage::SetRatio(const _float hpRatio)
{
    m_Ratio = hpRatio;
    m_MaskingType = 2;
}

void UIImage::ClearMasking()
{
    m_MaskingType = 0;
}

UIImage* UIImage::Create()
{
    UIImage* pInstance = new UIImage();
    if (FAILED(pInstance->InitializePrototype()))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

Object* UIImage::Clone(void* pArg)
{
    UIImage* pInstance = new UIImage(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void UIImage::Free()
{
    __super::Free();

    SafeRelease(m_pTexture);
}

HRESULT UIImage::ReadyComponents()
{
    if (FAILED(AddComponent(0, TEXT("VIBufferRect"), TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(0, TEXT("Shader_VtxPosTex"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT UIImage::LoadTextureFromPath()
{
    SafeRelease(m_pTexture);

    if (m_desc.imagePath.empty())
        return E_FAIL;

    m_pTexture = Texture::Create(m_desc.imagePath.c_str(), 1);
    if (m_pTexture == nullptr)
    {
        string str = "[UIImage] Failed to load texture: ";
        DEBUG_OUTPUT(str);
        return E_FAIL;
    }
    
    return S_OK;
}
