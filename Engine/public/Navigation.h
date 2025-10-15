#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL Navigation final : public Component
{
public:
	typedef struct tagNavigationDesc
	{
		_int			iCurrentCellIndex = {-1};
	}NAVIGATION_DESC;
private:	
	Navigation();
	Navigation(const Navigation& Prototype);
	virtual ~Navigation() = default;

public:
	virtual HRESULT InitializePrototype(const _tchar* pNavigationDataFile);
	virtual HRESULT Initialize(void* pArg) override;

	void Update(const _float4x4* pParentMatrix);
	_bool isMove(_fvector vResultPos);
	_vector SetOnNavigation(_fvector vWorldPos);

	const vector<class Cell*>& GetCells();
	void AddCell(_float3* pPoints);
	void RemoveRecentCell();
	void ClearCells();
	void SaveCells(const _tchar* pNavigationDataFile);

	static Navigation* Create(const _tchar* pNavigationDataFile);
	virtual Component* Clone(void* pArg) override;
	virtual void Free() override;
#ifdef _DEBUG
	HRESULT Render();
#endif

private:
	void SetNeighbors();
private:
	_int					m_iCurrentCellIndex = { -1 };
	vector<class Cell*>	m_Cells;
	static const _float4x4* m_pParentMatrix;
#ifdef _DEBUG
	class Shader* m_pShader = { nullptr };
#endif
};

NS_END