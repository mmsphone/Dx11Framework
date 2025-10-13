#pragma once

#include "Tool_Defines.h"
#include "Object.h"

NS_BEGIN(Engine)
class Shader;
class Model;
NS_END

NS_BEGIN(Tool)

class TestObject final : public Object
{
private:
	TestObject();
	TestObject(const TestObject& Prototype);
	virtual ~TestObject() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static Object* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free();

private:
	HRESULT ReadyComponents();
};

NS_END