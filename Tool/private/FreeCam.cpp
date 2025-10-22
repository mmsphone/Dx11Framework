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

    if (m_pEngineUtility->GetKeyState(DIK_W) & 0x80) pTransform->GoForward(fTimeDelta);
    if (m_pEngineUtility->GetKeyState(DIK_S) & 0x80) pTransform->GoBackward(fTimeDelta);
    if (m_pEngineUtility->GetKeyState(DIK_A) & 0x80) pTransform->GoLeft(fTimeDelta);
    if (m_pEngineUtility->GetKeyState(DIK_D) & 0x80) pTransform->GoRight(fTimeDelta);
    if (m_pEngineUtility->GetKeyState(DIK_SPACE) & 0x80) pTransform->GoUp(fTimeDelta);    
    if (m_pEngineUtility->GetKeyState(DIK_LCONTROL) & 0x80) pTransform->GoDown(fTimeDelta);

    static bool altPressedLastFrame = false;
    bool altPressed = m_pEngineUtility->GetKeyState(DIK_LALT) & 0x8000;

    _float2 winSize = m_pEngineUtility->GetWindowSize();
    _float2 center = { winSize.x / 2.f, winSize.y / 2.f };

    if (altPressed == false)
    {
        m_pEngineUtility->SetMousePos(center);
        m_pEngineUtility->SetMouseVisible(false);
    }
    else
        m_pEngineUtility->SetMouseVisible(true);

    if (altPressedLastFrame == false)
    {
        _long MouseMoveX = m_pEngineUtility->GetMouseMove(MOUSEMOVESTATE::X);
        if (MouseMoveX != 0)
            pTransform->RotateTimeDelta(XMVectorSet(0.f, 1.f, 0.f, 0.f), MouseMoveX * m_fSensor * fTimeDelta);

        _vector vLook = XMVector4Normalize(pTransform->GetState(LOOK));
        
        _float lookAngle = asinf(XMVectorGetY(vLook));
        constexpr _float limitAngle = XMConvertToRadians(89.f);

        _long MouseMoveY = m_pEngineUtility->GetMouseMove(MOUSEMOVESTATE::Y);
        if (MouseMoveY != 0)
        {
            _float moveAngle = MouseMoveY * m_fSensor * fTimeDelta;
            _float nextAngle = lookAngle - moveAngle;

            if(nextAngle < limitAngle && nextAngle > -limitAngle)
                pTransform->RotateTimeDelta(pTransform->GetState(RIGHT), moveAngle);
        }
    }
    altPressedLastFrame = altPressed;

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
