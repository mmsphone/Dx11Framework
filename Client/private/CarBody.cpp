#include "CarBody.h"

#include "EngineUtility.h"

CarBody::CarBody() 
	: ContainerTemplate{} 
{

}
CarBody::CarBody(const CarBody& Prototype)
	: ContainerTemplate{ Prototype } 
{

}

void CarBody::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT CarBody::Render()
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

HRESULT CarBody::SetUpParts()
{
    AddPart(SCENE::GAMEPLAY, L"CarWheel", 0, nullptr); 
    AddPart(SCENE::GAMEPLAY, L"CarWheel", 1, nullptr); 
    AddPart(SCENE::GAMEPLAY, L"CarWheel", 2, nullptr); 
    AddPart(SCENE::GAMEPLAY, L"CarWheel", 3, nullptr);
    AddPart(SCENE::GAMEPLAY, L"CarTrunk", 4, nullptr);
    AddPart(SCENE::GAMEPLAY, L"CarTrunkHandle", 5, nullptr);
    AddPart(SCENE::GAMEPLAY, L"CarTopStuff", 6, nullptr);
    return S_OK;
}

CarBody* CarBody::Create()
{
    CarBody* pInstance = new CarBody();
    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : CarBody");
        SafeRelease(pInstance);
    }
    return pInstance;
}

Object* CarBody::Clone(void* pArg)
{
    CarBody* pInstance = new CarBody(*this);
    CONTAINER_DESC containerDesc{};
    containerDesc.iNumParts = 7;

    if (FAILED(pInstance->Initialize(&containerDesc)))
    {
        MSG_BOX("Failed to Cloned : CarBody");
        SafeRelease(pInstance);
    }
    return pInstance;
}

HRESULT CarBody::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Shader_VtxMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_CarBody"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;
    return S_OK;
}
