#include "TriggerBox.h"

#include "EngineUtility.h"
#include "CollisionBoxAABB.h"
#include "Object.h"

TriggerBox::TriggerBox()
	:m_pEngineUtility{ EngineUtility::GetInstance()}
{
	SafeAddRef(m_pEngineUtility);
}

HRESULT TriggerBox::Initialize(void* pArg)
{
	if (pArg == nullptr)
		return E_FAIL;

	TRIGGERBOX_DESC* pDesc = static_cast<TRIGGERBOX_DESC*>(pArg);
	m_desc = *pDesc;

	CollisionBoxAABB::COLLISIONAABB_DESC desc{};
	desc.vCenter = pDesc->center;
	desc.vSize = pDesc->Extents;

	m_pCollisionBoxAABB = CollisionBoxAABB::Create(&desc);
	if (m_pCollisionBoxAABB == nullptr)
		return E_FAIL;

	isTrigger = false;
	m_TriggerFunction = nullptr;

#ifdef _DEBUG
	m_pBatch = new PrimitiveBatch<VertexPositionColor>(m_pEngineUtility->GetContext());
	m_pEffect = new BasicEffect(m_pEngineUtility->GetDevice());
	m_pEffect->SetVertexColorEnabled(true);

	const void* pShaderByteCode = { nullptr };
	size_t      iShaderByteCodeLength = {};
	m_pEffect->GetVertexShaderBytecode(&pShaderByteCode, &iShaderByteCodeLength);

	if (FAILED(m_pEngineUtility->GetDevice()->CreateInputLayout(VertexPositionColor::InputElements, VertexPositionColor::InputElementCount, pShaderByteCode, iShaderByteCodeLength, &m_pInputLayout)))
		return E_FAIL;
#endif

	return S_OK;
}

void TriggerBox::SetTriggerFunction(const function<void()>& func)
{
	m_TriggerFunction = func;
}

void TriggerBox::UpdateTrigger()
{
	if (isTrigger)
		return;
	if (m_pCollisionBoxAABB == nullptr)
		return;

	_uint iCurSceneId = m_pEngineUtility->GetCurrentSceneId();
	Object* pPlayer = m_pEngineUtility->FindObject(iCurSceneId, TEXT("Player"), 0);
	if (pPlayer == nullptr)
		return;

	Collision* pPlayerCollision = static_cast<Collision*>(pPlayer->FindComponent(TEXT("Collision")));
	if (m_pCollisionBoxAABB->Intersect(pPlayerCollision))
	{
		isTrigger = true;

		if (m_TriggerFunction)
			m_TriggerFunction();
	}
}

HRESULT TriggerBox::RenderTriggerBox()
{
	auto pContext = m_pEngineUtility->GetContext();
	ID3D11GeometryShader* pOldGS = nullptr;
	pContext->GSGetShader(&pOldGS, nullptr, 0);
	pContext->GSSetShader(nullptr, nullptr, 0);

	//충돌체 랜더링
	m_pEffect->SetWorld(XMMatrixIdentity());
	m_pEffect->SetView(m_pEngineUtility->GetTransformMatrix(D3DTS::D3DTS_VIEW));
	m_pEffect->SetProjection(m_pEngineUtility->GetTransformMatrix(D3DTS::D3DTS_PROJECTION));

	m_pEffect->Apply(m_pEngineUtility->GetContext());

	m_pEngineUtility->GetContext()->IASetInputLayout(m_pInputLayout);

	m_pBatch->Begin();

	m_pCollisionBoxAABB->Render(m_pBatch, XMVectorSet(0.f, 1.f, 0.f, 1.f));

	m_pBatch->End();

	//GS 다시 세팅
	pContext->GSSetShader(pOldGS, nullptr, 0);
	SafeRelease(pOldGS);

	return S_OK;
}

const TRIGGERBOX_DESC& TriggerBox::GetTriggerBoxDesc()
{
	return m_desc;
}

void TriggerBox::UpdateFromDesc(const TRIGGERBOX_DESC& desc)
{
	m_desc = desc;

	CollisionBoxAABB::COLLISIONAABB_DESC aabbDesc{};
	aabbDesc.vCenter = desc.center;
	aabbDesc.vSize = desc.Extents;

	SafeRelease(m_pCollisionBoxAABB);
	m_pCollisionBoxAABB = CollisionBoxAABB::Create(&aabbDesc);
}

TriggerBox* TriggerBox::Create(void* pArg)
{
	TriggerBox* pInstance = new TriggerBox{};
	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Create : TriggerBox");
		SafeRelease(pInstance);
		return nullptr;
	}
	return pInstance;
}

void TriggerBox::Free()
{
	__super::Free();

	SafeRelease(m_pCollisionBoxAABB);
	SafeRelease(m_pEngineUtility);
	m_TriggerFunction = nullptr;

#ifdef _DEBUG
	SafeDelete(m_pEffect);
	SafeDelete(m_pBatch);
	SafeRelease(m_pInputLayout);
#endif
}