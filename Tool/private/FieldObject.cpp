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
        0.f,
        1.f
    ));

    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    pModel->SetAnimation(0, true);
    pModel->StopAnimation();

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

#ifdef _DEBUG
    Collision* pCollision = dynamic_cast<Collision*>(FindComponent(TEXT("Collision")));
    if (pCollision != nullptr)
    {
        Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
        if (pTransform != nullptr)
            pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));
    }
#endif

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
    Shader* pShader2 = dynamic_cast<Shader*>(FindComponent(TEXT("Shader2")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pTransform || !pShader || !pShader2 || !pModel)
        return E_FAIL;

    {
        std::ostringstream oss;
        oss << "[RenderCheck] Sky "
            << "shader=" << (pShader ? "OK" : "NULL")
            << "shader2=" << (pShader2 ? "OK" : "NULL")
            << " model=" << (pModel ? "OK" : "NULL")
            << "\n";
        OutputDebugStringA(oss.str().c_str());
    }

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
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

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader2, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader2->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader2->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader2->BindRawValue("g_vCamPosition", m_pEngineUtility->GetCamPosition(), sizeof(_float4))))
        return E_FAIL;
    if (FAILED(pShader2->BindRawValue("g_vLightDir", &pLightDesc->vDirection, sizeof(_float4))))
        return E_FAIL;
    if (FAILED(pShader2->BindRawValue("g_vLightDiffuse", &pLightDesc->vDiffuse, sizeof(_float4))))
        return E_FAIL;
    if (FAILED(pShader2->BindRawValue("g_vLightAmbient", &pLightDesc->vAmbient, sizeof(_float4))))
        return E_FAIL;
    if (FAILED(pShader2->BindRawValue("g_vLightSpecular", &pLightDesc->vSpecular, sizeof(_float4))))
        return E_FAIL;

    _uint       iNumMeshes = pModel->GetNumMeshes();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (pModel->GetModelData()->bones.size() > 0)
        {
            if (FAILED(pModel->BindRenderTargetShaderResource(i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
            {
                OutputDebugStringA("[RenderCheck] DiffuseTexture bind failed\n");
                continue;
            }
            OutputDebugStringA("[RenderCheck] DiffuseTexture bound OK\n");

            pModel->BindBoneMatrices(i, pShader, "g_BoneMatrices"); 
            pShader->Begin(0);
            pModel->Render(i);
        }
        else
        {
            if (FAILED(pModel->BindRenderTargetShaderResource(i, pShader2, "g_DiffuseTexture", TextureType::Diffuse, 0)))
            {
                OutputDebugStringA("[RenderCheck] DiffuseTexture bind failed\n");
                continue;
            }
            OutputDebugStringA("[RenderCheck] DiffuseTexture bound OK\n");

            pShader2->Begin(0);
            pModel->Render(i);
        }
    }
#ifdef _DEBUG
    Collision* pCollision = dynamic_cast<Collision*>(FindComponent(TEXT("Collision")));
    if (pCollision != nullptr)
        pCollision->Render();
#endif

    return S_OK;
}

HRESULT FieldObject::ReadyComponents()
{
    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::MAP, TEXT("Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::MAP, TEXT("Shader_VtxMesh"), TEXT("Shader2"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(AddComponent(SCENE::MAP, TEXT("Model_Fiona"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    // For.Com_CollisionAABB
    CollisionBoxAABB::COLLISIONAABB_DESC     AABBDesc{};
    AABBDesc.vSize = _float3(2.f, 2.f, 2.f);
    AABBDesc.vCenter = _float3(0.f, 0.f, 0.f);
    if(FAILED(AddComponent(SCENE::MAP, TEXT("CollisionAABB"), TEXT("Collision"), nullptr, &AABBDesc)))
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
