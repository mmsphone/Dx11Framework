#pragma once

#include "Component.h"
#include "CollisionBoxAABB.h"
#include "CollisionBoxOBB.h"
#include "CollisionBoxSphere.h"

NS_BEGIN(Engine)

class ENGINE_DLL Collision final : public Component
{
private:
	Collision();
	Collision(const Collision& Prototype);
	virtual ~Collision() = default;

public:
	virtual HRESULT InitializePrototype(COLLISIONTYPE eType);
	virtual HRESULT Initialize(void* pArg) override;
	void Update(_fmatrix WorldMatrix);
	_bool Intersect(Collision* pCollision);


	COLLISIONTYPE GetType() const;
	void* GetWorldCollisionBox(COLLISIONTYPE eType);	
	void* GetLocalCollisionBox(COLLISIONTYPE eType);

#ifdef _DEBUG
	HRESULT Render();
#endif

	static Collision* Create(COLLISIONTYPE eType);
	virtual Component* Clone(void* pArg);
	virtual void Free() override;

private:
	COLLISIONTYPE m_CollisionType = { COLLISIONTYPE::COLLISIONTYPE_END };
	class CollisionBox* m_pCollisionBox = { nullptr };
	_bool m_isIntersected = false;

#ifdef _DEBUG
	ID3D11InputLayout* m_pInputLayout = { nullptr };
	BasicEffect* m_pEffect = { nullptr };
	PrimitiveBatch<VertexPositionColor>* m_pBatch = { nullptr };
#endif
};

NS_END