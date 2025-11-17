#pragma once

#include "UI.h"

NS_BEGIN(Engine)

class ENGINE_DLL UIButton final : public UI
{
private:
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

    static UIButton* Create();
    virtual Object* Clone(void* pArg) override;
    virtual void     Free() override;

public:
    // SaveLoadManager → BuildUIFromRes에서 세팅할 용도
    void                SetText(const std::string& text) { m_text = text; }
    void                SetCommand(const std::string& cmd) { m_command = cmd; }

    const std::string& GetText()    const { return m_text; }
    const std::string& GetCommand() const { return m_command; }

    // 필요하면 나중에 버튼 상태 (hover, pressed 등) 추가 가능
    // void SetHover(bool h) { m_bHover = h; }

private:
    HRESULT ReadyComponents();

private:
    std::string m_text;
    std::string m_command;
};

NS_END
