#pragma once

#include "Client_Defines.h"
#include "UI.h"

NS_BEGIN(Engine)
class Shader;
class Texture;
class VIBufferRect;
NS_END

NS_BEGIN(Client)

class Background final : public UI
{
public:
	typedef struct tagBackGroundDesc : public UI::UI_DESC
	{

	}BACKGROUND_DESC;

private:
	Background();
	Background(const Background& Prototype);
	virtual ~Background() = default;

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