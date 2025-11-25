#ifndef Engine_Struct_h__
#define Engine_Struct_h__

namespace Engine
{
	//엔진용 데이터
	typedef struct tagEngineDesc
	{
		HINSTANCE		hInstance;
		HWND			hWnd;

		WINMODE			eWinMode;
		unsigned int	iWinSizeX, iWinSizeY;
		unsigned int	iNumLevels;
	}ENGINE_DESC;

	//InputLayout - POS
	typedef struct tagVertexPostion
	{
		XMFLOAT3		vPosition;
		static const unsigned int					iNumElements = { 1 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
	}VTXPOS;

	//InputLayout - POS + TEX
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

	//InputLayout - POS + TEX (이름만 다름)
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

	//InputLayout - POS + NOR + TEX
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

	//InputLayout - POS + NOR + TEX + TAN + BINOR
	typedef struct tagVertexMesh
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT3		vTangent;
		XMFLOAT3		vBinormal;

		static const unsigned int					iNumElements = { 5 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	}VTXMESH;

	//InputLayout - POS + NOR + TEX + TAN + BINOR + BLENDINDEX + BLENDWEIGHT
	typedef struct tagVertexSkinnedMesh
	{
		XMFLOAT3		vPosition;
		XMFLOAT3		vNormal;
		XMFLOAT2		vTexcoord;
		XMFLOAT3		vTangent;

		XMUINT4			vBlendIndex;
		XMFLOAT4		vBlendWeight;

		XMFLOAT3		vBinormal;

		static const unsigned int					iNumElements = { 7 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDEX", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 76, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
	}VTXSKINMESH;

	//InputLayout - ??
	typedef struct tagVertexPosTexInstanceParticle
	{
		static const unsigned int					iNumElements = { 7 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },

			{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "TEXCOORD", 5, DXGI_FORMAT_R32G32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};
	}VTXPOSTEX_INSTANCEPARTICLE;

	//InputLayout - ??
	typedef struct tagVertexPosInstanceParticle
	{
		static const unsigned int					iNumElements = { 6 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },

			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
		};
	}VTXPOS_INSTANCEPARTICLE;

	//InputLayout - POS + TEX + WORLD
	typedef struct tagVertexPostexInstanceWorld
	{
		static const unsigned int					iNumElements = { 6 };
		static constexpr D3D11_INPUT_ELEMENT_DESC		Elements[iNumElements] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,   0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA,   0 },

			{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,  D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
			{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
		};
	}VTXPOSTEX_INSTANCEWORLD;

	//파티클 인스턴스 행렬 연산용 데이터 구조체
	typedef struct tagVertexInstanceParticle
	{
		XMFLOAT4			vRight, vUp, vLook, vTranslation;
		XMFLOAT2			vLifeTime;
	}VTXINSTANCEPARTICLE;

	//매쉬 인스턴스 행렬 연산용 데이터 구조체
	typedef struct tagVertexInstanceMesh
	{
		XMFLOAT4			vRight, vUp, vLook, vTranslation;
	}VTXINSTANCEMESH;

	//레이 구조체
	typedef struct tagRay
	{
		XMFLOAT3 origin;
		XMFLOAT3 direction;
	}RAY;

	//피킹 결과
	typedef struct tagPickResult
	{
		bool hit = false;
		class Object* pHitObject = nullptr;
		XMFLOAT3 hitPos = { 0.f,0.f,0.f };
		PICKTYPE pickType = PICKTYPE_END;
	}PICK_RESULT;

	//빛 데이터
	typedef struct tagLightDesc
	{
		// 빛 타입
		LIGHT			eType;

		//빛 기본 정보
		XMFLOAT4		vDiffuse;
		XMFLOAT4		vAmbient;
		XMFLOAT4		vSpecular;

		//방향성라이트
		XMFLOAT4		vDirection;

		//포인트라이트
		XMFLOAT4		vPosition;
		float			fRange;

		//스포트라이트
		float			fInnerCone;
		float			fOuterCone;
	}LIGHT_DESC;

	//그림자 광원 데이터
	typedef struct tagShadowDesc
	{
		XMFLOAT3 vEye;
		XMFLOAT3 vAt;

		float fFovy;
		float fNear;
		float fFar;
		float fAspect;
	} SHADOW_DESC;

	//맵 데이터
	typedef struct tagMapObjectData
	{
		std::string objectName;
		std::string modelPath;
		XMFLOAT4X4   worldMatrix;
	}MAP_OBJECTDATA;

	//이벤트 데이터
	struct EventData {
		std::string name;
		float fParam{ 0.f };
		void* pParam{ nullptr };	
	};

	//상태머신 상태 함수
	struct StateProc {
		std::function<void(class Object*, class StateMachine*)>                      enter;
		std::function<void(class Object*, class StateMachine*, float)>               update;
		std::function<void(class Object*, class StateMachine*)>                      exit;
		std::function<bool(class Object*, class StateMachine*, const EventData&)>    onEvent;
	};

	//상태머신 전이 데이터
	struct Transition {
		std::function<bool(class Object* pOwner, class StateMachine* stateMachine)> condition;
		std::string nextState;
		unsigned int priority = 0;
		bool restartFlag = false;
	};

	//데미지 연산 데이터
	struct DamageDesc {
		float      amount = 0.f;            // 피해량
		FACTION     sourceFaction = FACTION_NEUTRAL;
		class Object* pSource = nullptr;       // 타격 오브젝트
		XMVECTOR     hitPos = XMVectorZero(); // 피격 위치
		XMVECTOR     hitDir = XMVectorZero(); // 피격 시 방향
	};

	//스포너 몹 데이터
	typedef struct tagSpawnerMobDesc {
		_uint iSceneId;
		_wstring prototypeKey;
		_wstring layerKey;
		_float3 position;
		_float3 spawnRandomRange = { 0.f,0.f,0.f };
	}SPAWNER_MOB_DESC;

	//트리거박스 데이터
	typedef struct tagTriggerBoxDesc
	{
		_float3 center;
		_float3 Extents;
	}TRIGGERBOX_DESC;

	//UI용 데이터 구조
	typedef struct tagUIDesc
	{
		//트랜스폼 초기화용
		_float fSpeedPerSec{};
		_float fRotationPerSec{};
		
		//공통
		UITYPE type = UITYPE::UI_END;
		string     name;
		_float x = 0;
		_float y = 0;
		_float z = 0;
		_float w = 0;
		_float h = 0;
		_bool visible = true;

		//UILabel
		_wstring	font;
		_wstring     text;
		_float fontSize = 32.f;
		_float4 fontColor = {1.f, 1.f, 1.f, 1.f};

		//UIImage
		_wstring     imagePath;

		//UIButton
		_bool enable = true;
	}UI_DESC;

}


#endif // Engine_Struct_h__
