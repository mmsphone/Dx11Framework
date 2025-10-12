#pragma once

#include "Tool_Defines.h"
#include "Scene.h"

NS_BEGIN(Tool)

class ModelScene : public Scene
{
	ModelScene();
	virtual ~ModelScene() = default;
public:
	HRESULT Initialize() override;
	void Update(_float fTimeDelta) override;
	HRESULT Render() override;

	static ModelScene* Create(SCENE eStartLevelID);
	virtual void Free() override;
};

NS_END