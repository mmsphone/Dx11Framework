#include "Input.h"
#include "EngineUtility.h"

Input::Input()
	: m_pEngineUtility{EngineUtility::GetInstance() }
{
	SafeAddRef(m_pEngineUtility);
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
	//상태 백업
	memcpy(m_preKeyState, m_curKeyState, sizeof(m_curKeyState));
	m_preMouse = m_curMouse;

	//현재 상태
	HRESULT hrKey = m_pKeyboard->GetDeviceState(256, m_curKeyState);
	if (FAILED(hrKey))
	{
		if(hrKey == DIERR_INPUTLOST || hrKey == DIERR_NOTACQUIRED)
			m_pKeyboard->Acquire();
		else
			ZeroMemory(m_curKeyState, sizeof(m_curKeyState));
	}
	HRESULT hrMouse = m_pMouse->GetDeviceState(sizeof(m_curMouse), &m_curMouse);
	if (FAILED(hrMouse))
	{
		if (hrMouse == DIERR_INPUTLOST || hrMouse == DIERR_NOTACQUIRED)
			m_pMouse->Acquire();
		else
			ZeroMemory(&m_curMouse, sizeof(m_curMouse));
	}

	//키 비교
	for (_int i = 0; i < 256; ++i)
	{
		_bool wasDown = (m_preKeyState[i] & 0x80);
		_bool isDown = (m_curKeyState[i] & 0x80);

		if (!wasDown && isDown) m_KeyState[i] = KEY_PRESSED;
		else if (wasDown && isDown) m_KeyState[i] = KEY_DOWN;
		else if (wasDown && !isDown) m_KeyState[i] = KEY_RELEASED;
		else m_KeyState[i] = KEY_UP;
	}

	// 마우스 버튼 비교
	for (_int i = 0; i < MOUSEKEYSTATE_END; ++i)
	{
		_bool wasDown = (m_preMouse.rgbButtons[i] & 0x80);
		_bool isDown = (m_curMouse.rgbButtons[i] & 0x80);

		if (!wasDown && isDown) m_MouseState[i] = KEY_PRESSED;
		else if (wasDown && isDown) m_MouseState[i] = KEY_DOWN;
		else if (wasDown && !isDown) m_MouseState[i] = KEY_RELEASED;
		else m_MouseState[i] = KEY_UP;
	}
}

_byte Input::GetKeyState(_ubyte byKeyID)
{
	return m_curKeyState[byKeyID];
}

_byte Input::GetMouseState(MOUSEKEYSTATE eMouse)
{
	if (eMouse < 0 || eMouse >= MOUSEKEYSTATE::MOUSEKEYSTATE_END)
		return 0;

	return m_curMouse.rgbButtons[eMouse];
}

_long Input::GetMouseMove(MOUSEMOVESTATE eMouseState)
{
	switch (eMouseState)
	{
	case MOUSEMOVESTATE::MOUSEMOVE_X:
		return m_curMouse.lX;
	case MOUSEMOVESTATE::MOUSEMOVE_Y:
		return m_curMouse.lY;
	case MOUSEMOVESTATE::MOUSEMOVE_WHEEL:
		return m_curMouse.lZ;
	default:
		return 0;
	}
}

_float2 Input::GetMousePos()
{
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(m_pEngineUtility->GetWindowHandle(), &mousePos);

	return _float2(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
}

void Input::SetMousePos(_float2 mousePos)
{
	HWND hWnd = m_pEngineUtility->GetWindowHandle();
	if (hWnd == nullptr) return;

	POINT pt = { static_cast<LONG>(mousePos.x), static_cast<LONG>(mousePos.y) };
	ClientToScreen(hWnd, &pt);
	SetCursorPos(pt.x, pt.y);
}

void Input::SetMouseVisible(_bool bVisible)
{
	static bool cursorVisible = true;
	if (cursorVisible != bVisible)
	{
		ShowCursor(bVisible);
		cursorVisible = bVisible;
	}
}

_bool Input::IsKeyDown(_ubyte byKeyID) const
{
	return m_KeyState[byKeyID] == KEY_DOWN;
}

_bool Input::IsKeyPressed(_ubyte byKeyID) const
{
	return m_KeyState[byKeyID] == KEY_PRESSED;
}

_bool Input::IsKeyReleased(_ubyte byKeyID) const
{
	return m_KeyState[byKeyID] == KEY_RELEASED;
}

_bool Input::IsKeyUp(_ubyte byKeyID) const
{
	return m_KeyState[byKeyID] == KEY_UP;
}

_bool Input::IsMouseDown(MOUSEKEYSTATE eMouse) const
{
	return m_MouseState[eMouse] == KEY_DOWN;
}

_bool Input::IsMousePressed(MOUSEKEYSTATE eMouse) const
{
	return m_MouseState[eMouse] == KEY_PRESSED;
}

_bool Input::IsMouseReleased(MOUSEKEYSTATE eMouse) const
{
	return m_MouseState[eMouse] == KEY_RELEASED;
}

_bool Input::IsMouseUp(MOUSEKEYSTATE eMouse) const
{
	return m_MouseState[eMouse] == KEY_UP;
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
	SafeRelease(m_pEngineUtility);
}
