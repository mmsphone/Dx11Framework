#pragma once

#include "Client_Defines.h"
#include "Scene.h"

NS_BEGIN(Client)

class GameScene final : public Scene
{
private:
	GameScene();
	virtual ~GameScene() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	void EndScene(_bool bEnd);

	static GameScene* Create();
	virtual void Free() override;

private:
	HRESULT ReadyLights();
	HRESULT ReadyLayerCamera();
	HRESULT ReadyUI();
	HRESULT ReadyMinimap();
	HRESULT ReadyMouse();
	HRESULT PanelDoorLink();
	HRESULT ReadyQuest();
	HRESULT ReadyEnding();

private:
	_bool m_next = false;
};

NS_END

