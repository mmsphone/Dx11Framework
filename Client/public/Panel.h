#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class Panel final : public ObjectTemplate
{
	Panel();
	Panel(const Panel& Prototype);
	virtual ~Panel() = default;

public:
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void SetDoor(class Door* pDoor);
	void OpenDoor();

	static Panel* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free() override;

private:
	HRESULT ReadyComponents();

	_float scaleOffset = 0.025f;

	_bool m_playerInRange = false;
	_bool m_worked = false;

	class Door* m_pTargetDoor = { nullptr };
};

NS_END