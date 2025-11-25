#pragma once

#include "Base.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class EngineUtility;
class UILabel;
NS_END

NS_BEGIN(Client)

class ClientApp final : public Base
{
private:
	ClientApp();
	virtual ~ClientApp() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

#ifdef _DEBUG
	void SetFPS(_uint fps);
#endif

	static ClientApp* Create();
	virtual void Free() override;

private:
	HRESULT ReadyPrototypeForStatic();
	HRESULT StartScene(SCENE eStartSceneId);
private:
	EngineUtility* m_pEngineUtility = { nullptr };

#ifdef _DEBUG
	class UILabel* m_pFPSLabel = { nullptr };
#endif
};

NS_END