#pragma once

#include "Base.h"

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
	virtual void Free() override;
private:
	class EngineUtility* m_pEngineUtility = { nullptr };
};

NS_END