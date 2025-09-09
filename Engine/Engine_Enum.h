#ifndef Engine_Enum_h__
#define Engine_Enum_h__

namespace Engine
{	
	enum class WINMODE { FULL, WIN, END };

	enum class STATE { RIGHT, UP, LOOK, POSITION, END };
	enum class PROTOTYPE { GAMEOBJECT, COMPONENT };
	enum class RENDERGROUP { PRIORITY, NONBLEND, BLEND, UI, END };
	enum class D3DTS { VIEW, PROJECTION, END };

	enum class MOUSEKEYSTATE { LB, RB, WHEEL, END };
	enum class MOUSEMOVESTATE { X, Y, WHEEL, END };
}
#endif // Engine_Enum_h__
