#pragma once

#ifndef InputDev_h__
#define InputDev_h__

#include "Base.h"

NS_BEGIN(Engine)

class Input final : public Base
{
private:
	Input();
	virtual ~Input() = default;

public:
	//DInput 초기화
	HRESULT Initialize(HINSTANCE InstanceHandle, HWND WindowHandle);
	void	Update();
	//DInput 상태값 조회
	_byte	GetKeyState(_ubyte byKeyID);
	_byte	GetMouseState(MOUSEKEYSTATE eMouse);
	_long	GetMouseMove(MOUSEMOVESTATE eMouseState);
	_float2 GetMousePos();

	//Cursor 제어
	void SetMousePos(_float2 mousePos);
	void SetMouseVisible(_bool bVisible);

	static Input* Create(HINSTANCE InstanceHandle, HWND WindowHandle);
	virtual void	Free() override;
private:
	LPDIRECTINPUT8			m_pInputSDK = nullptr;

	LPDIRECTINPUTDEVICE8	m_pKeyboard = nullptr;
	LPDIRECTINPUTDEVICE8	m_pMouse = nullptr;

	_byte					m_byKeyState[256] = {};
	DIMOUSESTATE			m_tMouseState = {};

	class EngineUtility* m_pEngineUtility = nullptr;
};

NS_END
#endif