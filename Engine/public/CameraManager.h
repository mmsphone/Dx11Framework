#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class ENGINE_DLL CameraManager final : public Base
{
private:
    CameraManager();
    virtual ~CameraManager() = default;

public:
    HRESULT Initialize();

    // 매 프레임 호출
    void Update(_float fTimeDelta);

    // 카메라 등록/해제
    void RegisterCamera(class Camera* pCamera);
    void UnregisterCamera(class Camera* pCamera);

    // 메인 카메라 선택
    void SetMainCamera(class Camera* pCamera);
    class Camera* GetMainCamera() const;

    // 폭발 등에서 쉐이크 요청
    void RequestCameraShake(const CAMERA_SHAKE_DESC& desc);

    static CameraManager* Create();
    virtual void Free() override;

private:
    _matrix ApplyShakeToView(_fmatrix baseView, _float fTimeDelta);

private:
    class EngineUtility* m_pEngineUtility = { nullptr };
    vector<class Camera*> m_Cameras;
    class Camera* m_pMainCamera = nullptr;

    // 쉐이크 상태
    _bool             m_bShakeActive = false;
    CAMERA_SHAKE_DESC m_ShakeDesc{};
    _float            m_fShakeElapsed = 0.f;
};

NS_END
