#pragma once

#include "UI.h"

NS_BEGIN(Engine)

class ENGINE_DLL UIButton final : public UI
{
    UIButton();
    UIButton(const UIButton& Prototype);
    virtual ~UIButton() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    PriorityUpdate(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    void SetImagePath(const std::wstring& path);
    const wstring& GetImagePath() const;

    void AddButtonFunction(function<void()> func);
    void DoButtonFunctions();
    void ClearButtonFunctions();

    static UIButton* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void     Free() override;

private:
    HRESULT ReadyComponents();
    HRESULT LoadTextureFromPath();

private:
    class Texture* m_pTexture = nullptr;
    vector<function<void()>> buttonFunctions{};
};

NS_END
