#pragma once

#include <Windows.h>
#include <process.h>

namespace Tool
{
	static const unsigned int g_iWinSizeX = 1280;
	static const unsigned int g_iWinSizeY = 720;

	enum SCENE : int { STATIC, MODEL, MAP, SCENE_END };
}

extern HINSTANCE	g_hInstance;
extern HWND			g_hWnd;

using namespace Tool;
