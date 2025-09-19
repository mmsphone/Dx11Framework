#pragma once

#include "Base.h"

NS_(Engine)

/* 씬 전환할 때,  이전 씬 파괴와 자원 정리*/
/* 현재 필요한 씬 보관*/
/* 반복적인 업데이트, 랜더 호출*/
class SceneManager final : public Base
{
private:
	SceneManager();
	virtual ~SceneManager() = default;

public:
	HRESULT ChangeScene(_uint iSceneId, class Scene* pScene);
	void Update(_float fTimeDelta);
	HRESULT Render();

	static SceneManager* Create();
	virtual void Free() override;

private:
	_uint m_iSceneId = {};
	class Scene* m_pCurrentScene = { nullptr };
	class EngineUtility* m_pEngineUtility = { nullptr };
};

_NS