#pragma once

#include "Object.h"

NS_BEGIN(Engine)

class ENGINE_DLL Camera abstract : public Object
{
public:
	typedef struct tagCameraDesc : public Object::OBJECT_DESC
	{
		_float3 vEye = { 0.f, 0.f, 0.f};
		_float3 vAt = { 0.f, 0.f, 1.f };

		_float fFovy;
		_float fNear;
		_float fFar;
	}CAMERA_DESC;
protected:
	Camera();
	Camera(const Camera& Prototype);
	virtual ~Camera() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	_float GetFovy()   const;
	_float GetNear()   const;
	_float GetFar()    const;
	_float GetAspect() const;

	virtual Object* Clone(void* pArg) = 0;
	virtual void Free() override;

protected:
	_float					m_fFovy = {};
	_float					m_fNear = {};
	_float					m_fFar = {};
	_float					m_fAspect = {};
};

NS_END