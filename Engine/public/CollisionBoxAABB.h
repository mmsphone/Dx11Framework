#pragma once

#include "CollisionBox.h"

NS_BEGIN(Engine)

class CollisionBoxAABB final : public CollisionBox
{
public:
	typedef struct tagCollisionBoxAABBDesc final : public CollisionBox::COLLISIONBOX_DESC
	{
		_float3		vSize;
	}COLLISIONAABB_DESC;
private:
	CollisionBoxAABB();
	virtual ~CollisionBoxAABB() = default;

public:
	HRESULT Initialize(CollisionBox::COLLISIONBOX_DESC* pInitialDesc);
	void Update(_fmatrix WorldMatrix);

	BoundingBox* GetLocalBox() const;
	BoundingBox* GetWorldBox() const;

private:
	BoundingBox* m_pOriginalDesc = {}; //localBox
	BoundingBox* m_pDesc = {}; // worldBox

public:
	static CollisionBoxAABB* Create(CollisionBox::COLLISIONBOX_DESC* pInitialDesc);
	virtual void Free() override;
};

NS_END