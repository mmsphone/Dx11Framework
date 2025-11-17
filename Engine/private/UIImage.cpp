#include "UIImage.h"
#include "EngineUtility.h"
#include "Transform.h"

UIImage::UIImage()
    : UI{}
{
}

UIImage::UIImage(const UIImage& Prototype)
    : UI{ Prototype }
    , m_imagePath{ Prototype.m_imagePath }
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

    LoadTextureFromKey();

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
    if (FAILED(__super::Render()))
        return E_FAIL;

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    VIBufferRect* pVIBuffer = static_cast<VIBufferRect*>(FindComponent(TEXT("VIBuffer")));
    Shader* pShader = static_cast<Shader*>(FindComponent(TEXT("Shader")));

    if (FAILED(pShader->BindMatrix("g_WorldMatrix", pTransform->GetWorldMatrixPtr())))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", &m_ViewMatrix)))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", &m_ProjMatrix)))
        return E_FAIL;

    _float4 defaultColor = { 0.f, 0.f, 0.f, 0.f };
    if (FAILED(pShader->BindRawValue("g_vDefaultColor", &defaultColor, sizeof(_float4))))
        return E_FAIL;

    _bool bUse = true;
    if (m_pTexture)
    {
        if (FAILED(pShader->BindRawValue("g_bUseTex", &bUse, sizeof(_bool))))
            return E_FAIL;
        m_pTexture->BindShaderResources(pShader, "g_Texture");
    }
    else
    {
        bUse = false;
        if (FAILED(pShader->BindRawValue("g_bUseTex", &bUse, sizeof(_bool))))
            return E_FAIL;
    }

    _uint passIdx = 0; // UI용 패스 인덱스 (Shader에서 정해둔 걸로)
    pShader->Begin(passIdx);
    pVIBuffer->Render();

    return S_OK;
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

    if (m_pTexture)
        SafeRelease(m_pTexture);
}

void UIImage::SetImagePath(const std::string& path)
{
    m_imagePath = path;
    LoadTextureFromKey();
}

HRESULT UIImage::ReadyComponents()
{
    if (FAILED(AddComponent(0, TEXT("VIBufferRect"), TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(0, TEXT("Shader_VtxPosTex"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

static std::wstring ResolveUIImagePath(const std::string& key)
{
    if (key == "common/swarm_cycle")
        return L"../bin/Resources/UI/gamelobby/Textures/swarm_cycle.png";
    if (key == "maps/any")
        return L"../bin/Resources/UI/gamelobby/Textures/unknownmissionpic.png";
    if (key == "icon_button_arrow_right")
        return L"../bin/Resources/UI/gamelobby/Textures/icon_button_arrow_right.png";
    if (key == "icon_button_arrow_left")
        return L"../bin/Resources/UI/gamelobby/Textures/icon_button_arrow_left.png";
    if (key == "divider_gradient")
        return L"../bin/Resources/UI/gamelobby/Textures/divider_gradient.png";

    return {};
}

HRESULT UIImage::LoadTextureFromKey()
{
    if (m_pTexture)
        SafeRelease(m_pTexture);

    std::wstring path = ResolveUIImagePath(m_imagePath);
    if (path.empty())
        return E_FAIL;
    
    m_pTexture = Texture::Create(path.c_str(), 1);
    
    return S_OK;
}
