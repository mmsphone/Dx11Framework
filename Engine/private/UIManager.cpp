#include "UIManager.h"

UIManager::UIManager()
{
}

void UIManager::AddUI(_wstring tagUI, UI* pUI)
{
	if (pUI == nullptr)
		return;

	m_UIs[tagUI] = pUI;
}

void UIManager::RemoveUI(_wstring tagUI)
{
	m_UIs[tagUI] = nullptr;
}

UI* UIManager::FindUI(_wstring tagUI)
{
	auto it = m_UIs.find(tagUI);
	if (it == m_UIs.end())
		return nullptr;

	return it->second;
}

_uint UIManager::GetUICount()
{
	return m_UIs.size();
}

void UIManager::ClearUIs()
{
	m_UIs.clear();
}

UIManager* UIManager::Create()
{
	return new UIManager();
}

void UIManager::Free()
{
	__super::Free();
	ClearUIs();
}
