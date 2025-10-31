#include "FixedCam.h"
#include "EngineUtility.h"

NS_BEGIN(Client)

FixedCam::FixedCam()
    : Camera{ }
{
}

FixedCam::FixedCam(const FixedCam& Prototype)
    : Camera{ Prototype }
{
}

HRESULT FixedCam::InitializePrototype()
{
    return S_OK;
}

HRESULT FixedCam::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    return S_OK;
}

void FixedCam::PriorityUpdate(_float fTimeDelta)
{
    UpdatePipeLine();
}

void FixedCam::Update(_float fTimeDelta)
{
}

void FixedCam::LateUpdate(_float fTimeDelta)
{
}

HRESULT FixedCam::Render()
{
    return S_OK;
}

FixedCam* FixedCam::Create()
{
    FixedCam* pInstance = new FixedCam();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : FixedCam");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Object* FixedCam::Clone(void* pArg)
{
    FixedCam* pInstance = new FixedCam(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : FixedCam");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void FixedCam::Free()
{
    __super::Free();
}

NS_END
