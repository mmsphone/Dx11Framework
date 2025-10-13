#pragma once

#include "Tool_Defines.h"
#include "Camera.h"

NS_BEGIN(Tool)

class FreeCam final : public Camera
{
public:
	typedef struct tagCamera_Free_Desc : public Camera::CAMERA_DESC
	{
		_float		fSensor;
	}FREECAM_DESC;
private:
	FreeCam();
	FreeCam(const FreeCam& Prototype);
	virtual ~FreeCam() = default;

public:
	virtual HRESULT InitializePrototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void PriorityUpdate(_float fTimeDelta) override;
	virtual void Update(_float fTimeDelta) override;
	virtual void LateUpdate(_float fTimeDelta) override;
	virtual HRESULT Render() override;

	static FreeCam* Create();
	virtual Object* Clone(void* pArg) override;
	virtual void Free() override;
private:
	_float			m_fSensor = { };
};

NS_END