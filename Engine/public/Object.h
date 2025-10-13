#pragma once

#include "Base.h"
#include "Transform.h"

NS_BEGIN(Engine)

class ENGINE_DLL Object abstract : public Base
{
public:
	typedef struct tagObjectDesc : public Transform::TRANSFORM_DESC
	{

	}OBJECT_DESC;
protected:
	Object();
	Object(const Object& Prototype);
	virtual ~Object() = default;

public:
	virtual HRESULT InitializePrototype();
	virtual HRESULT Initialize(void* pArg);

	virtual void PriorityUpdate(_float fTimeDelta);
	virtual void Update(_float fTimeDelta);
	virtual void LateUpdate(_float fTimeDelta);

	virtual HRESULT Render();

	virtual Object* Clone(void* pArg) = 0;
	virtual void Free();

	virtual HRESULT AddComponent(_uint iPrototypeSceneId, const _wstring& strPrototypeTag, const _wstring& strComponentTag, Component** ppOut, void* pArg = nullptr);
	virtual class Component* FindComponent(const _wstring& strComponentTag);

protected:
	class EngineUtility* m_pEngineUtility = { nullptr };
	map<const _wstring, class Component*> m_Components;
};

NS_END