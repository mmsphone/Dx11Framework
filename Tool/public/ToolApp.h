#pragma once

#include "Tool_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class EngineUtility;
NS_END

NS_BEGIN(Tool)

class ToolApp final : public Base
{
private:
	ToolApp();
	virtual ~ToolApp() = default;

public:
	HRESULT Initialize();
	void Update(_float fTimeDelta);
	HRESULT Render();

	static ToolApp* Create();
	virtual void Free() override;

private:
	HRESULT ReadyPrototypeForStatic();
private:
	EngineUtility* m_pEngineUtility = { nullptr };
};

NS_END