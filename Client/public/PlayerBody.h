#pragma once

#include "Client_Defines.h"
#include "Part.h"

NS_BEGIN(Engine)
class Shader;
class Model;
NS_END

NS_BEGIN(Client)

class PlayerBody final : public Part
{
public:
	typedef struct tagPlayerBodyDesc final : public Part::PART_DESC
	{
		const _uint* pParentState = { nullptr };
	}PLAYERBODY_DESC;
private:
	PlayerBody();
	PlayerBody(const PlayerBody& Prototype);
	virtual ~PlayerBody() = default;

public:
	const _float4x4* GetSocketBoneMatrixPtr(const _char* pBoneName);

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static Object* Create();/* 원형생성 */
	virtual Object* Clone(void* pArg) override;/* 사본생성 */
	virtual void Free();

private:
	HRESULT ReadyComponents();
	HRESULT BindShaderResources();
private:
	const _uint* m_pParentState = { nullptr };
};

NS_END