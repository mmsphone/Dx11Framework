#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL Transform final : public Component
{
public:
	typedef struct tagTransformDesc
	{
		_float fSpeedPerSec{};
		_float fRotationPerSec{};
	}TRANSFORM_DESC;
private:
	Transform();
	virtual ~Transform() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;

	_vector GetState(MATRIXROW eState) const;
	void SetState(MATRIXROW eState, _fvector vState);
	_float3 GetScale() const;
	void SetScale(_float fSizeX = 1.f, _float fSizeY = 1.f, _float fSizeZ = 1.f);

	void SetSpeedPerSec(_float& fSpeed);
	_float GetSpeedPerSec() const;
	void SetRotatePerSec(_float& fRotate);
	_float GetRotatePerSec() const;

	const _float4x4* GetWorldMatrixPtr() const;
	_matrix GetWorldMatrixInverse();

	void GoForward(_float fTimeDelta);
	void GoLeft(_float fTimeDelta);
	void GoRight(_float fTimeDelta);
	void GoBackward(_float fTimeDelta);
	void GoUp(_float fTimeDelta);
	void GoDown(_float fTimeDelta);

	void RotateRadian(_fvector vAxis, _float fRadian);
	void RotateTimeDelta(_fvector vAxis, _float fTimeDelta);
	void Turn(_fvector vAxis, _float fTimeDelta);

	void LookAt(_fvector vFocus);
	void Chase(_fvector vDest, _float fTimeDelta, _float fLimitDistance = 0.f);

	void Translate(_fvector vTranslateDir, _float fTimeDelta);

	HRESULT BindRenderTargetShaderResource(class Shader* pShader, const _char* pConstantName);

	static Transform* Create();
	virtual Component* Clone(void* pArg);
	virtual void Free() override;

private:
	_float4x4					m_WorldMatrix = { };
	_float						m_fSpeedPerSec = { 0.f };
	_float						m_fRotationPerSec = { 0.f };
};

NS_END