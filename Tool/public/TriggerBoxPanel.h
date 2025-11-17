#pragma once

#include "Tool_Defines.h"
#include "Panel.h"

NS_BEGIN(Engine)
class TriggerBox;
NS_END

NS_BEGIN(Tool)

class TriggerBoxPanel final : public Panel
{
	TriggerBoxPanel(const std::string& panelName, bool open);
	virtual ~TriggerBoxPanel() = default;

public:
	virtual HRESULT Initialize();
	virtual void    OnRender() override;

	static TriggerBoxPanel* Create(const std::string& panelName, bool open = true);
	virtual void            Free() override;

private:
	void SyncFromSelectedTriggerBox();
	void ApplyToSelectedTriggerBox();

private:
	_int         m_SelectedTriggerIndex = -1;
	class TriggerBox* m_pSelectedTrigger = nullptr;

	_float3 m_EditCenter = _float3(0.f, 0.f, 0.f);
	_float3 m_EditExtents = _float3(1.f, 1.f, 1.f);
};

NS_END