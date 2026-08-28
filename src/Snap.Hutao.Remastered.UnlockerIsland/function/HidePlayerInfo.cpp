#include "HidePlayerInfo.h"
#include "../framework.h"
#include "../dllmain.h"
#include "../Constants.h"
#include "HooksShared.h"

typedef Il2CppString* (*FindStringFn)(const char*);
typedef void* (*FindGameObjectFn)(void*);
typedef void(*SetActiveFn)(void*, bool);
typedef Il2CppString* (*GetNameFn)(void*);

HidePlayerInfo* HidePlayerInfo::s_instance = nullptr;

static void* FindObjectByPath(
    const char* path,
    FindStringFn findStringFunc,
    FindGameObjectFn findGameObjectFunc)
{
    Il2CppString* pathString = findStringFunc(path);
    return pathString ? findGameObjectFunc(pathString) : nullptr;
}

static void ForceHideCachedObject(
    void*& cachedObject,
    SetActiveFn setActiveFunc)
{
    if (!cachedObject)
    {
        return;
    }

    __try
    {
        // The active state of these UI objects does not always reflect
        // whether their text is currently rendered, so hide once directly.
        setActiveFunc(cachedObject, false);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        cachedObject = nullptr;
    }
}

void HidePlayerInfo::Initialize()
{
    s_instance = this;
    m_uidResolveAttempts = MAX_RESOLVE_ATTEMPTS;

    if (g_pEnv->Offsets.FindString)
    {
        findString = GetFunctionAddress(g_pEnv->Offsets.FindString);
    }

    if (g_pEnv->Offsets.FindObject)
    {
        findGameObject = GetFunctionAddress(g_pEnv->Offsets.FindObject);
    }

    if (g_pEnv->Offsets.ObjectActive)
    {
        if (!setActive)
        {
            setActive = GetFunctionAddress(g_pEnv->Offsets.ObjectActive);
        }
    }

}

bool HidePlayerInfo::FilterSetActive(void* object, bool active)
{
    HidePlayerInfo* instance = s_instance;
    if (!instance || !g_pEnv->HidePlayerInfo || !object)
    {
        return false;
    }

    if (active &&
        (object == instance->m_cachedUid ||
         object == instance->m_cachedProfileUid ||
         object == instance->m_cachedProfileName))
    {
        return true;
    }

    if (object == instance->m_profileLayer)
    {
        instance->m_profilePageActive = active;
        if (active)
        {
            instance->m_cachedProfileUid = nullptr;
            instance->m_cachedProfileName = nullptr;
            instance->m_profileUidResolveAttempts = MAX_RESOLVE_ATTEMPTS;
            instance->m_profileNameResolveAttempts = MAX_RESOLVE_ATTEMPTS;
            instance->m_resolveProfileUidNext = true;
        }
        else
        {
            instance->m_cachedProfileUid = nullptr;
            instance->m_cachedProfileName = nullptr;
            instance->m_profileUidResolveAttempts = 0;
            instance->m_profileNameResolveAttempts = 0;
        }
        return false;
    }

    // Unknown inactive objects do not need name inspection. The cached profile
    // root was handled above, and target activation is filtered directly.
    if (!active)
    {
        return false;
    }

    if (!getName)
    {
        return false;
    }

    GetNameFn getNameFunc = (GetNameFn)getName;
    Il2CppString* name = getNameFunc(object);
    if (!name)
    {
        return false;
    }

    if (wcscmp(name->chars, L"PlayerProfilePage") == 0)
    {
        instance->m_profileLayer = object;
        instance->m_profilePageActive = active;
        instance->m_cachedProfileUid = nullptr;
        instance->m_cachedProfileName = nullptr;
        instance->m_profileUidResolveAttempts = active ? MAX_RESOLVE_ATTEMPTS : 0;
        instance->m_profileNameResolveAttempts = active ? MAX_RESOLVE_ATTEMPTS : 0;
        instance->m_resolveProfileUidNext = true;
        return false;
    }

    if (wcscmp(name->chars, L"TxtUID") == 0)
    {
        instance->m_cachedUid = object;
        instance->m_uidResolveAttempts = 0;
        return active;
    }

    if (instance->m_profilePageActive)
    {
        if (wcscmp(name->chars, L"UID") == 0)
        {
            instance->m_cachedProfileUid = object;
            instance->m_profileUidResolveAttempts = 0;
            return active;
        }

        if (wcscmp(name->chars, L"PlayerName") == 0)
        {
            instance->m_cachedProfileName = object;
            instance->m_profileNameResolveAttempts = 0;
            return active;
        }
    }

    return false;
}

void HidePlayerInfo::NotifyUidChanged()
{
    if (s_instance)
    {
        s_instance->m_cachedUid = nullptr;
        s_instance->m_uidResolveAttempts = MAX_RESOLVE_ATTEMPTS;
    }
}

void HidePlayerInfo::OnUpdate()
{
    if (!g_pEnv->HidePlayerInfo)
    {
        return;
    }

    const bool hasPendingUid = !m_cachedUid && m_uidResolveAttempts > 0;
    const bool hasPendingProfileUid =
        m_profilePageActive && !m_cachedProfileUid && m_profileUidResolveAttempts > 0;
    const bool hasPendingProfileName =
        m_profilePageActive && !m_cachedProfileName && m_profileNameResolveAttempts > 0;
    if (!hasPendingUid && !hasPendingProfileUid && !hasPendingProfileName)
    {
        return;
    }

    // Throttle only bounded event-driven resolution. There is no steady-state
    // polling once all pending paths have either resolved or expired.
    ULONGLONG now = GetTickCount64();
    if (now - m_lastExecuteTime < UPDATE_INTERVAL_MS)
    {
        return;
    }
    m_lastExecuteTime = now;

    if (!findString || !findGameObject || !setActive)
    {
        return;
    }

    FindStringFn findStringFunc = (FindStringFn)findString;
    FindGameObjectFn findGameObjectFunc = (FindGameObjectFn)findGameObject;
    SetActiveFn setActiveFunc = (SetActiveFn)setActive;

    // Resolve paths only during a bounded window after the corresponding UI
    // lifecycle event. Steady-state updates never call GameObject.Find.
    if (!m_cachedUid && m_uidResolveAttempts > 0)
    {
        --m_uidResolveAttempts;
        m_cachedUid = FindObjectByPath(UID_PATH, findStringFunc, findGameObjectFunc);
        ForceHideCachedObject(m_cachedUid, setActiveFunc);
        if (m_cachedUid)
        {
            m_uidResolveAttempts = 0;
        }
    }

    if (m_profilePageActive)
    {
        const bool canResolveUid = !m_cachedProfileUid && m_profileUidResolveAttempts > 0;
        const bool canResolveName = !m_cachedProfileName && m_profileNameResolveAttempts > 0;

        if (m_resolveProfileUidNext && canResolveUid)
        {
            --m_profileUidResolveAttempts;
            m_cachedProfileUid = FindObjectByPath(PROFILE_UID_PATH, findStringFunc, findGameObjectFunc);
            ForceHideCachedObject(m_cachedProfileUid, setActiveFunc);
            m_resolveProfileUidNext = false;
        }
        else if (canResolveName)
        {
            --m_profileNameResolveAttempts;
            m_cachedProfileName = FindObjectByPath(PROFILE_NAME_PATH, findStringFunc, findGameObjectFunc);
            ForceHideCachedObject(m_cachedProfileName, setActiveFunc);
            m_resolveProfileUidNext = true;
        }
        else if (canResolveUid)
        {
            --m_profileUidResolveAttempts;
            m_cachedProfileUid = FindObjectByPath(PROFILE_UID_PATH, findStringFunc, findGameObjectFunc);
            ForceHideCachedObject(m_cachedProfileUid, setActiveFunc);
            m_resolveProfileUidNext = false;
        }
    }

}

bool HidePlayerInfo::IsEnabled()
{
    return g_pEnv->HidePlayerInfo != FALSE;
}
