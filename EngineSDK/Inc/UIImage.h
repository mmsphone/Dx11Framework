#pragma once

#include "UI.h"

NS_BEGIN(Engine)

class ENGINE_DLL UIImage final : public UI
{
private:
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

    static UIImage* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void    Free() override;

public:
    void               SetImagePath(const std::string& path);
    const std::string& GetImagePath() const { return m_imagePath; }

private:
    HRESULT ReadyComponents();
    HRESULT LoadTextureFromKey();

private:
    std::string m_imagePath = {};
    class Texture* m_pTexture = { nullptr };
};

NS_END
