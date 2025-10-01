#include "Sky.h"

#include "EngineUtility.h"

Sky::Sky()
    : Object{ }
{
}

Sky::Sky(const Sky& Prototype)
    : Object{ Prototype }
{
}

HRESULT Sky::InitializePrototype()
{
    return S_OK;
}

HRESULT Sky::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    return S_OK;
}

void Sky::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void Sky::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void Sky::LateUpdate(_float fTimeDelta)
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    pTransform->SetState(STATE::POSITION, XMLoadFloat4(m_pEngineUtility->GetCamPosition()));

    /* 렌더러의 그룹들 중 어떤 순서로 그려져야할지 적절한 위치에 추가해준다. */
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::PRIORITY, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT Sky::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Texture* pTexture = dynamic_cast<Texture*>(FindComponent(TEXT("Texture")));
    VIBufferCube* pCube = dynamic_cast<VIBufferCube*>(FindComponent(TEXT("VIBuffer")));
    if (FAILED(pTransform->BindShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::PROJECTION))))
        return E_FAIL;

    if (FAILED(pTexture->BindShaderResource(pShader, "g_Texture", 2)))
        return E_FAIL;

    pShader->Begin(0);
    pCube->BindBuffers();
    pCube->Render();

    return S_OK;
}

HRESULT Sky::ReadyComponents()
{
    /* For.Com_VIBuffer */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Cube"), TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Texture */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_Texture_Sky"), TEXT("Texture"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_Shader_VtxCube"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

Object* Sky::Create()
{
    Sky* pInstance = new Sky();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Sky");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* Sky::Clone(void* pArg)
{
    Sky* pInstance = new Sky(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Sky");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Sky::Free()
{
    __super::Free();
}
