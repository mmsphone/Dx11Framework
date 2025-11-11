#pragma once

#include "Object.h"

NS_BEGIN(Engine)

class ENGINE_DLL Projectile abstract : public Object
{
public:
	typedef struct tagProjectileDesc : public OBJECT_DESC{
		_vector moveDir = {};
		_float  lifeTime = 3.f;
		_float  damageAmount = 10.f;
		FACTION faction = FACTION_PLAYER;
		_float  hitRadius = 0.1f;
		_float  accTime = 0.f;
	}PROJECTILE_DESC;
protected:
	Projectile();
	Projectile(const Projectile& Prototype);
	virtual ~Projectile() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual Object* Clone(void* pArg) = 0;
	virtual void Free() override;
protected:
	PROJECTILE_DESC m_desc{};
};

NS_END