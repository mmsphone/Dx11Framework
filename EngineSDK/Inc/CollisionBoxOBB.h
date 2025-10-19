#pragma once

#include "CollisionBox.h"

NS_BEGIN(Engine)

class CollisionBoxOBB final : public CollisionBox
{
public:
	typedef struct tagCollisionBoxOBBDesc final : public CollisionBox::COLLISIONBOX_DESC
	{
		_float3		vExtents;
		_float4		vOrientation;
	}COLLISIONOBB_DESC;
private:
	CollisionBoxOBB();
	virtual ~CollisionBoxOBB() = default;

public:
	HRESULT Initialize(CollisionBox::COLLISIONBOX_DESC* pInitialDesc);
	virtual void Update(_fmatrix WorldMatrix) override;
	virtual _bool Intersect(class Collision* pCollision)override;
#ifdef _DEBUG
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch, _fvector vColor) override;
#endif

	BoundingOrientedBox* GetLocalBox() const;
	BoundingOrientedBox* GetWorldBox() const;

private:
	BoundingOrientedBox* m_pOriginalDesc = {}; //localBox
	BoundingOrientedBox* m_pDesc = {}; // worldBox

public:
	static CollisionBoxOBB* Create(CollisionBox::COLLISIONBOX_DESC* pInitialDesc);
	virtual void Free() override;
};

NS_END