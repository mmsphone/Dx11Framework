#include "UIPanel.h"

#include "EngineUtility.h"
#include "Transform.h"

UIPanel::UIPanel()
    : UI{}
{
}

UIPanel::UIPanel(const UIPanel& Prototype)
    : UI{ Prototype }
{
}

HRESULT UIPanel::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT UIPanel::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    return S_OK;
}

void UIPanel::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void UIPanel::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void UIPanel::LateUpdate(_float fTimeDelta)
{
    __super::LateUpdate(fTimeDelta);
}

HRESULT UIPanel::Render()
{
    if (FAILED(__super::Render()))
        return E_FAIL;

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    VIBufferRect* pVIBuffer = static_cast<VIBufferRect*>(FindComponent(TEXT("VIBuffer")));
    Shader* pShader = static_cast<Shader*>(FindComponent(TEXT("Shader")));
    Texture* pTexture = dynamic_cast<Texture*>(FindComponent(TEXT("Texture")));

    if (FAILED(pShader->BindMatrix("g_WorldMatrix", pTransform->GetWorldMatrixPtr())))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", &m_ViewMatrix)))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", &m_ProjMatrix)))
        return E_FAIL;

    _float4 defaultColor = { 1.f, 1.f, 1.f, 0.6f };
    if (FAILED(pShader->BindRawValue("g_vDefaultColor", &defaultColor, sizeof(_float4))))
        return E_FAIL;

    _bool bUse = true;
    if (pTexture)
    {
        if (FAILED(pShader->BindRawValue("g_bUseTex", &bUse, sizeof(_bool))))
            return E_FAIL;
        pTexture->BindShaderResources(pShader, "g_Texture");
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

UIPanel* UIPanel::Create()
{
    UIPanel* pInstance = new UIPanel();
    if (FAILED(pInstance->InitializePrototype()))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

Object* UIPanel::Clone(void* pArg)
{
    UIPanel* pInstance = new UIPanel(*this);
    if (FAILED(pInstance->Initialize(pArg)))
    {
        SafeRelease(pInstance);
        return nullptr;
    }
    return pInstance;
}

void UIPanel::Free()
{
    __super::Free();
}

HRESULT UIPanel::ReadyComponents()
{
    if (FAILED(AddComponent(0, TEXT("VIBufferRect"), TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(0, TEXT("Shader_VtxPosTex"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}