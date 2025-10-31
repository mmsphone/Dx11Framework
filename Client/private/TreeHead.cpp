#include "TreeHead.h"

#include "EngineUtility.h"

TreeHead::TreeHead()
    : ObjectTemplate{ }
{
}

TreeHead::TreeHead(const TreeHead& Prototype)
    : ObjectTemplate{ Prototype }
{
}

void TreeHead::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT TreeHead::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pTransform || !pShader || !pModel)
        return E_FAIL;

    if (FAILED(pTransform->BindShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vCamPosition", m_pEngineUtility->GetCamPosition(), sizeof(_float4))))
        return E_FAIL;

    const LIGHT_DESC* pLightDesc = m_pEngineUtility->GetLight(0);
    if (nullptr == pLightDesc)
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
        return E_FAIL;

    if (FAILED(pModel->BindShaderResource(0, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
        return E_FAIL;

    pShader->Begin(0);
    pModel->Render(0);

    return S_OK;
}

HRESULT TreeHead::ReadyComponents()
{
    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Shader_VtxMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_TreeHead"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

TreeHead* TreeHead::Create()
{
    TreeHead* pInstance = new TreeHead();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : TreeHead");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* TreeHead::Clone(void* pArg)
{
    TreeHead* pInstance = new TreeHead(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : TreeHead");
        SafeRelease(pInstance);
    }

    return pInstance;
}
