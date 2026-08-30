#include "HidePlayerInfo.h"
#include "../framework.h"
#include "../dllmain.h"
#include "../Constants.h"
#include "HooksShared.h"

typedef Il2CppString* (*FindStringFn)(const char*);
typedef void* (*FindGameObjectFn)(void*);
typedef void(*SetActiveFn)(void*, bool);

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

bool HidePlayerInfo::IsEnabled()
{
    return g_pEnv->HidePlayerInfo != FALSE;
}
