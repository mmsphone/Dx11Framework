#include "PlayerWeapon.h"

#include "EngineUtility.h"
#include "Player.h"

PlayerWeapon::PlayerWeapon()
    : Part{ }
{
}

PlayerWeapon::PlayerWeapon(const PlayerWeapon& Prototype)
    : Part{ Prototype }
{
}

HRESULT PlayerWeapon::InitializePrototype()
{
    return S_OK;
}

HRESULT PlayerWeapon::Initialize(void* pArg)
{
    PLAYERWEAPON_DESC* pDesc = static_cast<PLAYERWEAPON_DESC*>(pArg);

    m_pParentState = pDesc->pParentState;
    m_pSocketBoneMatrix = pDesc->pSocketBoneMatrix;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    pTransform->SetScale(0.1f, 0.1f, 0.1f);
    pTransform->RotateRadian(XMVectorSet(0.f, 1.f, 0.f, 0.f), XMConvertToRadians(90.0f));
    pTransform->SetState(STATE::POSITION, XMVectorSet(0.8f, 0.f, 0.f, 1.f));

    return S_OK;
}

void PlayerWeapon::PriorityUpdate(_float fTimeDelta)
{
}

void PlayerWeapon::Update(_float fTimeDelta)
{
    _matrix SocketMatrix = XMLoadFloat4x4(m_pSocketBoneMatrix);

    for (size_t i = 0; i < 3; i++)
        SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);

    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    XMStoreFloat4x4(&m_CombinedWorldMatrix,
        XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()) *
        SocketMatrix *
        XMLoadFloat4x4(m_pParentMatrix));
}

void PlayerWeapon::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::NONBLEND, this);
}

HRESULT PlayerWeapon::Render()
{
    if (FAILED(BindShaderResources()))
        return E_FAIL;

    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    _uint iNumMeshes = pModel->GetNumMeshes();

    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    for (size_t i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(pModel->BindShaderResource(i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
            return E_FAIL;

        pShader->Begin(0);

        pModel->Render(i);
    }
    return S_OK;
}

HRESULT PlayerWeapon::ReadyComponents()
{
    /* For.Com_Model */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_Model_ForkLift"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;


    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Prototype_Component_Shader_VtxMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;

}

HRESULT PlayerWeapon::BindShaderResources()
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


Object* PlayerWeapon::Create()
{
    PlayerWeapon* pInstance = new PlayerWeapon();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : PlayerWeapon");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* PlayerWeapon::Clone(void* pArg)
{
    PlayerWeapon* pInstance = new PlayerWeapon(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : PlayerWeapon");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void PlayerWeapon::Free()
{
    __super::Free();
}
