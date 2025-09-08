#ifndef Engine_Struct_h__
#define Engine_Struct_h__

namespace Engine
{
	typedef struct tagEngineDesc 
	{
		HWND			hWnd;
		
		WINMODE			eWinMode;
		unsigned int	iWinSizeX, iWinSizeY;
		unsigned int	iNumLevels;
	}ENGINE_DESC;

	typedef struct tagVertexPostion
	{
		XMFLOAT3		vPosition;
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



	

}


#endif // Engine_Struct_h__
