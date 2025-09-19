#ifndef Engine_Enum_h__
#define Engine_Enum_h__

namespace Engine
{	
	enum WINMODE : int { FULL, WIN, END };

	enum STATE : int { RIGHT, UP, LOOK, POSITION, END };
	enum PROTOTYPE : int { GAMEOBJECT, COMPONENT };
	enum RENDERGROUP : int { PRIORITY, NONBLEND, BLEND, UI, END };
	enum D3DTS : int { VIEW, PROJECTION, END };

	enum MOUSEKEYSTATE : int { LB, RB, WHEEL, END };
	enum MOUSEMOVESTATE : int { X, Y, WHEEL, END };
}
#endif // Engine_Enum_h__
