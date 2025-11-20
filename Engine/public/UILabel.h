#pragma once

#include "UI.h"

NS_BEGIN(Engine)

class ENGINE_DLL UILabel final : public UI
{
private:
    UILabel();
    UILabel(const UILabel& Prototype);
    virtual ~UILabel() = default;

public:
    virtual HRESULT InitializePrototype() override;
    virtual HRESULT Initialize(void* pArg) override;
    virtual void    PriorityUpdate(_float fTimeDelta) override;
    virtual void    Update(_float fTimeDelta) override;
    virtual void    LateUpdate(_float fTimeDelta) override;
    virtual HRESULT Render() override;

    static UILabel* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void    Free() override;

public:
    void SetText(const wstring& text);
    const wstring& GetText() const;
};

NS_END
