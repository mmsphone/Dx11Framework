#pragma once

#include "Client_Defines.h"
#include "Scene.h"

NS_BEGIN(Client)

class LogoScene final : public Scene
{
private:
	LogoScene();
	virtual ~LogoScene() = default;

public:
	virtual HRESULT Initialize() override;
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	HRESULT ReadyLights();
	HRESULT ReadyLayerCamera(const _tchar* pLayerTag);
	HRESULT ReadyLayerUI();

	static LogoScene* Create();
	virtual void Free() override;

private:
	HRESULT LoadMapObjects(const std::vector<MAP_OBJECTDATA>& mapData);
};

NS_END