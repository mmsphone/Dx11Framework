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
    void SetRatio(const _float hpRatio);
    void ClearMasking();

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
};

NS_END
