#include "GameScene.h"

#include "EngineUtility.h"

#include "FreeCam.h"
#include "FixedCam.h"
#include "ChaseCam.h"

#include "Layer.h"
#include "UIImage.h"
#include "UILabel.h"
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

	PanelDoorLink();

	return S_OK;
}

void GameScene::Update(_float fTimeDelta)
{
}

HRESULT GameScene::Render()
{
	SetWindowText(g_hWnd, TEXT("게임플레이레벨입니다."));

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
	Desc.offset = _vector{ 0.f, 8.f, -5.f, 0.f };
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
	m_pEngineUtility->LoadUI("../bin/data/minimap.dat", SCENE::GAMEPLAY);
	m_pEngineUtility->LoadUI("../bin/data/playerUI.dat", SCENE::GAMEPLAY);

	D3D11_RECT rect{};
	rect.left = 1132.f;
	rect.top = 552.f;
	rect.right = 1262.f;
	rect.bottom = 685.f;
	list<Object*> UIs = m_pEngineUtility->FindLayer(SCENE::GAMEPLAY, L"UI")->GetAllObjects();
	for (auto& ui : UIs)
	{
		UI* u = dynamic_cast<UI*>(ui);
		if (u == nullptr)
			continue;
		string str = u->GetUIDesc().name;
		if (str == "minimapGameScene")
		{
			static_cast<UIImage*>(u)->SetScissor(rect);
		}
		else if (str == "playerWeapon")
		{
			m_pEngineUtility->AddUI(L"playerWeapon", u);
		}
		else if (str == "playerHpFront")
		{
			m_pEngineUtility->AddUI(L"playerHpFront", u);
		}
		else if (str == "bulletCount")
		{
			m_pEngineUtility->AddUI(L"bulletCount", u);
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			vector<Part*> pWeapons = static_cast<Player*>(pPlayer)->GetParts();
			Part* pWeapon = pWeapons.at(0);
			Info* pInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
			_int iCurBullet = *std::get_if<_int>(pInfo->GetInfo().GetPtr("CurBullet"));
			static_cast<UILabel*>(u)->SetText(to_wstring(iCurBullet));

		}
		else if (str == "grenadeCount")
		{
			m_pEngineUtility->AddUI(L"grenadeCount", u);
		}
		else if (str == "specialCount")
		{
			m_pEngineUtility->AddUI(L"specialCount", u);
		}
		else if (str == "ammo_front1")
		{
			m_pEngineUtility->AddUI(L"ammo_front1", u);
			Object* pPlayer = m_pEngineUtility->FindObject(SCENE::GAMEPLAY, TEXT("Player"), 0);
			vector<Part*> pWeapons = static_cast<Player*>(pPlayer)->GetParts();
			Part* pWeapon = pWeapons.at(0);
			Info* pInfo = static_cast<Info*>(pWeapon->FindComponent(TEXT("Info")));
			_int iCurAmmo = *std::get_if<_int>(pInfo->GetInfo().GetPtr("CurAmmo"));
			m_pEngineUtility->FindUI(L"ammo_front1")->SetVisible(iCurAmmo >= 1);
		}
		else if (str == "ammo_front2")
		{
			m_pEngineUtility->AddUI(L"ammo_front2", u);
		}
		else if (str == "ammo_front3")
		{
			m_pEngineUtility->AddUI(L"ammo_front3", u);
		}
		else if (str == "ammo_front4")
		{
			m_pEngineUtility->AddUI(L"ammo_front4", u);
		}
		else if (str == "ammo_front5")
		{
			m_pEngineUtility->AddUI(L"ammo_front5", u);
		}
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