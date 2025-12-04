#include "GameScene.h"

#include "EngineUtility.h"
#include "LoadingScene.h"

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
#include "QuestUI.h"
#include "minimapUI.h"
#include "endingUI.h"
#include "TriggerBox.h"

GameScene::GameScene()
	: Scene{ }
{
}

HRESULT GameScene::Initialize()
{
	m_pEngineUtility->PlaySound2D("BGM_game", 0.5f);

	if (FAILED(ReadyLights()))
		return E_FAIL;

	if (FAILED(ReadyLayerCamera()))
		return E_FAIL;

	if (FAILED(ReadyUI()))
		return E_FAIL;

	if (FAILED(ReadyMinimap()))
		return E_FAIL;

	if (FAILED(ReadyMouse()))
		return E_FAIL;

	if (FAILED(PanelDoorLink()))
		return E_FAIL;

	if (FAILED(ReadyQuest()))
		return E_FAIL;

	if (FAILED(ReadyEnding()))
		return E_FAIL;

	return S_OK;
}

void GameScene::Update(_float fTimeDelta)
{
	if (m_next == true)
	{
		if (FAILED(m_pEngineUtility->ChangeScene(SCENE::LOADING, LoadingScene::Create(SCENE::LOGO))))
			return;
		return;
	}

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

	if (m_pEngineUtility->IsKeyPressed(DIK_9))
	{
		Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0);
		static_cast<Transform*>(pPlayer->FindComponent(L"Transform"))->SetState(MATRIXROW_POSITION, _vector{ -122.f, 16.5f, 12.f, 1.f });
	}
	if (m_pEngineUtility->IsKeyPressed(DIK_8))
	{
		Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0);
		static_cast<Transform*>(pPlayer->FindComponent(L"Transform"))->SetState(MATRIXROW_POSITION, _vector{ -150.f, 16.5f, -15.f, 1.f });
	}
	if (m_pEngineUtility->IsKeyPressed(DIK_7))
	{
		Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0);
		static_cast<Transform*>(pPlayer->FindComponent(L"Transform"))->SetState(MATRIXROW_POSITION, _vector{ -140.f, 17.2f, -63.f, 1.f });
	}
	if (m_pEngineUtility->IsKeyPressed(DIK_1))
	{
		Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0);
		static_cast<Info*>(pPlayer->FindComponent(L"Info"))->GetInfo().SetData("CurHP", _float{ 10.f });
	}
}

HRESULT GameScene::Render()
{
	return S_OK;
}

void GameScene::EndScene(_bool bEnd)
{
	m_next = bEnd;
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
	m_pEngineUtility->StopSound("BGM_game");

	m_pEngineUtility->ClearLights();
	m_pEngineUtility->ClearShadowLights();
	m_pEngineUtility->ClearSpawners();
	m_pEngineUtility->ClearTriggerBoxes();
	m_pEngineUtility->ClearCells();
	while (ShowCursor(true) < 0) {}

	__super::Free();
}

HRESULT GameScene::ReadyLights()
{
	m_pEngineUtility->ReadyLightsFromFile("../bin/data/GameScene1_Light.dat");

	return S_OK;
}

HRESULT GameScene::ReadyLayerCamera()
{
	ChaseCam::CHASECAM_DESC			Desc{};
	Object* pPlayer =m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
	if (pPlayer == nullptr)
		return E_FAIL;
	Transform* pTransform = dynamic_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));
	if (pTransform == nullptr)
		return E_FAIL;
	Desc.pTarget = pPlayer;
	Desc.offset = _vector{ 0.f, 8.2f, -3.3f, 0.f };
	XMStoreFloat3(&Desc.vAt, pTransform->GetState(MATRIXROW_POSITION));
	XMStoreFloat3(&Desc.vEye, pTransform->GetState(MATRIXROW_POSITION) + Desc.offset);
	Desc.fFovy = XMConvertToRadians(60.0f);
	Desc.fNear = 0.1f;
	Desc.fFar = 100.f;
	Desc.fSpeedPerSec = 0.f;
	Desc.fRotationPerSec = XMConvertToRadians(120.0f);
	
	if (FAILED(m_pEngineUtility->AddObject(SCENE::STATIC, TEXT("ChaseCam"), SCENE::GAMEPLAY, TEXT("Cam"), &Desc)))
		return E_FAIL;

	Object* pObj = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Cam"), 0);
	Camera* pCam = dynamic_cast<Camera*>(pObj);
	if (pCam != nullptr)
		m_pEngineUtility->SetMainCamera(pCam);

	return S_OK;
}

HRESULT GameScene::ReadyUI()
{
	list<Object*> UIs = m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects();
	for (auto& ui : UIs)
	{
		UI* u = dynamic_cast<UI*>(ui);
		if (u == nullptr)
			continue;
		string str = u->GetUIDesc().name;

		//playerUI
		if (str == "playerStatBack") { u->SetVisible(true); }
		else if (str == "playerFace") { u->SetVisible(true); }
		else if (str == "playerHpBack") { u->SetVisible(true); }
		else if (str == "playerHpFront") { u->SetVisible(true); }
		else if (str == "playerClassIcon") { u->SetVisible(true); }
		else if (str == "ammo_grenade") { u->SetVisible(true); }
		else if (str == "ammo_bullet") { u->SetVisible(true); }
		else if (str == "ammo_back") { u->SetVisible(true); }
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
		else if (str == "playerWeapon") { u->SetVisible(true); }
		else if (str == "bulletCount")
		{
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			vector<Part*> pWeapons = static_cast<Player*>(pPlayer)->GetParts();
			Part* pWeapon = pWeapons.at(0);
			Info* pInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
			_int iCurBullet = *std::get_if<_int>(pInfo->GetInfo().GetPtr("CurBullet"));
			static_cast<UILabel*>(u)->SetText(to_wstring(iCurBullet));
			u->SetVisible(true);
		}
		else if (str == "grenadeCount")	{ 
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			Info* pInfo = static_cast<Info*>(pPlayer->FindComponent(TEXT("Info")));
			_int iGrenadeCount = *std::get_if<_int>(pInfo->GetInfo().GetPtr("GrenadeCount"));
			static_cast<UILabel*>(u)->SetText(to_wstring(iGrenadeCount));
			u->SetVisible(true); 
		}
		else if (str == "specialCount")	{ 
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			Info* pInfo = static_cast<Info*>(pPlayer->FindComponent(TEXT("Info")));
			_int iSpecialCount = *std::get_if<_int>(pInfo->GetInfo().GetPtr("SpecialCount"));
			static_cast<UILabel*>(u)->SetText(L"x"+to_wstring(iSpecialCount));
			u->SetVisible(true); 
		}
		else if (str == "playerWeaponIcon") { 
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
			u->SetVisible(true); 
		}
		else if (str == "specialIcon") {
			static_cast<UIImage*>(u)->SetMaskingColor(_float4{0.4f, 1.f, 1.f, 1.f});
			u->SetVisible(true);
		}

		//hackingUI
		else if (str == "hacking_text_plate")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "hacking_image_plate")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}

		//hacking2UI
		else if (str == "hacking2_text_plate")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "hacking2_image_plate")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}

		//questUI
		else if (str == "quest_backplate")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}

		//hackingGame
		else if (str == "hacking_back")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "hacking_done1")
		{
			static_cast<UIImage*>(u)->SetBrightness(0.1f);
			static_cast<UIImage*>(u)->SetGamma(1.f);
		}
		else if (str == "hacking_done2")
		{
			static_cast<UIImage*>(u)->SetBrightness(0.1f);
			static_cast<UIImage*>(u)->SetGamma(1.0f);
		}
		else if (str == "hacking_progress")
		{
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
		}
		else if (str == "hacking_close") { static_cast<UIButton*>(u)->SetEnable(false);	}
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

		//hacking2Game
		else if (str == "hacking2_backplate") { 
			static_cast<UIImage*>(u)->SetBrightness(1.6f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "hacking2_close") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking2_minimize") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking2_updown1_up") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking2_updown2_up") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking2_updown3_up") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking2_updown4_up") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking2_updown1_down") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking2_updown2_down") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking2_updown3_down") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking2_updown4_down") { static_cast<UIButton*>(u)->SetEnable(false); }
		else if (str == "hacking2_updown1_back") { 
			static_cast<UIImage*>(u)->SetBrightness(0.5f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "hacking2_updown2_back") { 
			static_cast<UIImage*>(u)->SetBrightness(0.5f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "hacking2_updown3_back") { 
			static_cast<UIImage*>(u)->SetBrightness(0.5f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "hacking2_updown4_back") { 
			static_cast<UIImage*>(u)->SetBrightness(0.5f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
		else if (str == "hacking2_access_back") { 
			static_cast<UIImage*>(u)->SetBrightness(0.5f);
			static_cast<UIImage*>(u)->SetGamma(0.5f);
			static_cast<UIImage*>(u)->SetAlpha(0.5f);
		}
	}

	return S_OK;
}

HRESULT GameScene::ReadyMinimap()
{
	D3D11_RECT rect{};
	rect.left = 1132;
	rect.top = 553;
	rect.right = 1262;
	rect.bottom = 683;
	UI* pMinimap = m_pEngineUtility->FindUI(L"minimapGameScene");
	static_cast<UIImage*>(pMinimap)->SetScissor(rect);

	m_pEngineUtility->AddObject(SCENE::GAMEPLAY, L"minimapUI", SCENE::GAMEPLAY, L"UI", nullptr);
	UI* pUI = static_cast<UI*>(m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects().back());
	m_pEngineUtility->AddUI(L"minimapUI", pUI);

	static_cast<minimapUI*>(pUI)->SetPlayer(m_pEngineUtility->FindObject(SCENE::GAMEPLAY, L"Player", 0));
	static_cast<minimapUI*>(pUI)->SetScale(2.33f);
	static_cast<minimapUI*>(pUI)->Show(true);

	return S_OK;
}

HRESULT GameScene::ReadyMouse()
{
	//기존 마우스 안보이게
	while (ShowCursor(FALSE) >= 0) {}

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
	doors.at(6)->SetLock(true);

	panels.at(0)->SetPanelTag(L"panel0");

	return S_OK;
}

HRESULT GameScene::ReadyQuest()
{
	//퀘스트 1
	{
		QUEST_DESC q{};
		q.id = 1;
		q.state = QUEST_INACTIVE;
		q.nameKey = L"";
		q.descKey = L"적진 진입";

		// 트리거 진입
		{
			CONTENTS_DESC c{};
			c.id = 0;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_TRIGGER;
			c.targetTag = L"trigger0"; // TriggerBox 이름/태그
			c.triggerId = -1;                // ID 안 쓰면 -1
			c.requiredCount = 1;
			c.currentCount = 0;
			c.titleKey = L"구역 진입";
			c.descKey = L"표시된 구역으로 이동하십시오";
			q.contents.push_back(c);
		}

		// 몬스터 처치
		{
			CONTENTS_DESC c{};
			c.id = 1;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_KILL;
			c.targetTag = L"Drone";   // Enemy 타입 태그
			c.requiredCount = 6;
			c.titleKey = L"몬스터 처치";
			c.descKey = L"드론을 6기 무력화하십시오";
			q.contents.push_back(c);
		}
		{
			CONTENTS_DESC c{};
			c.id = 2;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_KILL;
			c.targetTag = L"Worm";   // Enemy 타입 태그
			c.requiredCount = 1;
			c.titleKey = L"몬스터 처치";
			c.descKey = L"웜을 1기 무력화하십시오";
			q.contents.push_back(c);
		}
		{
			CONTENTS_DESC c{};
			c.id = 3;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_KILL;
			c.targetTag = L"Shieldbug";   // Enemy 타입 태그
			c.requiredCount = 1;
			c.titleKey = L"몬스터 처치";
			c.descKey = L"실드버그를 1기 무력화하십시오";
			q.contents.push_back(c);
		}

		// 콘솔 상호작용
		{
			CONTENTS_DESC c{};
			c.id = 4;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_INTERACT;
			c.targetTag = L"panel0";
			c.requiredCount = 1;
			c.titleKey = L"목표물 조작";
			c.descKey = L"문 잠금 패널을 해킹하여 입구를 개방하십시오";
			q.contents.push_back(c);
		}

		// 퀘스트 등록
		Quest* pQuest = m_pEngineUtility->AddQuest(q);
		if (!pQuest)
			return E_FAIL;
	}
	m_pEngineUtility->SetContentsStartFunction(1, 0, []() {
		EngineUtility::GetInstance()->Play2DWithCallback("FBX_questGoNext", 1.f, []() {
			EngineUtility::GetInstance()->PlaySound2D("FBX_playerYes");
			});
	});
	m_pEngineUtility->SetContentsCompleteFunction(1, 4, []() {
		EngineUtility::GetInstance()->PlaySound2D("FBX_quest1End");
	});
	
	//퀘스트 2
	{
		QUEST_DESC q{};
		q.id = 2;
		q.state = QUEST_INACTIVE;
		q.nameKey = L"";
		q.descKey = L"탐사 진행";

		{
			CONTENTS_DESC c{};
			c.id = 0;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_INTERACT;
			c.targetTag = L"ammobag";
			c.requiredCount = 1;
			c.titleKey = L"보급 수령";
			c.descKey = L"보급 상자에서 탄창을 수령하십시오";
			q.contents.push_back(c);
		}
		{
			CONTENTS_DESC c{};
			c.id = 1;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_TRIGGER;
			c.targetTag = L"trigger8";
			c.triggerId = -1;
			c.requiredCount = 1;
			c.currentCount = 0;
			c.titleKey = L"구역 진입";
			c.descKey = L"탐사를 위해 더 깊은 곳으로 들어가십시오";
			q.contents.push_back(c);
		}
		{
			CONTENTS_DESC c{};
			c.id = 2;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_INTERACT;
			c.targetTag = L"itembox";
			c.requiredCount = 1;
			c.titleKey = L"무기 개방";
			c.descKey = L"수류탄 상자에서 수류탄을 수령하십시오";
			q.contents.push_back(c);
		}
		{
			CONTENTS_DESC c{};
			c.id = 3;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_KILL;
			c.targetTag = L"Shieldbug";
			c.requiredCount = 3;
			c.titleKey = L"몬스터 처치";
			c.descKey = L"수류탄으로 실드버그를 처치하십시오";
			q.contents.push_back(c);
		}

		Quest* pQuest = m_pEngineUtility->AddQuest(q);
		if (!pQuest)
			return E_FAIL;
	}
	m_pEngineUtility->SetContentsStartFunction(2, 1, []() {
		EngineUtility::GetInstance()->Play2DWithCallback("FBX_questGoDeeper", 1.f, []() {
			EngineUtility::GetInstance()->PlaySound2D("FBX_playerYes");
			});
	});
	m_pEngineUtility->SetContentsStartFunction(2, 2, []() {
		EngineUtility::GetInstance()->Play2DWithCallback("FBX_questPickup", 1.f, []() {
			EngineUtility::GetInstance()->PlaySound2D("FBX_playerYes");
			});
	});
	m_pEngineUtility->SetQuestCompleteFunction(2, []() {
		Object* pPlayer = EngineUtility::GetInstance()->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
		Info* pInfo = static_cast<Info*>(pPlayer->FindComponent(TEXT("Info")));
		_int iSpecialCount = *std::get_if<_int>(pInfo->GetInfo().GetPtr("SpecialCount"));
		iSpecialCount += 5;
		pInfo->GetInfo().SetData("SpecialCount", iSpecialCount);
		static_cast<UILabel*>(EngineUtility::GetInstance()->FindUI(L"specialCount"))->SetText(L"x" + to_wstring(iSpecialCount));
	});

	//퀘스트 3
	{
		QUEST_DESC q{};
		q.id = 3;
		q.state = QUEST_INACTIVE;
		q.nameKey = L"";
		q.descKey = L"연구 데이터 회수";

		{
			CONTENTS_DESC c{};
			c.id = 0;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_TRIGGER;
			c.targetTag = L"trigger9";
			c.triggerId = -1;
			c.requiredCount = 1;
			c.currentCount = 0;
			c.titleKey = L"구역 진입";
			c.descKey = L"연구 데이터를 찾아 앞으로 진행하십시오";
			q.contents.push_back(c);
		}
		{
			CONTENTS_DESC c{};
			c.id = 1;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_INTERACT;
			c.targetTag = L"console0";
			c.requiredCount = 1;
			c.titleKey = L"연구 콘솔 해킹";
			c.descKey = L"연구 데이터를 확보하기 위해 콘솔을 해킹하십시오";
			q.contents.push_back(c);
		}
		{
			CONTENTS_DESC c{};
			c.id = 2;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_INTERACT;
			c.targetTag = L"console1";
			c.requiredCount = 1;
			c.titleKey = L"연구 데이터 확보";
			c.descKey = L"연구 데이터를 확보하십시오";
			q.contents.push_back(c);
		}
		{
			CONTENTS_DESC c{};
			c.id = 3;
			c.state = CONTENTS_INACTIVE;
			c.type = CONTENTSTYPE_TRIGGER;
			c.targetTag = L"trigger7";
			c.triggerId = -1;
			c.requiredCount = 1;
			c.currentCount = 0;
			c.titleKey = L" 탈출";
			c.descKey = L"데이터를 가지고 탈출하십시오";
			q.contents.push_back(c);
		}

		Quest* pQuest = m_pEngineUtility->AddQuest(q);
		if (!pQuest)
			return E_FAIL;
	}
	m_pEngineUtility->SetContentsStartFunction(3, 1, []() {
		EngineUtility::GetInstance()->Play2DWithCallback("FBX_questHacking", 1.f, []() {
			EngineUtility::GetInstance()->PlaySound2D("FBX_playerYes");
			});
	});
	m_pEngineUtility->SetContentsStartFunction(3, 2, []() {
		EngineUtility::GetInstance()->Play2DWithCallback("FBX_questDownload", 1.f, []() {
			EngineUtility::GetInstance()->PlaySound2D("FBX_playerYes");
			});
	});
	m_pEngineUtility->SetContentsStartFunction(3, 3, []() {
		EngineUtility::GetInstance()->Play2DWithCallback("FBX_questEscape", 1.f, []() {
			EngineUtility::GetInstance()->PlaySound2D("FBX_playerYes");
			});
	});
	m_pEngineUtility->SetContentsCompleteFunction(3, 3, []() {
		EngineUtility::GetInstance()->PlaySound2D("FBX_quest3End");
	});

	// 퀘스트 시작 + 메인퀘스트로 지정
	m_pEngineUtility->StartQuest(1);
	m_pEngineUtility->SetMainQuestId(1);

	//퀘스트UI 생성
	{
		m_pEngineUtility->AddObject(SCENE::GAMEPLAY, L"questUI", SCENE::GAMEPLAY, L"UI", nullptr);
		UI* pUI = static_cast<UI*>(m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects().back());
		if (pUI == nullptr)
			return E_FAIL;

		m_pEngineUtility->AddUI(L"questUI", pUI);
		static_cast<QuestUI*>(pUI)->SetNpcType(QUEST_NPC_MALE);
		static_cast<QuestUI*>(pUI)->Show(false);
	}

	return S_OK;
}

HRESULT GameScene::ReadyEnding()
{
	if (m_pEngineUtility->FindUI(L"endingUI") == nullptr)
	{
		m_pEngineUtility->AddObject(SCENE::GAMEPLAY, TEXT("endingUI"), SCENE::GAMEPLAY, L"UI");
		UI* pUI = static_cast<UI*>(m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects().back());
		static_cast<endingUI*>(pUI)->SetCurScene(this);
		m_pEngineUtility->AddUI(L"endingUI", pUI);
	}

	vector<TriggerBox*> triggers = m_pEngineUtility->GetTriggerBoxes();
	triggers[7]->SetTriggerFunction([]() {
		static_cast<endingUI*>(EngineUtility::GetInstance()->FindUI(L"endingUI"))->SetEndingText("mission\ncomplete", endingUI::ENDTYPE::END_WIN);
		static_cast<endingUI*>(EngineUtility::GetInstance()->FindUI(L"endingUI"))->Show(true);
		});

	return S_OK;
}
