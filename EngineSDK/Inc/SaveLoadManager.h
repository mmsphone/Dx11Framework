#pragma once
#include "Base.h"
#include "NoAssimpModelStruct.h"

NS_BEGIN(Engine)

class SaveLoadManager final : public Base
{
	SaveLoadManager();
	virtual ~SaveLoadManager() = default;

public:
	ModelData* LoadNoAssimpModel(const _char* pFilePath);
	std::vector<MAP_OBJECTDATA> LoadMapData(const std::string& path);
	HRESULT SaveMapData(const std::string& path);

	HRESULT SaveLights(const std::string& path);
	HRESULT ReadyLightsFromFile(const std::string& path);

	static SaveLoadManager* Create();
	virtual void Free() override;

private:
	class EngineUtility* m_pEngineUtility = { nullptr };
};

NS_END