#include "Input.h"
#include "EngineUtility.h"

Input::Input()
{
}

HRESULT Input::Initialize(HINSTANCE InstanceHandle, HWND WindowHandle)
{
	//DInput 생성
	if (FAILED(DirectInput8Create(InstanceHandle, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pInputSDK, NULL)))
		return E_FAIL;

	//Keyboard 생성
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, nullptr)))
		return E_FAIL;
	//Keyboard 설정
	m_pKeyboard->SetDataFormat(&c_dfDIKeyboard); // keyboard data
	m_pKeyboard->SetCooperativeLevel(WindowHandle, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE); // keyboard input
	if (FAILED(m_pKeyboard->Acquire())) // keyboard access
		return E_FAIL;
	//Mouse 생성
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
		return E_FAIL;
	//Mouse 설정
	m_pMouse->SetDataFormat(&c_dfDIMouse); // mouse data
	m_pMouse->SetCooperativeLevel(WindowHandle, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE); // mouse input
	if (FAILED(m_pMouse->Acquire())) // mouse access
		return E_FAIL;

	return S_OK;
}

void Input::Update()
{
	HRESULT hr = m_pKeyboard->GetDeviceState(256, m_byKeyState);
	if (FAILED(hr))
	{
		if(hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
			m_pKeyboard->Acquire();
		else
		ZeroMemory(m_byKeyState, sizeof(m_byKeyState));
	}
	if (FAILED(m_pMouse->GetDeviceState(sizeof(m_tMouseState), &m_tMouseState)))
	{
		if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
			m_pMouse->Acquire();
		else
		ZeroMemory(&m_tMouseState, sizeof(m_tMouseState));
	}
}

_byte Input::GetKeyState(_ubyte byKeyID)
{
	return m_byKeyState[byKeyID];
}

_byte Input::GetMouseState(MOUSEKEYSTATE eMouse)
{
	if (eMouse < 0 || eMouse >= MOUSEKEYSTATE::MOUSEKEYSTATE_END)
		return 0;

	return m_tMouseState.rgbButtons[eMouse];
}

_long Input::GetMouseMove(MOUSEMOVESTATE eMouseState)
{
	switch (eMouseState)
	{
	case MOUSEMOVESTATE::X:
		return m_tMouseState.lX;
	case MOUSEMOVESTATE::Y:
		return m_tMouseState.lY;
	case MOUSEMOVESTATE::MOVE_WHEEL:
		return m_tMouseState.lZ;
	default:
		return 0;
	}
}

Input* Input::Create(HINSTANCE InstanceHandle, HWND WindowHandle)
{
	Input* pInstance = new Input();

	if (FAILED(pInstance->Initialize(InstanceHandle, WindowHandle)))
	{
		MSG_BOX("Failed to Created : Input");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void Input::Free()
{
	__super::Free();

	SafeRelease(m_pKeyboard);
	SafeRelease(m_pMouse);
	SafeRelease(m_pInputSDK);
}
