#include "HidePlayerInfo.h"
#include "../framework.h"
#include "../dllmain.h"
#include "../Constants.h"
#include "HooksShared.h"

typedef Il2CppString* (*FindStringFn)(const char*);
typedef void* (*FindGameObjectFn)(void*);
typedef void(*SetActiveFn)(void*, bool);
typedef void (*SetupPlayerProfilePageFn)(void*);

static void* FindObjectByPath(
    const char* path,
    FindStringFn findStringFunc,
    FindGameObjectFn findGameObjectFunc)
{
    Il2CppString* pathString = findStringFunc(path);
    return pathString ? findGameObjectFunc(pathString) : nullptr;
}

static void ForceHide(
    void* object,
    SetActiveFn setActiveFunc)
{
    if (!object)
    {
        return;
    }

    __try
    {
        setActiveFunc(object, false);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

void HidePlayerInfo::Initialize()
{
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

    if (g_pEnv->Offsets.SetupPlayerProfilePage)
    {
        LPVOID setupPlayerProfilePageAddr = GetFunctionAddress(g_pEnv->Offsets.SetupPlayerProfilePage);
        if (setupPlayerProfilePageAddr)
        {
            MH_CreateHook(setupPlayerProfilePageAddr, &HidePlayerInfo::HookSetupPlayerProfilePage, &originalSetupPlayerProfilePage);
        }
    }
}

void HidePlayerInfo::OnUpdate()
{
    // UID hiding is event-driven from the SetWaterMaskUID hook; no polling.
}

void HidePlayerInfo::HideUidWatermark()
{
    if (!g_pEnv->HidePlayerInfo)
    {
        return;
    }

    if (!findString || !findGameObject || !setActive)
    {
        return;
    }

    FindStringFn findStringFunc = (FindStringFn)findString;
    FindGameObjectFn findGameObjectFunc = (FindGameObjectFn)findGameObject;
    SetActiveFn setActiveFunc = (SetActiveFn)setActive;

    void* uidObj = FindObjectByPath(UID_PATH, findStringFunc, findGameObjectFunc);
    ForceHide(uidObj, setActiveFunc);
}

void HidePlayerInfo::HideProfileInfo()
{
    if (!g_pEnv->HidePlayerInfo)
    {
        return;
    }

    if (!findString || !findGameObject || !setActive)
    {
        return;
    }

    FindStringFn findStringFunc = (FindStringFn)findString;
    FindGameObjectFn findGameObjectFunc = (FindGameObjectFn)findGameObject;
    SetActiveFn setActiveFunc = (SetActiveFn)setActive;

    const char* profilePaths[] = {
        PROFILE_UID_PATH,
        PROFILE_NAME_PATH,
        PROFILE_BIRTHDAY_PATH,
    };

    for (const char* path : profilePaths)
    {
        void* obj = FindObjectByPath(path, findStringFunc, findGameObjectFunc);
        ForceHide(obj, setActiveFunc);
    }
}

bool HidePlayerInfo::IsEnabled()
{
    return g_pEnv->HidePlayerInfo != FALSE;
}

void* HidePlayerInfo::GetHookFunction()
{
    return (void*)&HidePlayerInfo::HookSetupPlayerProfilePage;
}

void HidePlayerInfo::HookSetupPlayerProfilePage(void* pThis)
{
    if (originalSetupPlayerProfilePage)
    {
        SetupPlayerProfilePageFn original = (SetupPlayerProfilePageFn)originalSetupPlayerProfilePage;
        original(pThis);
    }

    // The page is allowed to open normally; hide the identity objects after
    // the original has populated them.
    if (g_pEnv->HidePlayerInfo)
    {
        HidePlayerInfo::HideProfileInfo();
    }
}
