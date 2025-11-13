#include "Physics.h"

#include "object.h"

Physics::Physics()
    :Component{}
{
}

Physics::Physics(const Physics& Prototype)
    :Component{ Prototype }
    ,m_isOnGround{ false }
    ,m_curSpeed{ 0.f }
{
}

HRESULT Physics::InitializePrototype()
{
    if (FAILED(__super::InitializePrototype()))
        return E_FAIL;

    return S_OK;
}

HRESULT Physics::Initialize(void* pArg)
{
    if (FAILED(__super::Initialize(pArg)))
        return E_FAIL;

    if (pArg != nullptr)
    {
        m_desc.worldSizeOffset = static_cast<PHYSICS_DESC*>(pArg)->worldSizeOffset;
    }

    return S_OK;
}

void Physics::Update(_float fTimeDelta)
{
    if (!m_isOnGround)
    {
        Transform* pTransform = static_cast<Transform*>(m_pOwner->FindComponent(TEXT("Transform")));
        _vector vPrePos = pTransform->GetState(MATRIXROW_POSITION);

        static const _vector vGravityDir = _vector{ 0.f, -1.f, 0.f, 0.f };
        m_curSpeed += 9.8f * fTimeDelta * m_desc.worldSizeOffset;
        _vector vPos = vPrePos + vGravityDir * m_curSpeed;

        pTransform->SetState(MATRIXROW_POSITION, vPos);
    }
    else
        m_curSpeed = 0.f;
}

_bool Physics::IsOnGround()
{
    return m_isOnGround;
}

void Physics::SetOnGround(const _bool& bOnGround)
{
    m_isOnGround = bOnGround;
}

Physics* Physics::Create()
{
    Physics* pInstance = new Physics();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Physics");
        SafeRelease(pInstance);
    }

    return pInstance;
}

Component* Physics::Clone(void* pArg)
{
    Physics* pInstance = new Physics(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Physics");
        SafeRelease(pInstance);
    }

    return pInstance;
}

void Physics::Free()
{
    __super::Free();
}
