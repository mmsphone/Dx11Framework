#include "AIController.h"

#include "EngineUtility.h"

#include "Object.h"

AIController::AIController()
	:Component{}
	, m_lastInput{}
	, m_lastOutput{}
	, m_process{}
	, m_time(0.f)
{
}

AIController::AIController(const AIController& Prototype)
	: Component{ Prototype }
{
}

HRESULT AIController::InitializePrototype()
{
	return S_OK;
}

HRESULT AIController::Initialize(void* pArg)
{
	m_time = 0.f;
	m_lastInput.Clear();
	m_lastOutput.Clear();

	return S_OK;
}

void AIController::Update(_float fTimeDelta)
{
	m_time += fTimeDelta;
	
	//1. 인지
	if (m_process.sense)
		m_process.sense(m_lastInput, fTimeDelta, m_time);
	//2. 의사결정
	AIOUTPUT_DESC out;
	if (m_process.decide)
		m_process.decide(m_lastInput, out, fTimeDelta, m_time);
	//3. 액션
	if (m_process.act)
		m_process.act(out, fTimeDelta, m_time);
	//4. 결과 저장
	m_lastOutput = std::move(out);
	//5. 결과 적용
	if (m_process.applyOutput)
		m_process.applyOutput(m_lastOutput);

}

void AIController::BindProcessDesc(const AIPROCESS_DESC& processDesc)
{
	m_process = processDesc;
}

void AIController::SetInput(const AIINPUT_DESC& inputDesc)
{
	m_lastInput = inputDesc;
}

const AIOUTPUT_DESC& AIController::GetLastOutput() const
{
	return m_lastOutput;
}

_float AIController::GetTime() const
{
	return m_time;
}

AIController* AIController::Create()
{
	AIController* pInstance = new AIController();

	if (FAILED(pInstance->InitializePrototype()))
	{
		MSG_BOX("Failed to Created : AIController");
		SafeRelease(pInstance);
	}

	return pInstance;
}

Component* AIController::Clone(void* pArg)
{
	AIController* pInstance = new AIController(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : AIController");
		SafeRelease(pInstance);
	}

	return pInstance;
}

void AIController::Free()
{
	__super::Free();
}
