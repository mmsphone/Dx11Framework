#include "Collision.h"

#include "EngineUtility.h"

Collision::Collision()
    : Component{}
{
}

Collision::Collision(const Collision& Prototype)
    : Component{ Prototype }
    , m_CollisionType{ Prototype.m_CollisionType }
#ifdef _DEBUG
    , m_pBatch{ Prototype.m_pBatch }
    , m_pEffect{ Prototype.m_pEffect }
    , m_pInputLayout{ Prototype.m_pInputLayout }
#endif
{
#ifdef _DEBUG
    SafeAddRef(m_pInputLayout);
#endif
}

HRESULT Collision::InitializePrototype(COLLISIONTYPE eType)
{
    m_CollisionType = eType;

#ifdef _DEBUG
    m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pEngineUtility->GetContext());
    m_pEffect = new BasicEffect(m_pEngineUtility->GetDevice());
    m_pEffect->SetVertexColorEnabled(true);

    const void* pShaderByteCode = { nullptr };
    size_t      iShaderByteCodeLength = {};
    m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iShaderByteCodeLength);

    if (FAILED(m_pEngineUtility->GetDevice()->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount, pShaderByteCode, iShaderByteCodeLength, &m_pInputLayout)))
        return E_FAIL;
#endif

    return S_OK;
}

HRESULT Collision::Initialize(void* pArg)
{
    CollisionBox::COLLISIONBOX_DESC* pDesc = static_cast<CollisionBox::COLLISIONBOX_DESC*>(pArg);
    switch (m_CollisionType)
    {
    case COLLISIONTYPE::AABB:
        m_pCollisionBox = CollisionBoxAABB::Create(pDesc);
        break;
    case COLLISIONTYPE::OBB:
        m_pCollisionBox = CollisionBoxOBB::Create(pDesc);
        break;
    case COLLISIONTYPE::SPHERE:
        m_pCollisionBox = CollisionBoxSphere::Create(pDesc);
        break;
    }

    return S_OK;
}

void Collision::Update(_fmatrix WorldMatrix)
{
    m_pCollisionBox->Update(WorldMatrix);
}

_bool Collision::Intersect(Collision* pCollision)
{
    m_isIntersected = m_pCollisionBox->Intersect(pCollision);
    return m_isIntersected;
}

COLLISIONTYPE Collision::GetType() const
{
    return m_CollisionType;
}

void* Collision::GetWorldCollisionBox(COLLISIONTYPE eType)
{
    switch (eType)
    {
    case COLLISIONTYPE::AABB:
        return dynamic_cast<CollisionBoxAABB*>(m_pCollisionBox)->GetWorldBox();
    case COLLISIONTYPE::OBB:
        return dynamic_cast<CollisionBoxOBB*>(m_pCollisionBox)->GetWorldBox();
    case COLLISIONTYPE::SPHERE:
        return dynamic_cast<CollisionBoxSphere*>(m_pCollisionBox)->GetWorldBox();
    }
    return nullptr;
}

void* Collision::GetLocalCollisionBox(COLLISIONTYPE eType)
{
    switch (eType)
    {
    case COLLISIONTYPE::AABB:
        return dynamic_cast<CollisionBoxAABB*>(m_pCollisionBox)->GetLocalBox();
    case COLLISIONTYPE::OBB:
        return dynamic_cast<CollisionBoxOBB*>(m_pCollisionBox)->GetLocalBox();
    case COLLISIONTYPE::SPHERE:
        return dynamic_cast<CollisionBoxSphere*>(m_pCollisionBox)->GetLocalBox();
    }
    return nullptr;
}

#ifdef _DEBUG
HRESULT Collision::Render()
{
    m_pEffect->SetWorld(XMMatrixIdentity());
    m_pEffect->SetView(m_pEngineUtility->GetTransformMatrix(D3DTS::D3DTS_VIEW));
    m_pEffect->SetProjection(m_pEngineUtility->GetTransformMatrix(D3DTS::D3DTS_PROJECTION));

    m_pEffect->Apply(m_pEngineUtility->GetContext());

    m_pEngineUtility->GetContext()->IASetInputLayout(m_pInputLayout);

    m_pBatch->Begin();

    m_pCollisionBox->Render(m_pBatch, true == m_isIntersected ? XMVectorSet(1.f, 0.f, 0.f, 1.f) : XMVectorSet(0.f, 1.f, 0.f, 1.f));

    m_pBatch->End();

    return S_OK;
}
#endif

Collision* Collision::Create(COLLISIONTYPE eType)
{
    Collision* pInstance = new Collision();

    if (FAILED(pInstance->InitializePrototype(eType)))
    {
        MSG_BOX("Failed to Created : Collision");
        SafeRelease(pInstance);
    }
    return pInstance;
}

Component* Collision::Clone(void* pArg)
{
    Collision* pInstance = new Collision(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Collision");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void Collision::Free()
{
    __super::Free();

    SafeRelease(m_pCollisionBox);

#ifdef _DEBUG
    if (false == m_isCloned)
    {
        SafeDelete(m_pEffect);
        SafeDelete(m_pBatch);
    }
    SafeRelease(m_pInputLayout);
#endif
}
