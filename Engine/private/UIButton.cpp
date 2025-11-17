#include "UIButton.h"

#include "EngineUtility.h"
#include "Transform.h"

UIButton::UIButton()
    : UI{}
{
}

UIButton::UIButton(const UIButton& Prototype)
    : UI{ Prototype }
    , m_text{ Prototype.m_text }
    , m_command{ Prototype.m_command }
{
}

HRESULT UIButton::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    // 버튼 공통 리소스 세팅이 필요하면 여기서 (quad, shader 등)
    return S_OK;
}

HRESULT UIButton::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    // 여기서 pArg(UI_DESC)는 이미 UI::Initialize에서 처리함.
    // 추가로 버튼 전용 초기화 필요하면 여기서.
    if (FAILED(ReadyComponents()))
        return E_FAIL;

    return S_OK;
}

void UIButton::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void UIButton::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    // 마우스 입력/포커스에 따른 상태 업데이트는
    // 나중에 UIManager 쪽과 연동해서 구현
}

void UIButton::LateUpdate(_float fTimeDelta)
{
    __super::LateUpdate(fTimeDelta);
}

HRESULT UIButton::Render()
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

    _float4 defaultColor = { 1.f, 0.f, 0.f, 0.6f };
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
}


HRESULT UIButton::ReadyComponents()
{
    if (FAILED(AddComponent(0,TEXT("VIBufferRect"),TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(0,TEXT("Shader_VtxPosTex"),TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}