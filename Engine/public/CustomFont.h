#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CustomFont final : public Base
{
private:
	CustomFont();
	virtual ~CustomFont() = default;

public:
	HRESULT Initialize(const _tchar* pFontFilePath, _float originFontSize);
	void Draw(const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fontSize = 0.f);

	static CustomFont* Create(const _tchar* pFontFilePath, _float originFontSize = 32.f);
	virtual void Free() override;

private:
	class EngineUtility* m_pEngineUtility = { nullptr };
	SpriteBatch* m_pBatch = { nullptr };
	SpriteFont* m_pFont = { nullptr };
	_float m_fOriginFontSize = 32.f;
};

NS_END