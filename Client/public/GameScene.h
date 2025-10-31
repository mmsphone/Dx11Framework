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

	HRESULT ReadyLights();
	HRESULT ReadyLayerCamera(const _tchar* pLayerTag);

	static GameScene* Create();
	virtual void Free() override;

private:
	HRESULT LoadMapObjects(const std::vector<MAP_OBJECTDATA>& mapData);
};

NS_END

