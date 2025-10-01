#include "PlayerBody.h"

#include "EngineUtility.h"
#include "Player.h"

PlayerBody::PlayerBody()
    : Part{ }
{
}

PlayerBody::PlayerBody(const PlayerBody& Prototype)
    : Part{ Prototype }
{
}

const _float4x4* PlayerBody::GetSocketBoneMatrixPtr(const _char* pBoneName)
{
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    return pModel->GetSocketBoneMatrixPtr(pBoneName);
}

HRESULT PlayerBody::InitializePrototype()
{
    return S_OK;
}

HRESULT PlayerBody::Initialize(void* pArg)
{
    PLAYERBODY_DESC* pDesc = static_cast<PLAYERBODY_DESC*>(pArg);
    m_pParentState = pDesc->pParentState;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    pModel->SetAnimation(3, false);

    return S_OK;
}

void PlayerBody::PriorityUpdate(_float fTimeDelta)
{
}

void PlayerBody::Update(_float fTimeDelta)
{
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));

    if (*m_pParentState & Player::STATE::IDLE)
    {
        pModel->SetAnimation(3, true);
    }

    if (*m_pParentState & Player::STATE::RUN)
    {
        pModel->SetAnimation(4, true);
    }

    pModel->PlayAnimation(fTimeDelta);

    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    XMStoreFloat4x4(&m_CombinedWorldMatrix,
        XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()) * XMLoadFloat4x4(m_pParentMatrix));
}

void PlayerBody::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT PlayerBody::Render()
{
    if (FAILED(BindShaderResources()))
        return E_FAIL;

    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    _uint       iNumMeshes = pModel->GetNumMeshes();

    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
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

HRESULT PlayerBody::ReadyComponents()
{
    /* For.Com_Model */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_Model_Fiona"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;

}

HRESULT PlayerBody::BindShaderResources()
{
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));

    if (FAILED(pShader->BindMatrix("g_WorldMatrix", &m_CombinedWorldMatrix)))
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

    return S_OK;
}


Object* PlayerBody::Create()
{
    PlayerBody* pInstance = new PlayerBody();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : PlayerBody");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* PlayerBody::Clone(void* pArg)
{
    PlayerBody* pInstance = new PlayerBody(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : PlayerBody");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void PlayerBody::Free()
{
    __super::Free();
}
