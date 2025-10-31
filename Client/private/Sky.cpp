#include "Sky.h"

#include "EngineUtility.h"

Sky::Sky()
    : ObjectTemplate{ }
{   
}

Sky::Sky(const Sky& Prototype)
    : ObjectTemplate{ Prototype }
{
}

void Sky::LateUpdate(_float fTimeDelta)
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (pTransform)
    {
        pTransform->SetState(STATE::POSITION, XMLoadFloat4(m_pEngineUtility->GetCamPosition()));
    }
    
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::PRIORITY, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT Sky::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pModel || !pShader || !pTransform)
        return E_FAIL;

    if (FAILED(pTransform->BindShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION))))
        return E_FAIL;

    if (FAILED(pModel->BindShaderResource(0, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
        return E_FAIL;

    pShader->Begin(0);
    pModel->Render(0);

    return S_OK;
}

HRESULT Sky::ReadyComponents()
{   
    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Shader_SphereSky"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;
    /* For.Com_Model */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Sky"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

Sky* Sky::Create()
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
