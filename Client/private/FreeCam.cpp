#include "FreeCam.h"

#include "EngineUtility.h"

FreeCam::FreeCam()
    : Camera{ }
{
}

FreeCam::FreeCam(const FreeCam& Prototype)
    : Camera{ Prototype }
{
}

HRESULT FreeCam::InitializePrototype()
{
    return S_OK;
}

HRESULT FreeCam::Initialize(void* pArg)
{
    FREECAM_DESC* pDesc = static_cast<FREECAM_DESC*>(pArg);
    m_fSensor = pDesc->fSensor;

    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void FreeCam::PriorityUpdate(_float fTimeDelta)
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    if (pTransform == nullptr)
        return;

    if (m_pEngineUtility->GetKeyState(DIK_W) & 0x80)
    {
        pTransform->GoForward(fTimeDelta);
    }

    if (GetKeyState('S') & 0x8000)
    {
        pTransform->GoBackward(fTimeDelta);
    }

    if (GetKeyState('A') & 0x8000)
    {
        pTransform->GoLeft(fTimeDelta);
    }

    if (GetKeyState('D') & 0x8000)
    {
        pTransform->GoRight(fTimeDelta);
    }

    _long         MouseMove = {};

    if (MouseMove = m_pEngineUtility->GetMouseMove(MOUSEMOVESTATE::X))
    {
        pTransform->RotateTimeDelta(XMVectorSet(0.f, 1.f, 0.f, 0.f), MouseMove * m_fSensor * fTimeDelta);
    }

    if (MouseMove = m_pEngineUtility->GetMouseMove(MOUSEMOVESTATE::Y))
    {
        pTransform->RotateTimeDelta(pTransform->GetState(STATE::RIGHT), MouseMove * m_fSensor * fTimeDelta);
    }

    UpdatePipeLine();
}

void FreeCam::Update(_float fTimeDelta)
{
}

void FreeCam::LateUpdate(_float fTimeDelta)
{
}

HRESULT FreeCam::Render()
{
    return S_OK;
}

FreeCam* FreeCam::Create()
{
    FreeCam* pInstance = new FreeCam();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : FreeCam");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* FreeCam::Clone(void* pArg)
{
    FreeCam* pInstance = new FreeCam(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : FreeCam");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void FreeCam::Free()
{
    __super::Free();
}
