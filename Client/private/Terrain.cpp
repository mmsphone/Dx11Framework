#include "Terrain.h"

#include "EngineUtility.h"



Terrain::Terrain()
    : Object{ }
{
}

Terrain::Terrain(const Terrain& Prototype)
    : Object{ Prototype }
{
}

HRESULT Terrain::InitializePrototype()
{
    return S_OK;
}

HRESULT Terrain::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    return S_OK;
}

void Terrain::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void Terrain::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);
}

void Terrain::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);
    __super::LateUpdate(fTimeDelta);
}

HRESULT Terrain::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Texture* pTextureDiffuse = dynamic_cast<Texture*>(FindComponent(TEXT("Texture_Diffuse")));
    Texture* pTextureMask = dynamic_cast<Texture*>(FindComponent(TEXT("Texture_Mask")));
    Texture* pTextureBrush = dynamic_cast<Texture*>(FindComponent(TEXT("Texture_Brush")));
    VIBufferTerrain* pTerrain = dynamic_cast<VIBufferTerrain*>(FindComponent(TEXT("VIBuffer")));

    if (FAILED(pTransform->BindShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::PROJECTION))))
        return E_FAIL;



    if (FAILED(pTextureDiffuse->BindShaderResources(pShader, "g_DiffuseTexture")))
        return E_FAIL;
    if (FAILED(pTextureMask->BindShaderResource(pShader, "g_MaskTexture", 0)))
        return E_FAIL;
    if (FAILED(pTextureBrush->BindShaderResource(pShader, "g_BrushTexture", 0)))
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

    pShader->Begin(0);
    pTerrain->BindBuffers();
    pTerrain->Render();

    return S_OK;
}

HRESULT Terrain::ReadyComponents()
{
    /* For.Com_VIBuffer */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_VIBufferTerrain"), TEXT("VIBuffer"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Texture_Diffuse */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Diffuse"), TEXT("Texture_Diffuse"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Texture_Mask */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Mask"), TEXT("Texture_Mask"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Texture_Brush */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_Texture_Terrain_Brush"), TEXT("Texture_Brush"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_Shader_VtxNorTex"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

Object* Terrain::Create()
{
    Terrain* pInstance = new Terrain();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Terrain");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* Terrain::Clone(void* pArg)
{
    Terrain* pInstance = new Terrain(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Terrain");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Terrain::Free()
{
    __super::Free();
}
