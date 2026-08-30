#pragma once

#include <Windows.h>

struct Il2CppString;

extern bool g_cachedIsResisted;

bool CacheResistState(Il2CppString* textContent);

bool CheckResistInBeyd();
