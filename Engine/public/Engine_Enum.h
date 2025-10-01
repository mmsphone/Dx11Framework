#ifndef Engine_Enum_h__
#define Engine_Enum_h__

namespace Engine
{	
	enum WINMODE : int { FULL, WIN, WINMODE_END };

	enum MOUSEKEYSTATE : int { LB, RB, KEY_WHEEL, MOUSEKEYSTATE_END };
	enum MOUSEMOVESTATE : int { X, Y, MOVE_WHEEL, MOUSEMOVESTATE_END };

	enum STATE : int { RIGHT, UP, LOOK, POSITION, STATE_END };
	enum PROTOTYPE : int { OBJECT, COMPONENT };
	enum RENDERGROUP : int { PRIORITY, NONBLEND, BLEND, UI, RENDERGROUP_END };
	enum LIGHT : int { DIRECTIONAL, POINT, LIGHT_END };
	enum D3DTS : int { VIEW, PROJECTION, D3DTS_END };
	enum MODELTYPE : int { ANIM, NONANIM };
}
#endif // Engine_Enum_h__
