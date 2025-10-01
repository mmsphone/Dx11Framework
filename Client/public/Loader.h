#pragma once

#include "Client_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class EngineUtility;
NS_END

NS_BEGIN(Client)

class Loader final : public Base
{
private:
	Loader();
	virtual ~Loader() = default;

public:
	HRESULT Initialize(SCENE eNextSceneId);

	HRESULT Loading();
	_bool isFinished() const;
	void PrintText();

	static Loader* Create(SCENE eNextLevelID);
	virtual void Free() override;
private:
	HRESULT LoadingForLogo();
	HRESULT LoadingForGamePlay();
private:
	EngineUtility* m_pEngineUtility = { nullptr };

private:
	_bool						m_isFinished = { false };
	SCENE						m_eNextLevelID = {};
	HANDLE						m_hThread = {};
	CRITICAL_SECTION			m_CriticalSection = {};
	_tchar						m_szLoading[128] = {};

};

NS_END