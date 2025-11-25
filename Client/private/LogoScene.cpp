#include "LogoScene.h"

#include "EngineUtility.h"
#include "LoadingScene.h"

#include "FixedCam.h"
#include "Layer.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIButton.h"

LogoScene::LogoScene()
	: Scene{ }
{
}

HRESULT LogoScene::Initialize()
{
	if (FAILED(ReadyLights()))
		return E_FAIL;

	if (FAILED(ReadyLayerCamera()))
		return E_FAIL;

	if (FAILED(ReadyUI()))
		return E_FAIL;

	m_next = false;

	return S_OK;
}

void LogoScene::Update(_float fTimeDelta)
{
	if (m_next == true)
	{
		if (FAILED(m_pEngineUtility->ChangeScene(SCENE::LOADING, LoadingScene::Create(SCENE::GAMEPLAY))))
			return;
	}
}

HRESULT LogoScene::Render()
{
	return S_OK;
}

HRESULT LogoScene::ReadyLights()
{
	return S_OK;
}

HRESULT LogoScene::ReadyLayerCamera()
{
	return S_OK;
}

HRESULT LogoScene::ReadyUI()
{
	list<Object*> UIs = m_pEngineUtility->FindLayer(SCENE::LOGO, L"UI")->GetAllObjects();
	for (auto& ui : UIs)
	{
		UI* u = dynamic_cast<UI*>(ui);
		if (u == nullptr)
			continue;
		string str = u->GetUIDesc().name;
		if (str == "setting_on")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "swarm_on")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "message_on")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "exit_on")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "room_on")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "solo_on")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "top1_on")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "top2_on")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "top3_on")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "top4_on")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "internet_default")
		{
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "top1_default")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "top2_default")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "top3_default")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "top4_default")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "topplate")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "mapplate")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "nameplate")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "lobbyplate")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "room_default")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "solo_default")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "setting")
		{
			static_cast<UIButton*>(u)->SetDefaultImage(L"setting_default");
			static_cast<UIButton*>(u)->SetOnImage(L"setting_on");
		}
		else if (str == "swarm")
		{
			static_cast<UIButton*>(u)->SetDefaultImage(L"swarm_default");
			static_cast<UIButton*>(u)->SetOnImage(L"swarm_on");
		}
		else if (str == "exit")
		{
			static_cast<UIButton*>(u)->SetDefaultImage(L"exit_default");
			static_cast<UIButton*>(u)->SetOnImage(L"exit_on");
			static_cast<UIButton*>(u)->AddButtonFunction([]() { 
				DestroyWindow(g_hWnd); 
			});
		}
		else if (str == "message")
		{
			static_cast<UIButton*>(u)->SetDefaultImage(L"message_default");
			static_cast<UIButton*>(u)->SetOnImage(L"message_on");
		}
		else if (str == "room")
		{
			static_cast<UIButton*>(u)->SetDefaultImage(L"room_default");
			static_cast<UIButton*>(u)->SetOnImage(L"room_on");
			static_cast<UIButton*>(u)->SetText(L"room_text");
		}
		else if (str == "solo")
		{
			static_cast<UIButton*>(u)->SetDefaultImage(L"solo_default");
			static_cast<UIButton*>(u)->SetOnImage(L"solo_on");
			static_cast<UIButton*>(u)->SetText(L"solo_text");
			static_cast<UIButton*>(u)->AddButtonFunction([this]() {
				m_next = true;
			});
		}
		else if (str == "top1")
		{
			static_cast<UIButton*>(u)->SetDefaultImage(L"top1_default");
			static_cast<UIButton*>(u)->SetOnImage(L"top1_on");
			static_cast<UIButton*>(u)->SetText(L"top1_text");
		}
		else if (str == "top2")
		{
			static_cast<UIButton*>(u)->SetDefaultImage(L"top2_default");
			static_cast<UIButton*>(u)->SetOnImage(L"top2_on");
			static_cast<UIButton*>(u)->SetText(L"top2_text");
		}
		else if (str == "top3")
		{
			static_cast<UIButton*>(u)->SetDefaultImage(L"top3_default");
			static_cast<UIButton*>(u)->SetOnImage(L"top3_on");
			static_cast<UIButton*>(u)->SetText(L"top3_text");
		}
		else if (str == "top4")
		{
			static_cast<UIButton*>(u)->SetDefaultImage(L"top4_default");
			static_cast<UIButton*>(u)->SetOnImage(L"top4_on");
			static_cast<UIButton*>(u)->SetText(L"top4_text");
		}
	}

	return S_OK;
}

LogoScene* LogoScene::Create()
{
	LogoScene* pInstance = new LogoScene();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : LogoScene");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void LogoScene::Free()
{
	__super::Free();
}
