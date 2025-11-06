#ifndef Engine_Struct_h__
#define Engine_Struct_h__

namespace Engine
{
	typedef struct tagEngineDesc
	{
		HINSTANCE		hInstance;
		HWND			hWnd;

		WINMODE			eWinMode;
		unsigned int	iWinSizeX, iWinSizeY;
		unsigned int	iNumLevels;
	}ENGINE_DESC;

	typedef struct tagVertexPostion
	{
		XMFLOAT3		vPosition;
		static const unsigned int					iNumElements = { 1 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXPOS;

	typedef struct tagVertexPostionTexcoord
	{
		XMFLOAT3		vPosition;
		XMFLOAT2		vTexcoord;

		static const unsigned int					iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	}VTXPOSTEX;

	typedef struct tagVertexPostionNormalTexcoord
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;

		static const unsigned int					iNumElements = { 3 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	}VTXNORTEX;

	typedef struct tagVertexMesh
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT3		vTangent;

		static const unsigned int					iNumElements = { 4 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	}VTXMESH;

	typedef struct tagVertexSkinnedMesh
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT3		vTangent;

		XMUINT4			vBlendIndex;
		XMFLOAT4		vBlendWeight;

		static const unsigned int					iNumElements = { 6 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	}VTXSKINMESH;

	typedef struct tagVertexCube
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vTexcoord;

		static const unsigned int					iNumElements = { 2 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	}VTXCUBE;

	typedef struct tagLightDesc
	{
		LIGHT			eType;

		XMFLOAT4		vDiffuse;
		XMFLOAT4		vAmbient;
		XMFLOAT4		vSpecular;

		XMFLOAT4		vDirection;
		XMFLOAT4		vPosition;
		float			fRange;

	}LIGHT_DESC;

	typedef struct tagRay
	{
		XMFLOAT3 origin;
		XMFLOAT3 direction;
	}RAY;

	typedef struct tagPickResult
	{
		bool hit = false;
		class Object* pHitObject = nullptr;
		XMFLOAT3 hitPos = { 0.f,0.f,0.f };
		PICKTYPE pickType = PICK_END;
	}PICK_RESULT;

	typedef struct tagMapObjectData
	{
		std::string objectName;
		std::string modelPath;
		XMFLOAT4X4   worldMatrix;
	}MAP_OBJECTDATA;

	struct EventData {
		std::string name;
		float fParam{ 0.f };
		void* pParam{ nullptr };	
	};

	struct StateProc {
		std::function<void(class Object*, class StateMachine*)>                      enter;
		std::function<void(class Object*, class StateMachine*, float)>               update;
		std::function<void(class Object*, class StateMachine*)>                      exit;
		std::function<bool(class Object*, class StateMachine*, const EventData&)>    onEvent;
	};

	struct Transition {
		std::function<bool(class Object* pOwner, class StateMachine* stateMachine)> condition;
		std::string nextState;
		unsigned int priority = 0;
	};

	typedef struct tagAIControllerBlackBoardDesc
	{
		bool   hasTarget = false;		// 플레이어를 찾았는가
		bool   hasLOS = false;			// 시야가림 없는가
		bool   inFOV = false;			// 시야각 안에 있는가
		bool   inAttackRange = false;   // 공격 사거리 이내인가
		float  distance = 0.f;			// 소유자-타겟 거리
		float  cosHalfFov = 0.f;		// cos(FOV/2)
		XMFLOAT4 ownerPos = {};			// 소유자 월드 위치
		XMFLOAT4 ownerLook = {};		// 소유자 정규화된 LOOK
		XMFLOAT4 targetPos = {};		// 타겟(플레이어) 월드 위치
	}AIBLACKBOARD_DESC;
}


#endif // Engine_Struct_h__
