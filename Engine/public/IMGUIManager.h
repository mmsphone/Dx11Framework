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
	HRESULT Render();
	HRESULT Shutdown();

	HRESULT AddPanel(const string& PanelName, class Panel* pPanel);
	HRESULT SetPanelOpen(const string& PanelName, bool open);
	HRESULT RemovePanel(const string& PanelName);
	HRESULT ClearPanels();
	ImGuiContext* GetIMGUIContext() const;

	void DrawPanels();

	static IMGUIManager* Create();
	virtual void		Free() override;

private:
	class EngineUtility* m_pEngineUtility = { nullptr };
	map<string, Panel*> m_Panels;
	ImGuiContext* m_IMGUIContext = { nullptr };
};

NS_END