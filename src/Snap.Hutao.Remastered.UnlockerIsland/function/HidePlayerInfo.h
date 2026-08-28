#pragma once

#include "IFunction.h"
#include "FunctionType.h"

class HidePlayerInfo : public IFunction
{
public:
    void Initialize() override;
    void OnUpdate() override;
    void* GetHookFunction() override { return nullptr; }
    bool IsEnabled() override;
    void SetEnabled(bool enabled) override { (void)enabled; }
    FunctionType GetFunctionType() override { return FunctionType::HIDE_PLAYER_INFO; }

    static bool FilterSetActive(void* object, bool active);
    static void NotifyUidChanged();

private:
    static HidePlayerInfo* s_instance;

    ULONGLONG m_lastExecuteTime = 0;
    void* m_profileLayer = nullptr;
    void* m_cachedUid = nullptr;
    void* m_cachedProfileUid = nullptr;
    void* m_cachedProfileName = nullptr;
    unsigned int m_uidResolveAttempts = 0;
    unsigned int m_profileUidResolveAttempts = 0;
    unsigned int m_profileNameResolveAttempts = 0;
    bool m_profilePageActive = false;
    bool m_resolveProfileUidNext = true;

    static constexpr ULONGLONG UPDATE_INTERVAL_MS = 200;
    static constexpr unsigned int MAX_RESOLVE_ATTEMPTS = 10;
};
