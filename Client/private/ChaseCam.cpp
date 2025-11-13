#include "ChaseCam.h"
#include "EngineUtility.h"

NS_BEGIN(Client)

ChaseCam::ChaseCam()
    : Camera{ }
{
}

ChaseCam::ChaseCam(const ChaseCam& Prototype)
    : Camera{ Prototype }
{
}

HRESULT ChaseCam::InitializePrototype()
{
    return S_OK;
}

HRESULT ChaseCam::Initialize(void* pArg)
{
    CHASECAM_DESC* pDesc = static_cast<CHASECAM_DESC*>(pArg);

    m_pTarget = pDesc->pTarget;
    m_vOffset = pDesc->offset;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void ChaseCam::PriorityUpdate(_float fTimeDelta)
{
    UpdatePipeLine();
}

void ChaseCam::Update(_float fTimeDelta)
{
}

void ChaseCam::LateUpdate(_float fTimeDelta)
{
    if (m_pTarget == nullptr)
        return;

    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (pTransform == nullptr)
        return;
    Transform* targetTransform = dynamic_cast<Transform*>(m_pTarget->FindComponent(TEXT("Transform")));
    if (targetTransform == nullptr)
        return;

    pTransform->SetState(MATRIXROW_POSITION, targetTransform->GetState(MATRIXROW_POSITION) + m_vOffset);
}

HRESULT ChaseCam::Render()
{
    return S_OK;
}

ChaseCam* ChaseCam::Create()
{
    ChaseCam* pInstance = new ChaseCam();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : ChaseCam");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Object* ChaseCam::Clone(void* pArg)
{
    ChaseCam* pInstance = new ChaseCam(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : ChaseCam");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void ChaseCam::Free()
{
    __super::Free();

    m_pTarget = nullptr;
}

NS_END
