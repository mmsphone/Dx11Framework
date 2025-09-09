#pragma once

#ifndef InputDev_h__
#define InputDev_h__

#include "Base.h"

NS_(Engine)

class Input final : public Base
{
private:
	Input();
	virtual ~Input() = default;

public:
	HRESULT Initialize(HINSTANCE InstanceHandle, HWND WindowHandle);
	void	Update();

	_byte	Get_DIKeyState(_ubyte byKeyID);
	_byte	Get_DIMouseState(MOUSEKEYSTATE eMouse);
	_long	Get_DIMouseMove(MOUSEMOVESTATE eMouseState);

	static Input* Create(HINSTANCE InstanceHandle, HWND WindowHandle);
	virtual void	Free() override;
private:
	LPDIRECTINPUT8			m_pInputSDK = nullptr;

	LPDIRECTINPUTDEVICE8	m_pKeyboard = nullptr;
	LPDIRECTINPUTDEVICE8	m_pMouse = nullptr;

	_byte					m_byKeyState[256] = {};
	DIMOUSESTATE			m_tMouseState = {};
};

_NS
#endif