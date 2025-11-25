#include "Door.h"

#include "EngineUtility.h"

Door::Door() 
    : ObjectTemplate{ }
{
}

Door::Door(const Door& Prototype)
    : ObjectTemplate{ Prototype }
{
}

HRESULT Door::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    m_isOpening = false;
    m_isOpen = false;
    m_openT = 0.f;
    m_posInitialized = false;
    m_isLock = false;

    return S_OK;
}

void Door::Update(_float fTimeDelta)
{
    __super::Update(fTimeDelta);

    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    Collision* pCollision = static_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Update(XMLoadFloat4x4(pTransform->GetWorldMatrixPtr()));

    if (m_isOpening && !m_isOpen && pTransform)
    {
        m_openT += fTimeDelta;
        _float t = m_openT / m_openDuration;
        if (t >= 1.f)
        {
            t = 1.f;
            m_isOpening = false;
            m_isOpen = true;
        }

        _vector closed = XMLoadFloat3(&m_closedPos);
        _vector opened = XMLoadFloat3(&m_openPos);
        _vector pos = XMVectorLerp(closed, opened, t);

        pTransform->SetState(MATRIXROW_POSITION, pos);
    }
}

void Door::LateUpdate(_float fTimeDelta)
{
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (m_pEngineUtility->IsIn_Frustum_WorldSpace(pTransform->GetState(MATRIXROW_POSITION), scaleOffset))
    {
        m_pEngineUtility->JoinRenderGroup(RENDERGROUP::RENDER_NONBLEND, this);
    }

    __super::LateUpdate(fTimeDelta);
}

HRESULT Door::Render()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    Shader* pShader = dynamic_cast<Shader*>(FindComponent(TEXT("Shader")));
    Model* pModel = dynamic_cast<Model*>(FindComponent(TEXT("Model")));
    if (!pTransform || !pShader || !pModel)
        return E_FAIL;

    if (FAILED(pTransform->BindRenderTargetShaderResource(pShader, "g_WorldMatrix")))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ViewMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_VIEW))))
        return E_FAIL;
    if (FAILED(pShader->BindMatrix("g_ProjMatrix", m_pEngineUtility->GetTransformFloat4x4Ptr(D3DTS::D3DTS_PROJECTION))))
        return E_FAIL;
    if (FAILED(pShader->BindRawValue("g_CamFarDistance", m_pEngineUtility->GetPipelineFarDistance(), sizeof(_float))))
        return E_FAIL;

    _uint       iNumMeshes = pModel->GetNumMeshes();
    for (_uint i = 0; i < iNumMeshes; i++)
    {
        if (FAILED(pModel->BindRenderTargetShaderResource(i, pShader, "g_DiffuseTexture", TextureType::Diffuse, 0)))
            continue;

        pShader->Begin(0);
        pModel->Render(i);
    }

#ifdef _DEBUG
    Collision* pCollision = dynamic_cast<Collision*>(FindComponent(TEXT("Collision")));
    pCollision->Render();
#endif

    return S_OK;
}

void Door::SetLock(_bool bLock)
{
    m_isLock = bLock;
}

_bool Door::IsLocked()
{
    return m_isLock;
}

void Door::Open()
{
    if (m_isLock || m_isOpen || m_isOpening)
        return;
    Transform* pTransform = static_cast<Transform*>(FindComponent(TEXT("Transform")));

    if (!m_posInitialized)
    {
        XMStoreFloat3(&m_closedPos, pTransform->GetState(MATRIXROW_POSITION));
        
        _vector right = pTransform->GetState(MATRIXROW_RIGHT);
        right = XMVector3Normalize(right);
        const float openDistance = 2.4f;
        
        _vector closed = XMLoadFloat3(&m_closedPos);
        _vector opened = closed - right * openDistance;
        XMStoreFloat3(&m_openPos, opened);

        m_posInitialized = true;
    }

    m_isOpening = true;
    m_openT = 0.f;
}

Door* Door::Create()
{
    Door* pInstance = new Door();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Door");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Object* Door::Clone(void* pArg)
{
    Door* pInstance = new Door(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Door");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Door::Free()
{
    __super::Free();
}

HRESULT Door::ReadyComponents()
{
    /* For.Com_Shader */
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("Shader_VtxMesh"), TEXT("Shader"), nullptr, nullptr)))
        return E_FAIL;

    /* For.Com_Model */
    if (FAILED(AddComponent(SCENE::GAMEPLAY, TEXT("Model_Door"), TEXT("Model"), nullptr, nullptr)))
        return E_FAIL;

    CollisionBoxOBB::COLLISIONOBB_DESC     OBBDesc{};
    OBBDesc.vOrientation = _float4(0.f, 0.f, 0.f, 1.f);
    XMStoreFloat3(&OBBDesc.vExtents, _vector{ 1.2f, 1.f, 0.30f } / scaleOffset);
    OBBDesc.vCenter = _float3(0.f, OBBDesc.vExtents.y * 0.7f, 0.f);
    if (FAILED(AddComponent(SCENE::STATIC, TEXT("CollisionOBB"), TEXT("Collision"), nullptr, &OBBDesc)))
        return E_FAIL;

    return S_OK;
}