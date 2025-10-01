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

#include "d3dcompiler.h"


#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <ctime>
using namespace std;
#include <string>

#include <fstream>
#include <memory>

using namespace DirectX;
#include "Engine_Enum.h"
#include "Engine_Macro.h"
#include "Engine_Struct.h"
#include "Engine_Typedef.h"
#include "Engine_Function.h"

#define DIRECTINPUT_VERSION	0x0800
#include <dinput.h>

#pragma warning(disable : 4251)

namespace Engine
{
	static const _wstring		g_strTransformTag = TEXT("Transform");
	static constexpr _uint		g_iMaxNumBones = { 512 };
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
