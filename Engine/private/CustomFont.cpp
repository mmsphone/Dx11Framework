#include "CustomFont.h"
#include "EngineUtility.h"

CustomFont::CustomFont()
	:m_pEngineUtility{ EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT CustomFont::Initialize(const _tchar* pFontFilePath)
{
	m_pBatch = new SpriteBatch(m_pEngineUtility->GetContext());
	m_pFont = new SpriteFont(m_pEngineUtility->GetDevice(), pFontFilePath);

	return S_OK;
}

void CustomFont::Draw(const _tchar* pText, const _float2& vPosition, _fvector vColor)
{
	m_pBatch->Begin();
	m_pFont->DrawString(m_pBatch,pText, vPosition, vColor);
	m_pBatch->End();
}

CustomFont* CustomFont::Create(const _tchar* pFontFilePath)
{
	CustomFont* pInstance = new CustomFont();

	if (FAILED(pInstance->Initialize(pFontFilePath)))
	{
		MSG_BOX("Failed to Created : CustomFont");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void CustomFont::Free()
{
	__super::Free();

	m_pEngineUtility->DestroyInstance();
	SafeDelete(m_pFont);
	SafeDelete(m_pBatch);
}
