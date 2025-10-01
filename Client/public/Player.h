#pragma once

#include "Client_Defines.h"
#include "Container.h"

NS_BEGIN(Client)

class Player final : public Container
{
public:
	enum PART : int { BODY, WEAPON, EFFECT, PART_END };
	enum STATE {
		IDLE = 0x00000001,		/* 0001 */
		RUN = 0x00000002,		/* 0010 */
		ATTACK = 0x00000004,	/* 0100 */
		END
	};

private:
	Player();
	Player(const Player& Prototype);
	virtual ~Player() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static Object* Create();/* 원형생성 */
	virtual Object* Clone(void* pArg) override;/* 사본생성 */
	virtual void Free();

private:
	HRESULT ReadyComponents();
	HRESULT ReadyParts();

private:
	_uint				m_iState = {};
};

NS_END