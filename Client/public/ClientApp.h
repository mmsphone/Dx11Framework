#pragma once

#include "Base.h"
#include "Client_Defines.h"

NS_BEGIN(Engine)
class EngineUtility;
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

	static ClientApp* Create();
	virtual void Free() override;

private:
	HRESULT ReadyPrototypeForStatic();
	HRESULT StartScene(SCENE eStartSceneId);
private:
	EngineUtility* m_pEngineUtility = { nullptr };
};

NS_END