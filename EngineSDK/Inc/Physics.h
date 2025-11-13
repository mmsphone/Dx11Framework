#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL Physics final : public Component
{
public:
	typedef struct tagPhysicsDesc
	{
		_float worldSizeOffset = 1.f;
	}PHYSICS_DESC;
private:
	Physics();
	Physics(const Physics& Prototype);
	virtual ~Physics() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Update(_float fTimeDelta);

	_bool IsOnGround();
	void SetOnGround(const _bool& bOnGround);

	static Physics* Create();
	Component* Clone(void* pArg) override;
	virtual void Free() override;
private:
	PHYSICS_DESC	m_desc{};
	_bool m_isOnGround = false;
	_float m_curSpeed = 0.f;
};

NS_END