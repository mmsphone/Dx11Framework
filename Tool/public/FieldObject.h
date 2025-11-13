#pragma once

#include "Tool_Defines.h"
#include "Object.h"

NS_BEGIN(Engine)
class Shader;
class Model;
NS_END

NS_BEGIN(Tool)

class FieldObject final : public Object
{
private:
	FieldObject();
	FieldObject(const FieldObject& Prototype);
	virtual ~FieldObject() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT RenderShadow(_uint iIndex) override;

	static Object* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free();

private:
	HRESULT ReadyComponents();
};

NS_END