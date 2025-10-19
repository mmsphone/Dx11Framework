#pragma once

#include "Base.h"
#include "DebugDraw.h"

NS_BEGIN(Engine)

class CollisionBox abstract : public Base
{
public:
	typedef struct tagCollisionBoxDesc
	{
		_float3		vCenter;
	}COLLISIONBOX_DESC;
protected:
	CollisionBox();
	virtual ~CollisionBox() = default;
public:
	virtual _bool Intersect(class Collision* pCollision) = 0;
	virtual void Update(_fmatrix WorldMatrix) = 0;
#ifdef _DEBUG
	virtual HRESULT Render(PrimitiveBatch<VertexPositionColor>* pBatch, _fvector vColor = XMVectorSet(1.f, 1.f, 1.f, 1.f)) = 0;
#endif

	virtual void Free() override;
private:
	class EngineUtility* m_pEngineUtility = { nullptr };
};

NS_END