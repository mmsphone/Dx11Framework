#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class FontManager final : public Base
{
private:
	FontManager();
	virtual ~FontManager() = default;

public:
	HRESULT AddFont(const _wstring& strFontTag, const _tchar* pFontFilePath);
	HRESULT DrawFont(const _wstring& strFontTag, const _wstring& strText, const _float2& vPosition, _fvector vColor);

	static FontManager* Create();
	virtual void Free() override;

private:
	class CustomFont* FindFont(const _wstring& strFontTag);
private:
	class EngineUtility* m_pEngineUtility = { nullptr };
	map<const _wstring, class CustomFont*> m_Fonts;
};

NS_END