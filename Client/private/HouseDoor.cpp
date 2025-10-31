#include "HouseDoor.h"

#include "EngineUtility.h"

HouseDoor::HouseDoor() 
	: ContainerTemplate{} 
{

}
HouseDoor::HouseDoor(const HouseDoor& Prototype)
	: ContainerTemplate{ Prototype } 
{

}

void HouseDoor::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT HouseDoor::Render()
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

HRESULT HouseDoor::SetUpParts()
{
    AddPart(SCENE::GAMEPLAY, L"HouseDoorHandle", 0, nullptr);
    return S_OK;
}

HouseDoor* HouseDoor::Create()
{
    HouseDoor* pInstance = new HouseDoor();
    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : HouseDoor");
        SafeRelease(pInstance);
    }
    return pInstance;
}

Object* HouseDoor::Clone(void* pArg)
{
    HouseDoor* pInstance = new HouseDoor(*this);
    CONTAINER_DESC containerDesc{};
    containerDesc.iNumParts = 1;

    if (FAILED(pInstance->Initialize(&containerDesc)))
    {
        MSG_BOX("Failed to Cloned : HouseDoor");
        SafeRelease(pInstance);
    }
    return pInstance;
}

HRESULT HouseDoor::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Shader_VtxMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_HouseDoor"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;
    return S_OK;
}
