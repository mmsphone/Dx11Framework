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

    void SetEnable(_bool bEnable);
    _bool IsEnable();

    void AddButtonFunction(function<void()> func);
    void DoButtonFunctions();
    void ClearButtonFunctions();

    void SetDefaultImage(const wstring& defaultKey);
    void SetOnImage(const wstring& onKey);
    void SetText(const wstring& textKey);
    _bool IsMouseOver() const;

    static UIButton* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void     Free() override;

private:
    HRESULT ReadyComponents();

private:
    UI* m_text = { nullptr };
    UI* m_defaultImage = { nullptr };
    UI* m_onImage = { nullptr };
    vector<function<void()>> buttonFunctions{};
};

NS_END
