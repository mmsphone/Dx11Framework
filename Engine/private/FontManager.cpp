#include "FontManager.h"
#include "EngineUtility.h"
#include "CustomFont.h"

FontManager::FontManager()
	:m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT FontManager::AddFont(const _wstring& strFontTag, const _tchar* pFontFilePath, _float originFontSize)
{
	if (FindFont(strFontTag) != nullptr) // 이미 있다면
		return E_FAIL;

	CustomFont* pFont = CustomFont::Create(pFontFilePath, originFontSize);
	if (pFont == nullptr)
		return E_FAIL;

	m_Fonts.emplace(strFontTag, pFont);

	return S_OK;
}

HRESULT FontManager::DrawFont(const _wstring& strFontTag, const _wstring& strText, const _float2& vPosition, _fvector vColor, _float drawFontSize)
{
	CustomFont* pFont = FindFont(strFontTag);
	if (pFont == nullptr) // 폰트가 없으면
		return E_FAIL;

	pFont->Draw(strText.c_str(), vPosition, vColor, drawFontSize);
	return S_OK;
}

FontManager* FontManager::Create()
{
	return new FontManager();
}


void FontManager::Free()
{
	__super::Free();
	SafeRelease(m_pEngineUtility);

	for (auto& Pair : m_Fonts)
		SafeRelease(Pair.second);
	m_Fonts.clear();
}

CustomFont* FontManager::FindFont(const _wstring& strFontTag)
{
	auto iter = m_Fonts.find(strFontTag);
	if (iter == m_Fonts.end())
		return nullptr;

	return iter->second;
}
