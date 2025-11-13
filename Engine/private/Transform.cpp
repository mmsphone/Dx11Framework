#include "Transform.h"
#include "Shader.h"

Transform::Transform()
	:Component{}
{
}

HRESULT Transform::InitializePrototype()
{
	return S_OK;
}

HRESULT Transform::Initialize(void* pArg)
{
	//월드행렬 항등행렬로 초기화
	XMStoreFloat4x4(&m_WorldMatrix, XMMatrixIdentity());

	//별도 세팅값이 없으면 기본값대로
	if (nullptr == pArg)
		return S_OK;

	//별도 세팅값으로 값 변경
	TRANSFORM_DESC* pDesc = static_cast<TRANSFORM_DESC*>(pArg);
	m_fSpeedPerSec = pDesc->fSpeedPerSec;
	m_fRotationPerSec = pDesc->fRotationPerSec;

	return S_OK;
}

_vector Transform::GetState(MATRIXROW eState) const
{
	return XMLoadFloat4x4(&m_WorldMatrix).r[eState];
}

void Transform::SetState(MATRIXROW eState, _fvector vState)
{
	if (eState == MATRIXROW::MATRIXROW_POSITION)
	{
		XMFLOAT4 pos;
		XMStoreFloat4(&pos, vState);
		if (pos.w == 0.f)
			pos.w = 1.f;
		XMStoreFloat4(reinterpret_cast<_float4*>(&m_WorldMatrix.m[eState]), XMLoadFloat4(&pos));
	}
	else
		XMStoreFloat4(reinterpret_cast<_float4*>(&m_WorldMatrix.m[eState]), vState);
}

_float3 Transform::GetScale() const
{
	return _float3(
		XMVectorGetX(XMVector3Length(GetState(MATRIXROW::MATRIXROW_RIGHT))),
		XMVectorGetX(XMVector3Length(GetState(MATRIXROW::MATRIXROW_UP))),
		XMVectorGetX(XMVector3Length(GetState(MATRIXROW::MATRIXROW_LOOK)))
	);
}

void Transform::SetScale(_float fSizeX, _float fSizeY, _float fSizeZ)
{
	SetState(MATRIXROW::MATRIXROW_RIGHT, XMVector3Normalize(GetState(MATRIXROW::MATRIXROW_RIGHT)) * fSizeX);
	SetState(MATRIXROW::MATRIXROW_UP, XMVector3Normalize(GetState(MATRIXROW::MATRIXROW_UP)) * fSizeY);
	SetState(MATRIXROW::MATRIXROW_LOOK, XMVector3Normalize(GetState(MATRIXROW::MATRIXROW_LOOK)) * fSizeZ);
}

void Transform::SetSpeedPerSec(_float& fSpeed)
{
	m_fSpeedPerSec = fSpeed;
}

_float Transform::GetSpeedPerSec() const
{
	return m_fSpeedPerSec;
}

void Transform::SetRotatePerSec(_float& fRotate)
{
	m_fRotationPerSec = fRotate;
}

_float Transform::GetRotatePerSec() const
{
	return m_fRotationPerSec;
}

const _float4x4* Transform::GetWorldMatrixPtr() const
{
	return &m_WorldMatrix;
}

_matrix Transform::GetWorldMatrixInverse()
{
	return XMMatrixInverse(nullptr, XMLoadFloat4x4(&m_WorldMatrix));
}

void Transform::GoForward(_float fTimeDelta)
{
	//벡터 가져오기
	_vector     vPosition = GetState(MATRIXROW::MATRIXROW_POSITION);
	_vector     vLook = GetState(MATRIXROW::MATRIXROW_LOOK);

	//바라보는 방향으로 Speed * TimeDelta 만큼 좌표 이동
	vPosition += XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	//위치 상태 재설정
	SetState(MATRIXROW::MATRIXROW_POSITION, vPosition);
}

void Transform::GoLeft(_float fTimeDelta)
{
	//벡터 가져오기
	_vector     vPosition = GetState(MATRIXROW::MATRIXROW_POSITION);
	_vector     vRight = GetState(MATRIXROW::MATRIXROW_RIGHT);

	//Right 벡터 반대 방향으로 Speed * TimeDelta 만큼 좌표 이동
	vPosition -= XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;
	
	//위치 상태 재설정
	SetState(MATRIXROW::MATRIXROW_POSITION, vPosition);
}

void Transform::GoRight(_float fTimeDelta)
{
	//벡터 가져오기
	_vector     vPosition = GetState(MATRIXROW::MATRIXROW_POSITION);
	_vector     vRight = GetState(MATRIXROW::MATRIXROW_RIGHT);

	//Right 벡터 방향으로 Speed * TimeDelta 만큼 좌표 이동
	vPosition += XMVector3Normalize(vRight) * m_fSpeedPerSec * fTimeDelta;

	//위치 상태 재설정
	SetState(MATRIXROW::MATRIXROW_POSITION, vPosition);
}

void Transform::GoBackward(_float fTimeDelta)
{
	//벡터 가져오기
	_vector     vPosition = GetState(MATRIXROW::MATRIXROW_POSITION);
	_vector     vLook = GetState(MATRIXROW::MATRIXROW_LOOK);

	//바라보는 반대 방향으로 Speed * TimeDelta 만큼 좌표 이동
	vPosition -= XMVector3Normalize(vLook) * m_fSpeedPerSec * fTimeDelta;

	//위치 상태 재설정
	SetState(MATRIXROW::MATRIXROW_POSITION, vPosition);
}

void Transform::GoUp(_float fTimeDelta)
{
	_vector vPos = GetState(MATRIXROW::MATRIXROW_POSITION);
	_vector vUp = GetState(MATRIXROW::MATRIXROW_UP);

	vPos += XMVector3Normalize(vUp) * m_fSpeedPerSec * fTimeDelta;

	SetState(MATRIXROW::MATRIXROW_POSITION, vPos);
}

void Transform::GoDown(_float fTimeDelta)
{
	_vector vPos = GetState(MATRIXROW::MATRIXROW_POSITION);
	_vector vUp = GetState(MATRIXROW::MATRIXROW_UP);

	vPos -= XMVector3Normalize(vUp) * m_fSpeedPerSec * fTimeDelta;

	SetState(MATRIXROW::MATRIXROW_POSITION, vPos);
}

void Transform::RotateRadian(_fvector vAxis, _float fRadian)
{
	//현재 크기 
	_float3 vScale = GetScale();

	//현재 크기 반영 Right,Up,Look 벡터 생성
	_vector vRight = XMVectorSet(1.f, 0.f, 0.f, 0.f) * vScale.x;
	_vector vUp = XMVectorSet(0.f, 1.f, 0.f, 0.f) * vScale.y;
	_vector vLook = XMVectorSet(0.f, 0.f, 1.f, 0.f) * vScale.z;

	//회전 행렬 연산
	_matrix RotationMatrix = XMMatrixRotationAxis(vAxis, fRadian);

	vRight = XMVector3TransformNormal(vRight, RotationMatrix);
	vUp = XMVector3TransformNormal(vUp, RotationMatrix);
	vLook = XMVector3TransformNormal(vLook, RotationMatrix);

	//상태 행렬 재설정
	SetState(MATRIXROW::MATRIXROW_RIGHT, vRight);
	SetState(MATRIXROW::MATRIXROW_UP, vUp);
	SetState(MATRIXROW::MATRIXROW_LOOK, vLook);
}

void Transform::RotateTimeDelta(_fvector vAxis, _float fTimeDelta)
{
	//현재 행렬 상태 가져오기
	_vector vRight = GetState(MATRIXROW::MATRIXROW_RIGHT);
	_vector vUp = GetState(MATRIXROW::MATRIXROW_UP);
	_vector vLook = GetState(MATRIXROW::MATRIXROW_LOOK);

	//회전 행렬 연산
	_matrix RotationMatrix = XMMatrixRotationAxis(vAxis, m_fRotationPerSec * fTimeDelta);

	vRight = XMVector3TransformNormal(vRight, RotationMatrix);
	vUp = XMVector3TransformNormal(vUp, RotationMatrix);
	vLook = XMVector3TransformNormal(vLook, RotationMatrix);

	//상태 행렬 재설정
	SetState(MATRIXROW::MATRIXROW_RIGHT, vRight);
	SetState(MATRIXROW::MATRIXROW_UP, vUp);
	SetState(MATRIXROW::MATRIXROW_LOOK, vLook);
}

void Transform::Turn(_fvector vAxis, _float fTimeDelta)
{
	_vector vRight = GetState(MATRIXROW::MATRIXROW_RIGHT);
	_vector vUp = GetState(MATRIXROW::MATRIXROW_UP);
	_vector vLook = GetState(MATRIXROW::MATRIXROW_LOOK);

	_matrix RotationMatrix = XMMatrixRotationAxis(vAxis, m_fRotationPerSec * fTimeDelta);

	vRight = XMVector3TransformNormal(vRight, RotationMatrix);
	vUp = XMVector3TransformNormal(vUp, RotationMatrix);
	vLook = XMVector3TransformNormal(vLook, RotationMatrix);

	SetState(MATRIXROW::MATRIXROW_RIGHT, vRight);
	SetState(MATRIXROW::MATRIXROW_UP, vUp);
	SetState(MATRIXROW::MATRIXROW_LOOK, vLook);
}

void Transform::LookAt(_fvector vFocus)
{
	//목표 좌표 - 현재 좌표 -> Look 벡터
	_vector     vLook = vFocus - GetState(MATRIXROW::MATRIXROW_POSITION);
	//Look 벡터를 y축과 외적 -> Right 벡터
	_vector     vRight = XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook);
	//Look 벡터와 Right벡터를 외적 -> Up 벡터
	_vector     vUp = XMVector3Cross(vLook, vRight);

	//현재 크기를 곱해 상태 행렬 재설정
	_float3     vScale = GetScale();

	SetState(MATRIXROW::MATRIXROW_RIGHT, XMVector3Normalize(vRight) * vScale.x);
	SetState(MATRIXROW::MATRIXROW_UP, XMVector3Normalize(vUp) * vScale.y);
	SetState(MATRIXROW::MATRIXROW_LOOK, XMVector3Normalize(vLook) * vScale.z);
}

void Transform::Chase(_fvector vDest, _float fTimeDelta, _float fLimitDistance)
{
	//현재 위치
	_vector     vPosition = GetState(MATRIXROW::MATRIXROW_POSITION);
	//목표 위치 - 현재 위치 -> 이동 방향 벡터
	_vector     vMoveDir = vDest - vPosition;
	// 사이 거리
	_float      fDistance = XMVectorGetX(XMVector3Length(vMoveDir));

	//이동필요 없을 정도 거리값 이내로 들어오면 이동 X
	if (fDistance < fLimitDistance)
		return;

	//이동 방향 벡터 방향으로 이동
	vPosition += XMVector3Normalize(vMoveDir) * m_fSpeedPerSec * fTimeDelta;

	//위치 상태 재설정
	SetState(MATRIXROW::MATRIXROW_POSITION, vPosition);
}

void Transform::Translate(_fvector vTranslateDir, _float fTimeDelta)
{
	_vector     vPosition = GetState(MATRIXROW::MATRIXROW_POSITION);
	vPosition += XMVector3Normalize(vTranslateDir) * m_fSpeedPerSec * fTimeDelta;

	SetState(MATRIXROW::MATRIXROW_POSITION, vPosition);
}

HRESULT Transform::BindRenderTargetShaderResource(Shader* pShader, const _char* pConstantName)
{
	return pShader->BindMatrix(pConstantName, &m_WorldMatrix);
}

Transform* Transform::Create()
{
	Transform* pInstance = new Transform();

	if (FAILED(pInstance->InitializePrototype()))
	{
		MSG_BOX("Failed to Created : Transform");
		SafeRelease(pInstance);
	}

	return pInstance;
}
Component* Transform::Clone(void* pArg)
{
	return nullptr;
}

void Transform::Free()
{
	__super::Free();
}
