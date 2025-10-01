#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CustomFont final : public Base
{
private:
	CustomFont();
	virtual ~CustomFont() = default;

public:
	HRESULT Initialize(const _tchar* pFontFilePath);
	void Draw(const _tchar* pText, const _float2& vPosition, _fvector vColor);

	static CustomFont* Create(const _tchar* pFontFilePath);
	virtual void Free() override;

private:
	class EngineUtility* m_pEngineUtility = { nullptr };
	SpriteBatch* m_pBatch = { nullptr };
	SpriteFont* m_pFont = { nullptr };
};

NS_END