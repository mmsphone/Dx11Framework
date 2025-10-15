#include "FieldObject.h"

#include "EngineUtility.h"

FieldObject::FieldObject()
    : Object{ }
{
}

FieldObject::FieldObject(const FieldObject& Prototype)
    : Object{ Prototype }
{
}

HRESULT FieldObject::InitializePrototype()
{
    return S_OK;
}

HRESULT FieldObject::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;
    
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));

    pTransform->SetState(STATE::POSITION, XMVectorSet(
        0.f,
        0.f,
        1.f,
        1.f
    ));

    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    pModel->SetAnimation(0, true);

    return S_OK;
}

void FieldObject::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void FieldObject::Update(_float fTimeDelta)
{
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    pModel->PlayAnimation(fTimeDelta);

    __super::Update(fTimeDelta);
}

void FieldObject::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT FieldObject::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));

    if (FAILED(pTransform->BindShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;

    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::PROJECTION))))
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

    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    _uint       iNumMeshes = pModel->GetNumMeshes();

    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(pModel->BindShaderResource(i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
            return E_FAIL;

        pModel->BindBoneMatrices(i, pShader, "g_BoneMatrices");
        pShader->Begin(0);
        pModel->Render(i);
    }

    return S_OK;
}

HRESULT FieldObject::ReadyComponents()
{
    /* For.Com_Model */
    if (FAILED(AddComponent(SCENE::MAP, TEXT("Prototype_Component_Model_Bullet"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::MAP, TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

Object* FieldObject::Create()
{
    FieldObject* pInstance = new FieldObject();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : FieldObject");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* FieldObject::Clone(void* pArg)
{
    FieldObject* pInstance = new FieldObject(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : FieldObject");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void FieldObject::Free()
{
    __super::Free();
}
