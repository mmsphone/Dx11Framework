#include "TestObject.h"

#include "EngineUtility.h"

TestObject::TestObject()
    : Object{ }
{
}

TestObject::TestObject(const TestObject& Prototype)
    : Object{ Prototype }
{
}

HRESULT TestObject::InitializePrototype()
{
    return S_OK;
}

HRESULT TestObject::Initialize(void* pArg)
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

void TestObject::PriorityUpdate(_float fTimeDelta)
{
    __super::PriorityUpdate(fTimeDelta);
}

void TestObject::Update(_float fTimeDelta)
{
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    pModel->PlayAnimation(fTimeDelta);

    __super::Update(fTimeDelta);
}

void TestObject::LateUpdate(_float fTimeDelta)
{

    /* 렌더러의 그룹들 중 어떤 순서로 그려져야할지 적절한 위치에 추가해준다. */
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT TestObject::Render()
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

    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(pModel->BindShaderResource(i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
            continue;

        pModel->BindBoneMatrices(i, pShader, "g_BoneMatrices");
        pShader->Begin(0);
        pModel->Render(i);
    }

    return S_OK;
}

HRESULT TestObject::ReadyComponents()
{
    /* For.Com_Model */
    if (FAILED(AddComponent(SCENE::MODEL, TEXT("Model_Fiona"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::MODEL, TEXT("Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

Object* TestObject::Create()
{
    TestObject* pInstance = new TestObject();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : TestObject");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* TestObject::Clone(void* pArg)
{
    TestObject* pInstance = new TestObject(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : TestObject");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void TestObject::Free()
{
    __super::Free();

}
