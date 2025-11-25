#include "GameScene.h"

#include "EngineUtility.h"

#include "FreeCam.h"
#include "FixedCam.h"
#include "ChaseCam.h"

#include "Layer.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIButton.h"
#include "Player.h"
#include "part.h"
#include "Panel.h"
#include "Door.h"

GameScene::GameScene()
	: Scene{ }
{
}

HRESULT GameScene::Initialize()
{
	if (FAILED(ReadyLights()))
		return E_FAIL;

	if (FAILED(ReadyLayerCamera()))
		return E_FAIL;

	if (FAILED(ReadyUI()))
		return E_FAIL;

	if (FAILED(ReadyMouse()))
		return E_FAIL;

	PanelDoorLink();

	return S_OK;
}

void GameScene::Update(_float fTimeDelta)
{
	_float2 mousePos = m_pEngineUtility->GetMousePos();
	_float2 center{
			static_cast<_float>(g_iWinSizeX) * 0.5f,
			static_cast<_float>(g_iWinSizeY) * 0.5f
	};
	_float dx = mousePos.x - center.x;
	_float dy = mousePos.y - center.y;
	if (fabsf(dx) < 0.001f && fabsf(dy) < 0.001f)
		return;
	_float angle = atan2f(-dy, dx);
	angle += XM_PIDIV2;
	
	{
		UIImage* pMouse = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"Mouse"));
		if (pMouse != nullptr)
		{
			UI_DESC desc = pMouse->GetUIDesc();
			desc.x = mousePos.x;
			desc.y = mousePos.y;
			pMouse->ApplyUIDesc(desc);

			pMouse->SetRotationRad(angle);
		}
	}
	{
		UIImage* pMouseBulletCount = dynamic_cast<UIImage*>(m_pEngineUtility->FindUI(L"MouseBulletCount"));
		if (pMouseBulletCount != nullptr)
		{
			UI_DESC desc = pMouseBulletCount->GetUIDesc();
			desc.x = mousePos.x;
			desc.y = mousePos.y;
			pMouseBulletCount->ApplyUIDesc(desc);
			pMouseBulletCount->SetRotationRad(angle);
		}
	}
}

HRESULT GameScene::Render()
{
	return S_OK;
}

HRESULT GameScene::ReadyLights()
{
	m_pEngineUtility->ReadyLightsFromFile("../bin/data/GameScene1_Light.dat");

	return S_OK;
}

HRESULT GameScene::ReadyLayerCamera()
{
	//FreeCam::FREECAM_DESC			Desc{};
	//Desc.vEye = _float3(0.f, 0.f, 50.f);
	//Desc.vAt = _float3(0.f, 0.f, 0.f);
	//Desc.fFovy = XMConvertToRadians(60.0f);
	//Desc.fNear = 0.1f;
	//Desc.fFar = 500.f;
	//Desc.fSensor = 0.1f;
	//Desc.fSpeedPerSec = 40.f;
	//Desc.fRotationPerSec = XMConvertToRadians(120.0f);
	//
	//if (FAILED(m_pEngineUtility->AddObject(SCENE::STATIC, TEXT("FreeCam"), SCENE::GAMEPLAY, TEXT("Cam"), &Desc)))
	//	return E_FAIL;

	ChaseCam::CHASECAM_DESC			Desc{};
	Object* pPlayer =m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
	if (pPlayer == nullptr)
		return E_FAIL;
	Transform* pTransform = dynamic_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));
	if (pTransform == nullptr)
		return E_FAIL;
	Desc.pTarget = pPlayer;
	Desc.offset = _vector{ 0.f, 8.f, -4.f, 0.f };
	XMStoreFloat3(&Desc.vAt, pTransform->GetState(MATRIXROW_POSITION));
	XMStoreFloat3(&Desc.vEye, pTransform->GetState(MATRIXROW_POSITION) + Desc.offset);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 500.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.fRotationPerSec = XMConvertToRadians(120.0f);
	
	if (FAILED(m_pEngineUtility->AddObject(SCENE::STATIC, TEXT("ChaseCam"), SCENE::GAMEPLAY, TEXT("Cam"), &Desc)))
		return E_FAIL;
	
	return S_OK;
}

HRESULT GameScene::ReadyUI()
{
	D3D11_RECT rect{};
	rect.left = 1132;
	rect.top = 552;
	rect.right = 1262;
	rect.bottom = 685;
	list<Object*> UIs = m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects();
	for (auto& ui : UIs)
	{
		UI* u = dynamic_cast<UI*>(ui);
		if (u == nullptr)
			continue;
		string str = u->GetUIDesc().name;

		//minimap
		if (str == "minimapGameScene")
		{
			static_cast<UIImage*>(u)->SetScissor(rect);
		}
		//playerUI
		else if (str == "bulletCount")
		{
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			vector<Part*> pWeapons = static_cast<Player*>(pPlayer)->GetParts();
			Part* pWeapon = pWeapons.at(0);
			Info* pInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
			_int iCurBullet = *std::get_if<_int>(pInfo->GetInfo().GetPtr("CurBullet"));
			static_cast<UILabel*>(u)->SetText(to_wstring(iCurBullet));

		}
		else if (str == "grenadeCount")
		{
		}
		else if (str == "specialCount")
		{
		}
		else if (str == "ammo_front1")
		{
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			vector<Part*> pWeapons = static_cast<Player*>(pPlayer)->GetParts();
			Part* pWeapon = pWeapons.at(0);
			Info* pInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
			_int iCurAmmo = *std::get_if<_int>(pInfo->GetInfo().GetPtr("CurAmmo"));
			m_pEngineUtility->FindUI(L"ammo_front1")->SetVisible(iCurAmmo >= 1);
		}
		else if (str == "ammo_front2")
		{
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			vector<Part*> pWeapons = static_cast<Player*>(pPlayer)->GetParts();
			Part* pWeapon = pWeapons.at(0);
			Info* pInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
			_int iCurAmmo = *std::get_if<_int>(pInfo->GetInfo().GetPtr("CurAmmo"));
			m_pEngineUtility->FindUI(L"ammo_front2")->SetVisible(iCurAmmo >= 2);
		}
		else if (str == "ammo_front3")
		{
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			vector<Part*> pWeapons = static_cast<Player*>(pPlayer)->GetParts();
			Part* pWeapon = pWeapons.at(0);
			Info* pInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
			_int iCurAmmo = *std::get_if<_int>(pInfo->GetInfo().GetPtr("CurAmmo"));
			m_pEngineUtility->FindUI(L"ammo_front3")->SetVisible(iCurAmmo >= 3);
		}
		else if (str == "ammo_front4")
		{
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			vector<Part*> pWeapons = static_cast<Player*>(pPlayer)->GetParts();
			Part* pWeapon = pWeapons.at(0);
			Info* pInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
			_int iCurAmmo = *std::get_if<_int>(pInfo->GetInfo().GetPtr("CurAmmo"));
			m_pEngineUtility->FindUI(L"ammo_front4")->SetVisible(iCurAmmo >= 4);
		}
		else if (str == "ammo_front5")
		{
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			vector<Part*> pWeapons = static_cast<Player*>(pPlayer)->GetParts();
			Part* pWeapon = pWeapons.at(0);
			Info* pInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
			_int iCurAmmo = *std::get_if<_int>(pInfo->GetInfo().GetPtr("CurAmmo"));
			m_pEngineUtility->FindUI(L"ammo_front5")->SetVisible(iCurAmmo >= 5);
		}
		//hackingUI
		else if (str == "hacking_text_plate")
		{
			u->SetVisible(false);
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "hacking_image_plate")
		{
			u->SetVisible(false);
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "hacking_key_plate")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_image_lock")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_image_unlock")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_key_image_default")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_key_image_on")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_text_hacking")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_text_open")
		{
			u->SetVisible(false);
		}
		//hackingGame
		else if (str == "hacking_back")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
			u->SetVisible(false);
		}
		else if (str == "hacking_gameplate")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_done1")
		{
			static_cast<UIImage*>(u)->SetBrightness(0.1f);
			static_cast<UIImage*>(u)->SetGamma(1.f);
			u->SetVisible(false);
		}
		else if (str == "hacking_done2")
		{
			static_cast<UIImage*>(u)->SetBrightness(0.1f);
			static_cast<UIImage*>(u)->SetGamma(1.0f);
			u->SetVisible(false);
		}
		else if (str == "hacking_start1")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_start2")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_goal1")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_goal2")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_back_text")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_gameplate_text")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_progress")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			u->SetVisible(false);
		}
		else if (str == "hacking_close_default")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_close_on")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_close") { static_cast<UIButton*>(u)->SetEnable(false);	}
		else if (str == "hacking_minimize_default")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_minimize_on")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_progressbar")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_fastmarker")
		{
			u->SetVisible(false);
		}
		else if (str == "hacking_minimize") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_up_00") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_up_01") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_up_02") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_up_10") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_up_11") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_up_12") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_down_00") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_down_01") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_down_02") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_down_10") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_down_11") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking_down_12") { static_cast<UIButton*>(u)->SetEnable(false); }
	}

	return S_OK;
}

HRESULT GameScene::ReadyMouse()
{
	//기존 마우스 안보이게
	ShowCursor(false);

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	POINT lt{ rc.left, rc.top };
	POINT rb{ rc.right, rc.bottom };

	ClientToScreen(g_hWnd, &lt);
	ClientToScreen(g_hWnd, &rb);

	RECT clipRect;
	clipRect.left = lt.x;
	clipRect.top = lt.y;
	clipRect.right = rb.x;
	clipRect.bottom = rb.y;

	ClipCursor(&clipRect);

	//기존 마우스 위치 대체할 UIImage
	{
		UI_DESC desc{};
		desc.fSpeedPerSec = 0.f;
		desc.fRotationPerSec = 0.f;
		desc.type = UITYPE::UI_IMAGE;
		desc.name = "Mouse";
		desc.x = g_iWinSizeX >> 1;
		desc.y = g_iWinSizeY >> 1;
		desc.z = 0.f;
		desc.w = 256.f * 0.3f;
		desc.h = 256.f * 0.3f;
		desc.visible = true;
		desc.imagePath = L"../bin/Resources/Textures/Mouse/mouseDir.png";

		m_pEngineUtility->AddObject(SCENE::STATIC, L"UIImage", SCENE::GAMEPLAY, L"Mouse", &desc);

		UIImage* pUI = dynamic_cast<UIImage*>(m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Mouse", 0));
		m_pEngineUtility->AddUI(L"Mouse", pUI);
	}
	//마우스 위치 총알 개수
	{
		UI_DESC desc{};
		desc.fSpeedPerSec = 0.f;
		desc.fRotationPerSec = 0.f;
		desc.type = UITYPE::UI_IMAGE;
		desc.name = "Mouse";
		desc.x = g_iWinSizeX >> 1;
		desc.y = g_iWinSizeY >> 1;
		desc.z = 0.f;
		desc.w = 256.f * 0.3f;
		desc.h = 256.f * 0.3f;
		desc.visible = true;
		desc.imagePath = L"../bin/Resources/Textures/Mouse/mouseBulletCount.png";

		m_pEngineUtility->AddObject(SCENE::STATIC, L"UIImage", SCENE::GAMEPLAY, L"Mouse", &desc);

		UIImage* pUI = dynamic_cast<UIImage*>(m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Mouse", 1));
		m_pEngineUtility->AddUI(L"MouseBulletCount", pUI);
	}

	return S_OK;
}

HRESULT GameScene::PanelDoorLink()
{
	vector<Panel*> panels;
	vector<Door*> doors;

	for (_uint i = 0;; ++i)
	{
		Object* pObj = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Panel"), i);
		if (!pObj)
			break;

		if (auto pPanel = dynamic_cast<Panel*>(pObj))
			panels.push_back(pPanel);
	}

	for (_uint i = 0;; ++i)
	{
		if (i == 3)
			continue;
		Object* pObj = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Door"), i);
		if (!pObj)
			break;

		if (auto pDoor = dynamic_cast<Door*>(pObj))
			doors.push_back(pDoor);
	}
	const size_t count = min(panels.size(), doors.size());
	for (size_t i = 0; i < count; ++i)
	{
		panels[i]->SetDoor(doors[i]);
	}

	doors.at(0)->SetLock(true);
	doors.at(4)->SetLock(true);

	return S_OK;
}

GameScene* GameScene::Create()
{
	GameScene* pInstance = new GameScene();

	if (FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : GameScene");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void GameScene::Free()
{
	__super::Free();
}