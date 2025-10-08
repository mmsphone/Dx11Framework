#pragma once

#include "Tool_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class EngineUtility;
NS_END

NS_BEGIN(Tool)

class MainApp final : public Base
{
private:
	MainApp();
	virtual ~MainApp() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

	static MainApp* Create();
	virtual void Free() override;

private:
	HRESULT ReadyPrototypeForStatic();
	HRESULT StartScene(SCENE eStartSceneId);
private:
	EngineUtility* m_pEngineUtility = { nullptr };
};

NS_END