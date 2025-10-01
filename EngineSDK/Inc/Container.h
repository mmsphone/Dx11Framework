#pragma once

#include "Object.h"

NS_BEGIN(Engine)

class ENGINE_DLL Container abstract : public Object
{ 
public:
	typedef struct tagContainerObjectDesc : public Object::OBJECT_DESC
	{
		_uint		iNumParts;
	}CONTAINER_DESC;
protected:
	Container();
	Container(const Container& Prototype);
	virtual ~Container() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	virtual Object* Clone(void* pArg) = 0;
	virtual void Free() override;

protected:
	HRESULT AddPart(_uint iPrototypeLevelID, const _wstring& strPrototypeTag, _uint iPartObjectIndex, void* pArg);

protected:
	_uint							m_iNumParts = {};
	vector<class Part*>		m_Parts;

};

NS_END