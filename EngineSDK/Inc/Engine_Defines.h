#ifndef Engine_Define_h__
#define Engine_Define_h__

#include <d3d11.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>

#include "fx11/d3dx11effect.h"
#include "DirectXTK/DDSTextureLoader.h"
#include "DirectXTK/WICTextureLoader.h"
#include "DirectXTK/ScreenGrab.h"
#include "DirectXTK/SpriteFont.h"
#include "DirectXTK/SpriteBatch.h"
#include "DirectXTK/VertexTypes.h"
#include "DirectXTK/PrimitiveBatch.h"
#include "DirectXTK/Effects.h"

#ifdef new
#undef new
#endif
#ifdef _IMGUI
#include "IMGUI/imgui.h"
#include "IMGUI/backends/imgui_impl_win32.h"
#include "IMGUI/backends/imgui_impl_dx11.h"
#include "IMGUI/ImGuizmo.h"
#endif

#include "d3dcompiler.h"

#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <ctime>
#include <string>
#include <variant>
using namespace std;

#include <fstream>
#include <filesystem>
#include <memory>

#include <sstream>
#include <windows.h>

using namespace DirectX;
#include "Engine_Macro.h"
#include "Engine_Enum.h"
#include "Engine_Typedef.h"
#include "Engine_Struct.h"
#include "Engine_Function.h"

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

#pragma warning(disable : 4251)

namespace Engine
{
	static const _wstring		g_strTransformTag = TEXT("Transform");
	static constexpr _uint		g_iMaxNumBones = { 512 };

	//const unsigned int g_iMaxWidth = 16384;
	//const unsigned int g_iMaxHeight = 9216;
	
	const unsigned int g_iMaxWidth = 8192;
	const unsigned int g_iMaxHeight = 4608;
}

#ifdef _DEBUG

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

#ifndef DBG_NEW 

#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 

#endif
#endif



using namespace Engine;


#endif // Engine_Define_h__
