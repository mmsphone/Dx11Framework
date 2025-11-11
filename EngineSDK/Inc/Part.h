#pragma once

#include "Object.h"

NS_BEGIN(Engine)
class ENGINE_DLL Part abstract : public Object
{
public:
	typedef struct tagPartObjectDesc : public Object::OBJECT_DESC
	{
		const _float4x4* pParentMatrix;
	}PART_DESC;

protected:
	Part();
	Part(const Part& Prototype);
	virtual ~Part() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	const _float4x4* GetCombinedWorldMatrix() const;

	virtual Object* Clone(void* pArg) = 0;
	virtual void Free() override;
protected:
	const _float4x4* m_pParentMatrix = { nullptr };
	_float4x4			m_CombinedWorldMatrix = { };
};

NS_END