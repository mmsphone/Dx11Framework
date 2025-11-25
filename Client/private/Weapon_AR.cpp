#include "Weapon_AR.h"

#include "EngineUtility.h"

#include "Player.h"
#include "UIImage.h"
#include "UILabel.h"

Weapon_AR::Weapon_AR()
	:PartTemplate{}
{
}

Weapon_AR::Weapon_AR(const Weapon_AR& Prototype)
	:PartTemplate{Prototype}
{
}

HRESULT Weapon_AR::Initialize(void* pArg)
{
    WEAPON_AR_DESC* pDesc = static_cast<WEAPON_AR_DESC*>(pArg);
    m_pSocketBoneMatrix = pDesc->pSocketBoneMatrix;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    pTransform->SetScale(1.f, 1.f, 1.f);
    pTransform->SetState(MATRIXROW::MATRIXROW_POSITION, XMVectorSet(15.f, 0.f, -7.f, 1.f));

    Model* pModel = static_cast<Model*>(FindComponent(TEXT("Model")));
    pModel->SetPreTransformRotation(_float3{90.f, 0.f ,0.f});
    pModel->SetAnimation(0, false, 0.f, false);
    pModel->PlayAnimation(0.0f);

    if (FAILED(SetUpInfo()))
        return E_FAIL;

    return S_OK;
}

void Weapon_AR::Update(_float fTimeDelta)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));

    _matrix SocketMatrix = XMLoadFloat4x4(m_pSocketBoneMatrix);
    for (size_t i = 0; i < 3; i++)
        SocketMatrix.r[i] = XMVector3Normalize(SocketMatrix.r[i]);

    XMStoreFloat4x4(&m_CombinedWorldMatrix, 
        XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()) * SocketMatrix * XMLoadFloat4x4(m_pParentMatrix));
}

void Weapon_AR::LateUpdate(_float fTimeDelta)
{
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_NONBLEND, this);
    m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_SHADOWLIGHT, this);

    __super::LateUpdate(fTimeDelta);
}

HRESULT Weapon_AR::Render()
{
    Shader* pShader = static_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = static_cast<Model*>(FindComponent(TEXT("Model")));

    if (FAILED(pShader->BindMatrix("g_WorldMatrix", &m_CombinedWorldMatrix))) return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_VIEW)))) return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS_PROJECTION)))) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_CamFarDistance", m_pEngineUtility->GetPipelineFarDistance(), sizeof(_float)))) return E_FAIL;

    _uint       iNumMeshes = pModel->GetNumMeshes();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(pModel->BindRenderTargetShaderResource(i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
            continue;

        pModel->BindBoneMatrices(i, pShader, "g_BoneMatrices");
        pShader->Begin(0);
        pModel->Render(i);
    }
    return S_OK;
}

HRESULT Weapon_AR::RenderShadow(_uint iIndex)
{
    Shader* pShader = static_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = static_cast<Model*>(FindComponent(TEXT("Model")));

    if (FAILED(pShader->BindMatrix("g_WorldMatrix", &m_CombinedWorldMatrix))) return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW, iIndex)))) return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetActiveShadowLightTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION, iIndex)))) return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_ShadowLightFarDistance", m_pEngineUtility->GetActiveShadowLightFarDistancePtr(iIndex), sizeof(_float)))) return E_FAIL;

    _uint       iNumMeshes = pModel->GetNumMeshes();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        pModel->BindBoneMatrices(i, pShader, "g_BoneMatrices");
        pShader->Begin(iIndex + 1);
        pModel->Render(i);
    }

    return S_OK;
}

Weapon_AR* Weapon_AR::Create()
{
    Weapon_AR* pInstance = new Weapon_AR();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Weapon_AR");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Object* Weapon_AR::Clone(void* pArg)
{
    Weapon_AR* pInstance = new Weapon_AR(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Weapon_AR");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void Weapon_AR::Free()
{
    __super::Free();

    m_pSocketBoneMatrix = nullptr;
}

HRESULT Weapon_AR::ReadyComponents()
{
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Weapon_AR"), TEXT("Model"), nullptr, nullptr))) 
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxAnimMesh"), TEXT("Shader"), nullptr, nullptr))) 
        return E_FAIL;
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Info"), TEXT("Info"), nullptr, nullptr)))
        return E_FAIL;

    return S_OK;
}

HRESULT Weapon_AR::SetUpInfo()
{
    Info* pInfo = dynamic_cast<Info*>(FindComponent(TEXT("Info")));
    if (!pInfo)
        return E_FAIL;

    INFO_DESC desc;
    desc.SetData("MaxBullet", _int{ 30 });
    desc.SetData("CurBullet", _int{ 30 });
    desc.SetData("Damage", _float{ 10.f });
    desc.SetData("CurAmmo", _int{ 5 });
    desc.SetData("Faction", FACTION_PLAYER);
    pInfo->BindInfoDesc(desc);

    return S_OK;
}