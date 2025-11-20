#include "CustomFont.h"
#include "EngineUtility.h"

CustomFont::CustomFont()
	:m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT CustomFont::Initialize(const _tchar* pFontFilePath, _float originFontSize)
{
	m_pBatch = new SpriteBatch(m_pEngineUtility->GetContext());
	m_pFont = new SpriteFont(m_pEngineUtility->GetDevice(), pFontFilePath);
	m_fOriginFontSize = originFontSize;

	m_pFont->SetDefaultCharacter(L'?');

	return S_OK;
}

void CustomFont::Draw(const _tchar* pText, const _float2& vPosition, _fvector vColor, _float fontSize)
{
	_float scale = 1.f;
	if (fontSize > 0.f)
		scale = fontSize / m_fOriginFontSize;
	_float rot = 0.f;

	m_pBatch->Begin();
	m_pFont->DrawString(m_pBatch,pText, vPosition, vColor, rot, _float2(0.f,0.f), scale);
	m_pBatch->End();
}

CustomFont* CustomFont::Create(const _tchar* pFontFilePath, _float originFontSize)
{
	CustomFont* pInstance = new CustomFont();

	if (FAILED(pInstance->Initialize(pFontFilePath, originFontSize)))
	{
		MSG_BOX("Failed to Created : CustomFont");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void CustomFont::Free()
{
	__super::Free();
	SafeDelete(m_pFont);
	SafeDelete(m_pBatch);
	SafeRelease(m_pEngineUtility);
}
