#include "UIButton.h"

#include "EngineUtility.h"
#include "Transform.h"

UIButton::UIButton()
    : UI{}
{
}

UIButton::UIButton(const UIButton& Prototype)
    : UI{ Prototype }
    , m_pTexture{ nullptr }
    , buttonFunctions{}
{
}

HRESULT UIButton::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT UIButton::Initialize(void* pArg)
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

void UIButton::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void UIButton::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

}

void UIButton::LateUpdate(_float fTimeDelta)
{
    __super::LateUpdate(fTimeDelta);
}

HRESULT UIButton::Render()
{
    if (m_desc.visible == false)
        return S_OK;

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

    _float4 defaultColor = { 1.f, 0.f, 0.f, 0.6f };
    if (FAILED(pShader->BindRawValue("g_vDefaultColor", &defaultColor, sizeof(_float4))))
        return E_FAIL;

    _bool bUse = (m_pTexture != nullptr);
    if (FAILED(pShader->BindRawValue("g_bUseTex", &bUse, sizeof(_bool))))
        return E_FAIL;

    if (m_pTexture)
        m_pTexture->BindShaderResources(pShader, "g_Texture");
    
    pShader->Begin(0);
    pVIBuffer->Render();

    return S_OK;
}

void UIButton::SetImagePath(const std::wstring& path)
{
    m_desc.imagePath = path;
    LoadTextureFromPath();
}

const std::wstring& UIButton::GetImagePath() const
{
    return m_desc.imagePath;
}

void UIButton::AddButtonFunction(function<void()> func)
{
    if (func)
        buttonFunctions.push_back(std::move(func));
}

void UIButton::DoButtonFunctions()
{
    if (m_desc.enable == false)
        return;

    for (auto& func : buttonFunctions)
    {
        if (func)
            func();
    }
}

void UIButton::ClearButtonFunctions()
{
    buttonFunctions.clear();
}

UIButton* UIButton::Create()
{
    UIButton* pInstance = new UIButton();
    if (FAILED(pInstance->InitializePrototype()))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

Object* UIButton::Clone(void* pArg)
{
    UIButton* pInstance = new UIButton(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void UIButton::Free()
{
    __super::Free();

    SafeRelease(m_pTexture);
}


HRESULT UIButton::ReadyComponents()
{
    if (FAILED(AddComponent(0,TEXT("VIBufferRect"),TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(0,TEXT("Shader_VtxPosTex"),TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT UIButton::LoadTextureFromPath()
{
    SafeRelease(m_pTexture);

    if (m_desc.imagePath.empty())
        return S_OK; // 이미지 없는 버튼도 허용

    m_pTexture = Texture::Create(m_desc.imagePath.c_str(), 1);
    if (m_pTexture == nullptr)
    {
        std::string str = "[UIButton] Failed to load texture: ";
        DEBUG_OUTPUT(str);
        return E_FAIL;
    }

    return S_OK;
}