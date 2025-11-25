#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class UIManager final : public Base
{
	UIManager();
	virtual ~UIManager() = default;

public:
	void AddUI(_wstring tagUI, class UI* pUI);
	void RemoveUI(_wstring tagUI);
	class UI* FindUI(_wstring tagUI);
	void ClearUIs();

	static UIManager* Create();
	virtual void Free() override;
private:
	unordered_map<_wstring, class UI*> m_UIs;

};

NS_END