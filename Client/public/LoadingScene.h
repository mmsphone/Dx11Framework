#pragma once

#include "Client_Defines.h"
#include "Scene.h"

NS_BEGIN(Client)

/* 씬에 대한 자원 관리*/
/* 씬을 구성해주기 위한 객체 생성, 업데이트, 랜더 호출으로 동적인 로딩 화면 구성*/

class LoadingScene final : public Scene
{
private:
	LoadingScene();
	virtual ~LoadingScene() = default;

public:
	virtual HRESULT Initialize(SCENE eNextSceneId);
	virtual void Update(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static LoadingScene* Create(SCENE eNextSceneId);
	virtual void Free() override;

private:
	HRESULT ReadyLayerBackGround();

private:
	SCENE			m_eNextSceneId = { SCENE::SCENE_END };
	class Loader* m_pLoader = { nullptr };
};

NS_END