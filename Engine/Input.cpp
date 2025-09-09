#include "Input.h"

Input::Input()
{
}

HRESULT Input::Initialize(HINSTANCE InstanceHandle, HWND WindowHandle)
{
	//Create DInput
	if (FAILED(DirectInput8Create(InstanceHandle, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pInputSDK, NULL)))
		return E_FAIL;

	//Create Keyboard
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, nullptr)))
		return E_FAIL;
	//Set Keyboard
	m_pKeyboard->SetDataFormat(&c_dfDIKeyboard); // keyboard data
	m_pKeyboard->SetCooperativeLevel(WindowHandle, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE); // keyboard input
	m_pKeyboard->Acquire(); // keyboard access
	//Create Mouse
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
		return E_FAIL;
	//Set Mouse
	m_pMouse->SetDataFormat(&c_dfDIMouse); // mouse data
	m_pMouse->SetCooperativeLevel(WindowHandle, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE); // mouse input
	m_pMouse->Acquire(); // mouse access

	return S_OK;
}

void Input::Update()
{
	m_pKeyboard->GetDeviceState(256, m_byKeyState);
	m_pMouse->GetDeviceState(sizeof(m_tMouseState), &m_tMouseState);
}

_byte Input::Get_DIKeyState(_ubyte byKeyID)
{
	return m_byKeyState[byKeyID];
}

_byte Input::Get_DIMouseState(MOUSEKEYSTATE eMouse)
{
	return m_tMouseState.rgbButtons[ENUM_CLASS(eMouse)];
}

_long Input::Get_DIMouseMove(MOUSEMOVESTATE eMouseState)
{
	return *(((_long*)&m_tMouseState) + ENUM_CLASS(eMouseState));
}

Input* Input::Create(HINSTANCE InstanceHandle, HWND WindowHandle)
{
	Input* pInstance = new Input();

	if (FAILED(pInstance->Initialize(InstanceHandle, WindowHandle)))
	{
		MSG_BOX("Failed to Created : Input");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void Input::Free()
{
	__super::Free();

	Safe_Release(m_pKeyboard);
	Safe_Release(m_pMouse);
	Safe_Release(m_pInputSDK);
}
