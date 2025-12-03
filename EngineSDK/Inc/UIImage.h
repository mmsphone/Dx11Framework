#pragma once

#include "UI.h"

NS_BEGIN(Engine)

class ENGINE_DLL UIImage final : public UI
{
    UIImage();
    UIImage(const UIImage& Prototype);
    virtual ~UIImage() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    PriorityUpdate(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    void SetScissor(const D3D11_RECT& rect);
    void SetHPRatio(const _float hpRatio);
    void SetLoadingRatio(const _float ratio);
    void SetBulletRatio(const _float ratio);
    void SetMaskingColor(const _float4 vColor);
    void SetMaskingColorGradient(const _float4 vColor);
    void ClearMasking();

    void SetAlpha(_float alpha);
    void ResetAlpha();

    void SetBrightness(_float brightness);
    void SetGamma(_float gamma);
    void ResetCustomColor();

    void SetRotationRad(_float rad);

    static UIImage* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void    Free() override;

private:
    HRESULT ReadyComponents();
    HRESULT LoadTextureFromPath();

private:
    class Texture* m_pTexture = { nullptr };

    _int       m_MaskingType = 0;
    D3D11_RECT  m_scissorRect{};
    _float m_Ratio = {};

    _bool m_useCustomAlpha = false;
    _float m_customAlpha = 1.f;

    _bool m_useCustomColor = false;
    _float m_customGamma = 1.f;
    _float m_customBrightness = 1.f;
    _float4 m_vCustomColor = {};
};

NS_END
