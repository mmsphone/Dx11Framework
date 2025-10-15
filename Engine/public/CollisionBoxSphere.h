#pragma once

#include "CollisionBox.h"

NS_BEGIN(Engine)

class CollisionBoxSphere final : public CollisionBox
{
public:
	typedef struct tagCollisionBoxSphereDesc final : public CollisionBox::COLLISIONBOX_DESC
	{
		_float		fRadius;
	}COLLISIONSPHERE_DESC;
private:
	CollisionBoxSphere();
	virtual ~CollisionBoxSphere() = default;

public:
	HRESULT Initialize(CollisionBox::COLLISIONBOX_DESC* pInitialDesc);
	void Update(_fmatrix WorldMatrix);

	BoundingSphere* GetLocalBox() const;
	BoundingSphere* GetWorldBox() const;

private:
	BoundingSphere* m_pOriginalDesc = {}; //localBox
	BoundingSphere* m_pDesc = {}; // worldBox

public:
	static CollisionBoxSphere* Create(CollisionBox::COLLISIONBOX_DESC* pInitialDesc);
	virtual void Free() override;
};

NS_END