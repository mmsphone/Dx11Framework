#include "Player.h"

#include "PlayerBody.h"
#include "PlayerWeapon.h"

Player::Player()
    : Container{  }
{
}

Player::Player(const Player& Prototype)
    : Container{ Prototype }
{
}

HRESULT Player::InitializePrototype()
{
    return S_OK;
}

HRESULT Player::Initialize(void* pArg)
{
    Container::CONTAINER_DESC     Desc{};

    Desc.fSpeedPerSec = 10.f;
    Desc.fRotationPerSec = XMConvertToRadians(180.0f);
    Desc.iNumParts = PART::PART_END;

    if (FAILED(__super::Initialize(&Desc)))
        return E_FAIL;

    if (FAILED(ReadyComponents()))
        return E_FAIL;

    if (FAILED(ReadyParts()))
        return E_FAIL;

    return S_OK;
}

void Player::PriorityUpdate(_float fTimeDelta)
{
    for (auto& pPartObj : m_Parts)
    {
        if (nullptr != pPartObj)
            pPartObj->PriorityUpdate(fTimeDelta);
    }
}

void Player::Update(_float fTimeDelta)
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));

    if (GetKeyState(VK_LEFT) & 0x8000)
    {
        pTransform->RotateTimeDelta(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta * -1.f);
    }

    if (GetKeyState(VK_RIGHT) & 0x8000)
    {
        pTransform->RotateTimeDelta(XMVectorSet(0.f, 1.f, 0.f, 0.f), fTimeDelta);
    }

    if (GetKeyState(VK_DOWN) & 0x8000)
    {
        pTransform->GoBackward(fTimeDelta);
    }

    if (GetKeyState(VK_UP) & 0x8000)
    {
        pTransform->GoForward(fTimeDelta);

        if (m_iState & STATE::IDLE)
            m_iState ^= STATE::IDLE;

        m_iState |= STATE::RUN;
    }
    else
    {
        m_iState = STATE::IDLE;
    }

    for (auto& pPartObj : m_Parts)
    {
        if (nullptr != pPartObj)
            pPartObj->Update(fTimeDelta);
    }
}

void Player::LateUpdate(_float fTimeDelta)
{
    for (auto& pPartObj : m_Parts)
    {
        if (nullptr != pPartObj)
            pPartObj->LateUpdate(fTimeDelta);
    }
}

HRESULT Player::Render()
{
    return S_OK;
}

HRESULT Player::ReadyComponents()
{
    return S_OK;
}

HRESULT Player::ReadyParts()
{
    Transform* pTransform = dynamic_cast<Transform*>(FindComponent(TEXT("Transform")));
    PlayerBody::PLAYERBODY_DESC        BodyDesc{};
    BodyDesc.pParentMatrix = pTransform->GetWorldMatrixPtr();
    BodyDesc.pParentState = &m_iState;
    if (FAILED(__super::AddPart(SCENE::GAMEPLAY, TEXT("Prototype_GameObject_Player_Body"), PART::BODY, &BodyDesc)))
        return E_FAIL;

    PlayerWeapon::PLAYERWEAPON_DESC        WeaponDesc{};
    WeaponDesc.pParentMatrix = pTransform->GetWorldMatrixPtr();
    WeaponDesc.pParentState = &m_iState;
    WeaponDesc.pSocketBoneMatrix = dynamic_cast<PlayerBody*>(m_Parts[PART::BODY])->GetSocketBoneMatrixPtr("SWORD");

    if (FAILED(__super::AddPart(SCENE::GAMEPLAY, TEXT("Prototype_GameObject_Player_Weapon"), PART::WEAPON, &WeaponDesc)))
        return E_FAIL;

    return S_OK;
}

Object* Player::Create()
{
    Player* pInstance = new Player();

    if (FAILED(pInstance->InitializePrototype()))
    {
        MSG_BOX("Failed to Created : Player");
        SafeRelease(pInstance);
    }

    return pInstance;
}


Object* Player::Clone(void* pArg)
{
    Player* pInstance = new Player(*this);

    if (FAILED(pInstance->Initialize(pArg)))
    {
        MSG_BOX("Failed to Cloned : Player");
        SafeRelease(pInstance);
    }
    return pInstance;
}

void Player::Free()
{
    __super::Free();
}
