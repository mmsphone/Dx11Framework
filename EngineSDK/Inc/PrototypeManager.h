#pragma once

#include "Base.h"

#include "Model.h"
#include "Shader.h"
#include "Texture.h"
#include "Transform.h"
#include "Collision.h"
#include "StateMachine.h"
#include "AIController.h"

#include "VIBufferRect.h"
#include "VIBufferCube.h"
#include "VIBufferTerrain.h"

NS_BEGIN(Engine)

/* 원형 객체 보관, 복제해 반환*/

class PrototypeManager final : public Base
{
private:
	PrototypeManager();
	virtual ~PrototypeManager() = default;

public:
	HRESULT Initialize(_uint iNumScenes);
	_bool HasPrototype(_uint iSceneId, const _wstring& strPrototypeTag);
	HRESULT AddPrototype(_uint iSceneId, const _wstring& strPrototypeTag, Base* pPrototype);
	Base* ClonePrototype(PROTOTYPE eType, _uint iSceneId, const _wstring& strPrototypeTag, void* pArg = nullptr);
	void Clear(_uint iSceneId);

	static PrototypeManager* Create(_uint iNumLevels);
	virtual void Free() override;

private:
	Base* FindPrototype(_uint iSceneId, const _wstring& strPrototypeTag);

private:
	_uint m_iNumScenes = {};
	map<const _wstring, Base*>* m_pPrototypes = { nullptr };
	typedef map<const _wstring, Base*> PROTOTYPES;
};

NS_END