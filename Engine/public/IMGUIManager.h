#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class IMGUIManager final : public Base
{
private:
	IMGUIManager();
	virtual ~IMGUIManager() = default;

public:
	HRESULT Initialize(HWND hWnd);
	HRESULT Begin();
	HRESULT Show();
	HRESULT Render();
	HRESULT Shutdown();

	static IMGUIManager* Create();
	virtual void		Free() override;

private:
	class EngineUtility* m_pEngineUtility;
};

NS_END