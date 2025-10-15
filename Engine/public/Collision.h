#pragma once

#include "Component.h"

#include "CollisionBoxAABB.h"
#include "CollisionBoxOBB.h"
#include "CollisionBoxSphere.h"

NS_BEGIN(Engine)

class ENGINE_DLL Collision final : public Component
{
private:
	Collision(COLLISIONTYPE eType);
	Collision(const Collision& Prototype);
	virtual ~Collision() = default;

public:
	virtual HRESULT Initialize(void* pArg) override;
	
	COLLISIONTYPE GetType() const;
	void* GetWorldCollisionBox(COLLISIONTYPE eType);	
	void* GetLocalCollisionBox(COLLISIONTYPE eType);

	static Collision* Create(COLLISIONTYPE eType);
	virtual Component* Clone(void* pArg);
	virtual void Free() override;

private:
	COLLISIONTYPE m_CollisionType = { COLLISIONTYPE::COLLISION_END };
	class CollisionBox* m_pCollisionBox = { nullptr };
};

NS_END