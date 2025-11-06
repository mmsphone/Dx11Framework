#include "AIController.h"

#include "EngineUtility.h"

#include "Object.h"

AIController::AIController()
	:Component{}
{
}

AIController::AIController(const AIController& Prototype)
	: Component{ Prototype }
	, m_aiControllerDesc{ Prototype.m_aiControllerDesc }
	, m_ownerDesc{ Prototype.m_ownerDesc }
	, m_aiBlackBoard{ Prototype.m_aiBlackBoard }
	, m_time{ Prototype.m_time }
	, m_lastSeenTime{ Prototype.m_lastSeenTime }
{
}

HRESULT AIController::InitializePrototype()
{
	return S_OK;
}

HRESULT AIController::Initialize(void* pArg)
{
	m_time = 0.f;
	m_lastSeenTime = -999.f;
	m_aiBlackBoard = AIBLACKBOARD_DESC{};

	if (pArg == nullptr)
		return S_OK;
	
	m_aiControllerDesc = *static_cast<AICONTROLLER_DESC*>(pArg);

	return S_OK;
}

void AIController::Update(_float fTimeDelta)
{
	m_time += fTimeDelta;
	
	if (!m_pOwner)
		return;

	Transform* pTransform = dynamic_cast<Transform*>(m_pOwner->FindComponent(TEXT("Transform")));
	if (!pTransform && (!m_ownerDesc.getOwnerPos || !m_ownerDesc.getOwnerLook))
		return;

	//1. 인지
	Sense(m_pOwner, pTransform);
	//2. 의사결정
	Decide(m_pOwner, fTimeDelta);
	//3. 액션
	Act(m_pOwner, fTimeDelta);

}

void AIController::BindOwnerDesc(const AI_OWNER_DESC& ownerDesc)
{
	m_ownerDesc = ownerDesc;
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

void AIController::Sense(Object* owner, Transform* pTransform)
{
	//몬스터 위치
	_vector ownerPos{}, ownerLook{};
	if (m_ownerDesc.getOwnerPos)  ownerPos = m_ownerDesc.getOwnerPos();
	if (m_ownerDesc.getOwnerLook) ownerLook = m_ownerDesc.getOwnerLook();
	if (!m_ownerDesc.getOwnerPos || !m_ownerDesc.getOwnerLook) // 위치 데이터를 못가져오면
	{
		// Transform에서 보충
		if (pTransform)
		{
			if (!m_ownerDesc.getOwnerPos)  ownerPos = pTransform->GetState(POSITION);
			if (!m_ownerDesc.getOwnerLook) ownerLook = XMVector3Normalize(pTransform->GetState(LOOK));
		}
	}

	//결과값 초기화
	m_aiBlackBoard = {};
	XMStoreFloat4(&m_aiBlackBoard.ownerPos, ownerPos);
	XMStoreFloat4(&m_aiBlackBoard.ownerLook, XMVector3Normalize(ownerLook));

	//타겟(플레이어) 위치
	_vector targetPos{};
	_bool   hasTarget = false;
	if (m_pEngineUtility)
	{
		Object* pPlayer = m_pEngineUtility->FindObject(m_pEngineUtility->GetCurrentSceneId(), TEXT("Player"), 0);
		if (pPlayer)
		{
			Transform* pPlayerTransform = dynamic_cast<Transform*>(pPlayer->FindComponent(TEXT("Transform")));
			if (pPlayerTransform)
			{
				targetPos = pPlayerTransform->GetState(POSITION);
				hasTarget = true;
			}
		}
	}

	m_aiBlackBoard.hasTarget = hasTarget;
	XMStoreFloat4(&m_aiBlackBoard.targetPos, targetPos);
	if (!hasTarget) return;

	// 거리
	_vector toTarget = targetPos - ownerPos;
	_float  dist = XMVectorGetX(XMVector3Length(toTarget));
	m_aiBlackBoard.distance = dist;
	m_aiBlackBoard.inAttackRange = (dist <= m_aiControllerDesc.attackRange + 1e-4f);

	// FOV
	_vector dirN = (dist > 1e-6f) ? XMVector3Normalize(toTarget) : XMVectorZero();
	_float  cosAngle = XMVectorGetX(XMVector3Dot(XMLoadFloat4(&m_aiBlackBoard.ownerLook), dirN));
	cosAngle = std::clamp(cosAngle, -1.0f, 1.0f);

	_float  cosHalfFov = cosf(XMConvertToRadians(m_aiControllerDesc.fovDeg * 0.5f));
	m_aiBlackBoard.cosHalfFov = cosHalfFov;
	m_aiBlackBoard.inFOV = (cosAngle >= cosHalfFov);

	// LOS (콜백 없으면 true로 간주)
	_bool los = true;
	if (m_ownerDesc.hasLineOfSight)
		los = m_ownerDesc.hasLineOfSight(ownerPos, targetPos);
	m_aiBlackBoard.hasLOS = los;

	// 마지막으로 본 시간 갱신
	if (los && m_aiBlackBoard.inFOV && dist <= m_aiControllerDesc.sightRange)
		m_lastSeenTime = m_time;
}

void AIController::Decide(Object* owner, _float fTimeDelta)
{
	
}

void AIController::Act(Object* owner, _float fTimeDelta)
{
	if (!m_ownerDesc.applyInput)
		return;

	// 1) 추적 유지 조건: 방금 본 상태거나, 마지막으로 본 후 chaseKeepSec 이내
	const _bool recentlySeen = (m_time - m_lastSeenTime) <= m_aiControllerDesc.chaseKeepSec;
	const _bool canChase = m_aiBlackBoard.hasTarget &&
		( (m_aiBlackBoard.hasLOS && m_aiBlackBoard.inFOV && m_aiBlackBoard.distance <= m_aiControllerDesc.sightRange) || recentlySeen);

	if (!canChase)
	{
		// 대기
		m_ownerDesc.applyInput(false, XMVectorZero(), false);
		return;
	}

	if (m_aiBlackBoard.inAttackRange)
	{
		// 공격 사거리면 정지(여기선 입력만 전달; 실제 공격은 상위 오브젝트가 StateMachine 등으로 처리)
		m_ownerDesc.applyInput(false, XMVectorZero(), false);
		return;
	}

	// 이동: 타겟 방향으로 전진
	_vector moveDir = XMVectorZero();
	if (m_aiBlackBoard.distance > 1e-5f)
		moveDir = XMVector3Normalize(XMLoadFloat4(&m_aiBlackBoard.targetPos) - XMLoadFloat4(&m_aiBlackBoard.ownerPos));

	// 지면 투영(옵션): 필요하면 Y=0 고정
	moveDir = XMVectorSetY(moveDir, 0.f);
	if (!XMVector3Equal(moveDir, XMVectorZero()))
		moveDir = XMVector3Normalize(moveDir);

	// 스프린트 여부는 판단 규칙에 따라 변경 가능(지금은 FOV+가까움이면 스프린트 예시)
	const _bool sprint = (m_aiBlackBoard.inFOV && m_aiBlackBoard.distance > (m_aiControllerDesc.attackRange * 1.5f));

	m_ownerDesc.applyInput(true, moveDir, sprint);
}
