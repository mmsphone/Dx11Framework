#pragma once

#include "Client_Defines.h"
#include "ObjectTemplate.h"

NS_BEGIN(Client)

class GameScene1_Map final : public ObjectTemplate
{
private:
	GameScene1_Map();
	GameScene1_Map(const GameScene1_Map& Prototype);
	virtual ~GameScene1_Map() = default;

public:
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static GameScene1_Map* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free() override;

private:
	HRESULT ReadyComponents();
};

NS_END