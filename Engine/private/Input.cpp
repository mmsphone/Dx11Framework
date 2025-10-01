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
	m_pKeyboard->Acquire(); // keyboard access
	//Mouse 생성
	if (FAILED(m_pInputSDK->CreateDevice(GUID_SysMouse, &m_pMouse, nullptr)))
		return E_FAIL;
	//Mouse 설정
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

_byte Input::GetKeyState(_ubyte byKeyID)
{
	return m_byKeyState[byKeyID];
}

_byte Input::GetMouseState(MOUSEKEYSTATE eMouse)
{
	return m_tMouseState.rgbButtons[ENUM_CLASS(eMouse)];
}

_long Input::GetMouseMove(MOUSEMOVESTATE eMouseState)
{
	return *(((_long*)&m_tMouseState) + ENUM_CLASS(eMouseState));
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
