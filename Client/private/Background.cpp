#include "Background.h"

#include "EngineUtility.h"

Background::Background()
    : UI { }
{
}

Background::Background(const Background& Prototype)
    : UI{ Prototype }
{
}

HRESULT Background::InitializePrototype()
{
    return S_OK;
}

HRESULT Background::Initialize(void* pArg)
{
    BACKGROUND_DESC     Desc{};

    Desc.fX = 150.0f;
    Desc.fY = 150.0f;
    Desc.fSizeX = 300.0f;
    Desc.fSizeY = 300.0f;

    Desc.fSpeedPerSec = 5.f;

    if (FAILED(__super::Initialize(&Desc)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    return S_OK;
}

void Background::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void Background::Update(_float fTimeDelta)
{
    m_fX += 5.0f * fTimeDelta;

    __super::Update(fTimeDelta);
}

void Background::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::UI, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT Background::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Texture* pTexture = dynamic_cast<Texture*>(FindComponent(TEXT("Texture")));
    VIBufferRect* pRect = dynamic_cast<VIBufferRect*>(FindComponent(TEXT("VIBuffer")));

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", &m_ViewMatrix)))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", &m_ProjMatrix)))
        return E_FAIL;
    if (FAILED(pTexture->BindRenderTargetShaderResource(pShader, "g_Texture", 0)))
        return E_FAIL;

    pShader->Begin(0);
    pRect->BindBuffers();
    pRect->Render();

    return S_OK;
}

HRESULT Background::ReadyComponents()
{
    /* For.Com_Texture */
    if (FAILED(AddComponent(SCENE::LOGO, TEXT("Texture_BackGround"), TEXT("Texture"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::LOGO, TEXT("Shader_VtxPosTex"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_VIBuffer */
    if (FAILED(AddComponent(SCENE::LOGO, TEXT("VIBuffer_Rect"), TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

Object* Background::Create()
{
    Background* pInstance = new Background();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Background");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* Background::Clone(void* pArg)
{
    Background* pInstance = new Background(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Background");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Background::Free()
{
    __super::Free();
}

